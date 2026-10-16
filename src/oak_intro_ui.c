#include "oak_intro.h"

/* ---------------------------------------------------------------- message box */

void OakUI_ShowMessage(const u8 *str)
{
    LoadMessageBox(WINDOW_MESSAGE, 0);
    StringExpandPlaceholders(gStringVar4, str);
    AddTextPrinterParameterized2(WINDOW_MESSAGE, 4, gStringVar4, OAK_TEXT_SPEED, NULL, 2, 1, 3);
    CopyWindowToVram(WINDOW_MESSAGE, 3);
}

/* ---------------------------------------------------------------- multichoice */

u8 OakUI_ShowMultichoice(const struct WindowTemplate *template, const u8 *const *choices)
{
    u8 windowId = AddWindow(template);
    u8 count = 0;
    u8 i;

    DrawStdFrameWithCustomTileAndPalette(windowId, TRUE, 0x214, 0xE);
    FillWindowPixelBuffer(windowId, 0x11);
    PutWindowTilemap(windowId);

    for (i = 0; choices[i] != NULL; i++)
    {
        OakMenu_PrintText(windowId, 2, 8, 1 + 16 * i, 1, 0, choices[i]);
        count++;
    }

    Menu_InitCursor(windowId, 2, 0, 1, GetFontAttribute(2, 1) + 2, count, 0);
    CopyWindowToVram(windowId, 3);
    return windowId;
}

void OakUI_HideMultichoice(u8 windowId)
{
    /* all three calls are needed to fully clear a multichoice, and the dialogue box with it */
    ClearStdWindowAndFrame(windowId, TRUE);
    RemoveWindow(windowId);
    ClearDialogWindowAndFrame(WINDOW_MESSAGE, TRUE);
}

/* ---------------------------------------------------------------- sprite blending */

void OakUI_Blend_Set(u8 eva, u8 evb)
{
    gOak->blendEva = eva;
    gOak->blendTimer = FADE_STEPS;
    SetGpuReg(REG_OFFSET_BLDALPHA, BLDALPHA_BLEND(eva, evb));
}

void OakUI_Blend_Reset(void)
{
    SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT2_ALL_BG);
    OakUI_Blend_Set(0, 16);
}

/* Moves the sprite weight one step towards targetEva every FADE_STEPS frames. */
bool8 OakUI_Blend_Step(u8 targetEva)
{
    struct OakIntro *oak = gOak;

    if (oak->blendEva == targetEva)
        return TRUE;
    if (oak->blendTimer != 0)
    {
        oak->blendTimer--;
        return FALSE;
    }
    if (oak->blendEva < targetEva)
        oak->blendEva++;
    else
        oak->blendEva--;
    OakUI_Blend_Set(oak->blendEva, 16 - oak->blendEva);
    return oak->blendEva == targetEva;
}

/* ---------------------------------------------------------------- screen fades */

void OakUI_FadeToBlack(void)
{
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
}

void OakUI_FadeFromBlack(s8 delay)
{
    BeginNormalPaletteFade(PALETTES_ALL, delay, 16, 0, RGB_BLACK);
}

/* ---------------------------------------------------------------- random */

/*
 * Random number in 0..n-1 without a division: Thumb on the GBA has no divide instruction,
 * so `%` would need __aeabi_uidivmod from libgcc, which this build does not link.
 */
u16 OakUI_RandomBelow(u16 n)
{
    return ((u32)Random() * n) >> 16;
}
