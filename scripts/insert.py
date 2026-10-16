#!/usr/bin/env python3
"""ROM insertion, hooks and byte replacements (used by make.py).

hooks            one per line:  <rom address> <function> [low register 0-7, default 1]
                 The first bytes of the ROM function at <address> are replaced by
                     ldr rN, [pc, ...] ; bx rN ; .word <function>
                 so the vanilla function jumps into your code. r0-r3 keep their arguments.
bytereplacement  one per line:  <rom address> <hex bytes>   e.g.  0812F7C0 00 49 08 47
Lines starting with '#' and blank lines are ignored.
"""
import os
import shutil
import struct
import sys


def fail(msg):
    print('error: ' + msg)
    sys.exit(1)


def _lines(path):
    if not os.path.exists(path):
        return
    with open(path) as f:
        for n, line in enumerate(f, 1):
            line = line.split('#', 1)[0].strip()
            if line:
                yield n, line.split()


def find_free(rom, size, start=0x100000, align=0x100, limit=8):
    data = open(rom, 'rb').read()
    found, run, i = [], 0, start
    while i < len(data) and len(found) < limit:
        j = data.find(b'\xff' * size, i)
        if j < 0:
            break
        j = (j + align - 1) // align * align
        if data[j:j + size] == b'\xff' * size:
            found.append(j)
            i = j + size
        else:
            i = j + 1
    return found


def thumb_hook(addr, target, reg):
    """Bytes that make a Thumb function at addr (rom offset) jump to target (thumb address)."""
    if not 0 <= reg <= 7:
        fail('hook register must be r0-r7')
    ldr, bx = 0x4800 | (reg << 8), 0x4700 | (reg << 3)
    if addr % 4 == 0:
        return struct.pack('<HHI', ldr, bx, target)
    return struct.pack('<HHHI', ldr | 1, bx, 0x46C0, target)  # ldr (imm 1), bx, nop, .word


def header(path):
    with open(path, 'rb') as f:
        f.seek(0xAC)
        code = f.read(4).decode('ascii', 'replace')
        f.seek(0xBC)
        return code, f.read(1)[0]


def insert(rom, blob, offset, out, hooks_path, bytes_path, symbols, force=False):
    code, version = header(rom)
    hook_lines = list(_lines(hooks_path))
    if hook_lines and not force and (code != 'BPRE' or version != 0):
        fail('%s is %s v%d. The addresses in "hooks" are for FireRed US 1.0 (BPRE, version 0).\n'
             '       Use the right base ROM, or set FORCE = True in scripts/make.py if you know they match.'
             % (rom, code, version))
    data = open(blob, 'rb').read()
    with open(rom, 'rb') as f:
        f.seek(0, 2)
        if offset + len(data) > f.tell():
            fail('0x%X + %d bytes runs past the end of %s' % (offset, len(data), rom))
        f.seek(offset)
        existing = f.read(len(data))
    if existing.strip(b'\xff'):
        fail('0x%08X..0x%08X in %s is not free space.\n       Set OFFSET_TO_PUT in scripts/make.py '
             '(try: python scripts/make.py find).' % (0x8000000 + offset, 0x8000000 + offset + len(data), rom))
    src = open(rom, 'rb').read()
    for n, p in hook_lines:
        addr = int(p[0], 16) & 0x1FFFFFF
        old = src[addr:addr + 12]
        # A Thumb function starts with push {..., lr} = 0xB5xx. Anything else means the address is
        # wrong for this ROM (or something else, e.g. CFRU, already patched that function).
        if not force and old[1:2] != b'\xb5':
            fail('hooks:%d: 0x%08X does not start like a Thumb function.\n'
                 '       bytes there: %s   (expected xx B5 = push {..,lr})\n'
                 '       The hook address is wrong for this ROM, or that function was already patched.\n'
                 '       Nothing was written. Set FORCE = True in scripts/make.py to patch anyway.'
                 % (n, 0x8000000 + addr, old.hex()))
    del src
    shutil.copyfile(rom, out)
    with open(out, 'r+b') as f:
        f.seek(offset)
        f.write(data)
        for n, p in hook_lines:
            if len(p) < 2:
                fail('hooks:%d: expected "<address> <function> [register]"' % n)
            addr = int(p[0], 16) & 0x1FFFFFF
            if p[1] not in symbols:
                fail('hooks:%d: %s is not a global function in the build' % (n, p[1]))
            reg = int(p[2].lstrip('rR'), 0) if len(p) > 2 else 1
            f.seek(addr)
            old = f.read(12)
            f.seek(addr)
            f.write(thumb_hook(addr, symbols[p[1]], reg))
            print('  hook 0x%08X -> %s (0x%08X)   was: %s' % (0x8000000 + addr, p[1], symbols[p[1]], old[:8].hex()))
        for n, p in _lines(bytes_path):
            f.seek(int(p[0], 16) & 0x1FFFFFF)
            f.write(bytes(int(b, 16) for b in p[1:]))
            print('  patch 0x%s' % p[0])


def verify(out, blob, offset, hooks_path, symbols):
    """Re-read the patched ROM and check the hooks and the blob really are what the build produced."""
    ok = True
    data = open(blob, 'rb').read()
    rom = open(out, 'rb').read()
    print('ROM header: %s v%d' % header(out))
    if rom[offset:offset + len(data)] == data:
        print('blob      : OK, %d bytes at 0x%08X' % (len(data), 0x8000000 + offset))
    else:
        ok = False
        print('blob      : MISMATCH at 0x%08X - the ROM does not contain this build' % (0x8000000 + offset))
    lo, hi = 0x8000000 + offset, 0x8000000 + offset + len(data)
    by_addr = {v: k for k, v in symbols.items()}
    for n, p in _lines(hooks_path):
        addr = int(p[0], 16) & 0x1FFFFFF
        reg = int(p[2].lstrip('rR'), 0) if len(p) > 2 else 1
        want = thumb_hook(addr, symbols.get(p[1], 0), reg)
        got = rom[addr:addr + len(want)]
        lit = struct.unpack('<I', got[-4:])[0]
        in_blob = lo <= (lit & ~1) < hi
        print('hook %s -> %s' % (p[0], p[1]))
        print('   bytes now : %s' % got.hex())
        print('   jumps to  : 0x%08X  (%s)  %s' % (lit, by_addr.get(lit, '?'), 'inside the blob' if in_blob else 'OUTSIDE the blob!'))
        print('   thumb bit : %s' % ('set' if lit & 1 else 'MISSING - would crash'))
        if got != want or not in_blob or not lit & 1:
            ok = False
            print('   RESULT    : BAD')
        else:
            tgt = rom[(lit & ~1) - 0x8000000:(lit & ~1) - 0x8000000 + 4]
            print('   target code starts: %s   RESULT: OK' % tgt.hex())
    return ok
