#!/usr/bin/env python3
"""Convert graphics/**/*.png into GBA data, no grit needed (only Pillow).

For every  graphics/<name>.png  this writes, into build/graphics/:

    <name>.4bpp.lz    LZ77-compressed 4bpp tiles, 8x8 tiles in row-major order
    <name>.gbapal     16-colour BGR555 palette (colour 0 = transparent)

and for  graphics/<name>.bg.png  (a background picture) additionally

    <name>.bin.lz     LZ77-compressed 32x32 tilemap (duplicate/flipped tiles merged)

Rules
  * Max 15 opaque colours + transparency. Colours are reduced to 15-bit first.
  * Transparent = alpha < 128, or a colour within 3 of a key in TRANSPARENT_KEYS
    (the old project's sprites use #ADDDDD).
  * Width and height must be multiples of 8.
  * A 64-pixel-wide image is already laid out for 64x64 OAM sprites in 1D mapping:
    each 64 rows are one sprite frame (64 tiles).

build/gen/graphics.s (assembled from the project root) declares  gGfx_<name>_Tiles / _Pal / _Map  and
build/gen/graphics.h declares them plus  gGfx_<name>_TilesSize  (decompressed bytes).
"""
import os
import re
import struct
import sys

TRANSPARENT_KEYS = [(0xAD, 0xDD, 0xDD)]
KEY_TOLERANCE = 3


def lz77(data):
    """GBA BIOS LZ77 (type 0x10). Never emits displacement 1, so it is valid for
    both the WRAM and the VRAM decompressors."""
    n = len(data)
    out = bytearray(b'\x10' + struct.pack('<I', n)[:3])
    i = 0
    while i < n:
        flags = 0
        block = bytearray()
        for bit in range(8):
            best_len, best_off = 0, 0
            if i < n:
                max_len = min(18, n - i)
                for off in range(2, min(i, 4096) + 1):
                    j = i - off
                    ln = 0
                    while ln < max_len and data[j + ln] == data[i + ln]:
                        ln += 1
                    if ln > best_len:
                        best_len, best_off = ln, off
                        if ln == max_len:
                            break
            if best_len >= 3:
                flags |= 0x80 >> bit
                d = best_off - 1
                block.append(((best_len - 3) << 4) | (d >> 8))
                block.append(d & 0xFF)
                i += best_len
            elif i < n:
                block.append(data[i])
                i += 1
            else:
                break
        out.append(flags)
        out += block
    while len(out) % 4:
        out.append(0)
    return bytes(out)


def rgb555(r, g, b):
    return (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10)


def load(path):
    from PIL import Image
    im = Image.open(path).convert('RGBA')
    w, h = im.size
    if w % 8 or h % 8:
        raise ValueError('%s: size %dx%d is not a multiple of 8' % (path, w, h))
    px = im.load()
    pal = [0]  # index 0 = transparent
    index = {}
    rows = []
    for y in range(h):
        row = []
        for x in range(w):
            r, g, b, a = px[x, y]
            if a < 128 or any(max(abs(r - k[0]), abs(g - k[1]), abs(b - k[2])) <= KEY_TOLERANCE
                              for k in TRANSPARENT_KEYS):
                row.append(0)
                continue
            c = rgb555(r, g, b)
            if c not in index:
                if len(pal) >= 16:
                    raise ValueError('%s: more than 15 opaque colours (found at x=%d y=%d). '
                                     'Reduce the palette first.' % (path, x, y))
                index[c] = len(pal)
                pal.append(c)
            row.append(index[c])
        rows.append(row)
    pal += [0] * (16 - len(pal))
    return w, h, rows, pal


def tile_at(rows, tx, ty):
    return tuple(tuple(rows[ty * 8 + y][tx * 8:tx * 8 + 8]) for y in range(8))


def tile_bytes(t):
    out = bytearray()
    for row in t:
        for x in range(0, 8, 2):
            out.append(row[x] | (row[x + 1] << 4))
    return bytes(out)


def hflip(t):
    return tuple(tuple(reversed(r)) for r in t)


def vflip(t):
    return tuple(reversed(t))


def convert(path, outdir, name, is_bg):
    w, h, rows, pal = load(path)
    tw, th = w // 8, h // 8
    outputs = {}
    if is_bg:
        if tw > 32 or th > 32:
            raise ValueError('%s: background larger than 256x256' % path)
        uniq, lookup, tmap = [], {}, [0] * (32 * 32)
        for ty in range(th):
            for tx in range(tw):
                t = tile_at(rows, tx, ty)
                entry = None
                for flip, bits in ((t, 0), (hflip(t), 1), (vflip(t), 2), (vflip(hflip(t)), 3)):
                    if flip in lookup:
                        entry = lookup[flip] | (bits << 10)
                        break
                if entry is None:
                    lookup[t] = len(uniq)
                    uniq.append(t)
                    entry = lookup[t]
                tmap[ty * 32 + tx] = entry
        tiles = b''.join(tile_bytes(t) for t in uniq)
        outputs['bin.lz'] = lz77(b''.join(struct.pack('<H', e) for e in tmap))
    else:
        tiles = b''.join(tile_bytes(tile_at(rows, tx, ty)) for ty in range(th) for tx in range(tw))
    outputs['4bpp.lz'] = lz77(tiles)
    outputs['gbapal'] = b''.join(struct.pack('<H', c) for c in pal)
    os.makedirs(outdir, exist_ok=True)
    for ext, data in outputs.items():
        with open(os.path.join(outdir, '%s.%s' % (name, ext)), 'wb') as f:
            f.write(data)
    return len(tiles), 'bin.lz' in outputs


def symbol_for(rel):
    base = re.sub(r'(\.bg)?\.png$', '', rel.replace('\\', '/'))
    return re.sub(r'[^A-Za-z0-9]+', '_', base).strip('_')


def build(gfx_dir, build_dir, gen_dir, log=print):
    """Convert changed PNGs, regenerate graphics.s / graphics.h. Returns number of images."""
    entries = []
    for dp, _, files in os.walk(gfx_dir):
        for fn in sorted(files):
            if fn.lower().endswith('.png'):
                full = os.path.join(dp, fn)
                rel = os.path.relpath(full, gfx_dir)
                entries.append((symbol_for(rel), full, fn.lower().endswith('.bg.png')))
    entries.sort()
    names = [e[0] for e in entries]
    dups = {n for n in names if names.count(n) > 1}
    if dups:
        raise ValueError('duplicate graphic names: ' + ', '.join(sorted(dups)))
    outdir = os.path.join(build_dir, 'graphics')
    asm = ['@ Generated by scripts/gfx.py - do not edit', '.section .rodata', '']
    hdr = ['/* Generated by scripts/gfx.py - do not edit */', '#ifndef GUARD_GEN_GRAPHICS_H',
           '#define GUARD_GEN_GRAPHICS_H', '', '#include "types.h"', '']
    for name, full, is_bg in entries:
        stamp = os.path.join(outdir, name + '.gbapal')
        if not os.path.exists(stamp) or os.path.getmtime(stamp) < os.path.getmtime(full) \
                or os.path.getmtime(stamp) < os.path.getmtime(__file__):
            log('  gfx  %s' % os.path.relpath(full, os.path.dirname(gfx_dir)))
            size, _ = convert(full, outdir, name, is_bg)
        else:
            size = _decompressed_size(os.path.join(outdir, name + '.4bpp.lz'))
        rel = lambda ext: os.path.relpath(os.path.join(outdir, '%s.%s' % (name, ext)), os.path.dirname(build_dir)).replace('\\', '/')
        parts = [('Tiles', '4bpp.lz'), ('Pal', 'gbapal')] + ([('Map', 'bin.lz')] if is_bg else [])
        for sym, ext in parts:
            asm += ['.align 2', '.global gGfx_%s_%s' % (name, sym), 'gGfx_%s_%s:' % (name, sym),
                    '\t.incbin "%s"' % rel(ext)]
            ctype = 'u16' if sym == 'Pal' else 'u32'
            hdr.append('extern const %s gGfx_%s_%s[];' % (ctype, name, sym))
        hdr.append('#define gGfx_%s_TilesSize %d' % (name, size))
        hdr.append('')
    hdr += ['#endif', '']
    os.makedirs(gen_dir, exist_ok=True)
    with open(os.path.join(gen_dir, 'graphics.s'), 'w', newline='\n') as f:
        f.write('\n'.join(asm) + '\n')
    with open(os.path.join(gen_dir, 'graphics.h'), 'w', newline='\n') as f:
        f.write('\n'.join(hdr))
    return len(entries)


def _decompressed_size(path):
    with open(path, 'rb') as f:
        return struct.unpack('<I', f.read(4)[1:4] + b'\x00')[0]


if __name__ == '__main__':
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    n = build(os.path.join(root, 'graphics'), os.path.join(root, 'build'), os.path.join(root, 'build', 'gen'))
    print('%d images' % n)
