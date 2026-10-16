#ifndef GUARD_WINDOW_H
#define GUARD_WINDOW_H

#include "types.h"

struct WindowTemplate
{
    u8 bg;
    u8 tilemapLeft;
    u8 tilemapTop;
    u8 width;
    u8 height;
    u8 paletteNum;
    u16 baseBlock;
};

#define WINDOW_NONE 0xFF
#define WINDOW_MESSAGE 0   /* window 0 is the dialogue box set up by the Oak-speech window init */

/* text / window engine */
u8 AddWindow(const struct WindowTemplate *template);
void RemoveWindow(u8 windowId);
void PutWindowTilemap(u8 windowId);
void FillWindowPixelBuffer(u8 windowId, u8 fillValue);
void CopyWindowToVram(u8 windowId, u8 mode);
bool8 IsTextPrinterActive(u8 id);
u8 *StringExpandPlaceholders(u8 *dest, const u8 *src);
u8 *StringCopy(u8 *dest, const u8 *src);
extern u8 gStringVar4[];

void LoadMessageBox(u8 windowId, u8 copyToVram);
void AddTextPrinterParameterized2(u8 windowId, u8 fontId, const u8 *str, u8 speed, void (*callback)(void),
                                  u8 fgColor, u8 bgColor, u8 shadowColor);
void HideMessageBox(void);
u8 GetFontAttribute(u8 fontId, u8 attr);
void OakMenu_PrintText(u8 windowId, u8 fontId, u8 x, u8 y, u32 a, u32 b, const u8 *str);
void DrawStdFrameWithCustomTileAndPalette(u8 windowId, bool8 copyToVram, u16 baseTileNum, u8 paletteNum);
void ClearStdWindowAndFrame(u8 windowId, bool8 copyToVram);
void ClearDialogWindowAndFrame(u8 windowId, bool8 copyToVram);
void Menu_InitCursor(u8 windowId, u8 fontId, u8 left, u8 top, u8 cursorHeight, u8 numChoices, u8 initialCursorPos);
s8 Menu_ProcessInputNoWrapClearOnChoose(void);
void sub_8003ECC(void);
void sub_80F6C6C(void);
void sub_80F6C98(void);
void sub_80F6808(void);

#endif // GUARD_WINDOW_H
