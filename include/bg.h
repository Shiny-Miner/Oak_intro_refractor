#ifndef GUARD_BG_H
#define GUARD_BG_H

#include "types.h"

#define BG_COORD_SET 0
#define BG_COORD_ADD 1
#define BG_COORD_SUB 2

struct BgTemplate;
extern const struct BgTemplate gOakIntroBgTemplates; /* first entry of the ROM table; pass its address */

void ResetBgsAndClearDma3BusyFlags(bool32 enable);
void InitBgsFromTemplates(u8 bgMode, const struct BgTemplate *templates, u8 numTemplates);
void SetBgTilemapBuffer(u8 bg, void *tilemap);
u32 ChangeBgX(u8 bg, u32 value, u8 op);
u32 ChangeBgY(u8 bg, u32 value, u8 op);
s32 GetBgX(u8 bg);
s32 GetBgY(u8 bg);
void ShowBg(u8 bg);
void HideBg(u8 bg);
u16 LoadBgTiles(u8 bg, const void *src, u16 size, u16 tileOffset);
void CopyToBgTilemapBuffer(u8 bg, const void *src, u16 mode, u16 destOffset);
void CopyBgTilemapBufferToVram(u8 bg);
void LZ77UnCompVram(const void *src, void *dest);
void *malloc_and_decompress(const void *src, u32 *size);
void *AllocZeroed(u32 size);
void Free(void *ptr);
void *memcpy(void *dest, const void *src, u32 n);
void *memset(void *dest, int c, u32 n);

/* palette fade / blend */
struct PaletteFadeControl
{
    u16 unk0;
    u16 unk2;
    u16 unk4;
    u16 status; /* bit 7 set while a fade is running */
};
extern struct PaletteFadeControl gPaletteFade;
#define PaletteFadeActive() ((gPaletteFade.status & 0x80) != 0)

void BeginNormalPaletteFade(u32 selectedPalettes, s8 delay, u8 startY, u8 targetY, u16 blendColor);
void LoadPalette(const void *src, u16 offset, u16 size);
void sub_8070528(void);
void sub_8087E64(void);

/* sound */
void PlayBGM(u16 song);
void PlaySE(u16 song);
void FadeOutBGM(u8 speed);

/* BIOS: BitUnPack (see assembly/bios.s) */
struct BitUnPackParams
{
    u16 srcLength;
    u8 srcBitWidth;
    u8 destBitWidth;
    u32 destOffset;
};
void CpuFill32(u32 value, void *dest, u32 bytes);
void BitUnPack(const void *src, void *dest, const struct BitUnPackParams *params);

#endif // GUARD_BG_H
