/*
 * Oak intro: entry points, video setup and the opening scene.
 *
 *   OakIntro_Hook            hooked over the vanilla "Welcome to the world of Pokemon" task (see hooks).
 *                            Runs inside the vanilla Oak speech: its BGs, windows, VBlank and main
 *                            callback stay exactly as the vanilla code left them (no video reset).
 *   CB2_OakIntro_Setup       only used when coming back from the naming screen, which wipes the video
 *                            state: it rebuilds it, then creates the task for gOak->entry.
 *
 * Scene order:  hello -> boy/girl (oak_intro_gender.c) -> player name -> rival
 *               (oak_intro_name.c) -> final speech and hand-over (oak_intro_finish.c)
 */
#include "oak_intro.h"

void Task_OakIntro_Dummy(u8 taskId)
{
}

/* ---------------------------------------------------------------- state block */

static void AllocState(enum OakEntry entry)
{
    struct OakIntro *oak = AllocZeroed(sizeof(struct OakIntro));

    oak->entry = entry;
    oak->boy.top = oak->boy.bottom = SPRITE_NONE;
    oak->girl.top = oak->girl.bottom = SPRITE_NONE;
    oak->npc.top = oak->npc.bottom = SPRITE_NONE;
    oak->ballSprite = oak->monSprite = SPRITE_NONE;
    gOak = oak; /* the BG tilemap buffers are only needed after the naming screen (CB2_OakIntro_Setup) */
}

/* The vanilla intro leaves its own sprites alive; the actors need the sprite slots and VRAM. */
static void DestroyVanillaSprites(void)
{
    int i;

    for (i = 0; i < MAX_SPRITES; i++)
    {
        if (gSprites[i].inUse)
            DestroySpriteAndFreeResources(&gSprites[i]);
    }
}

/* Hook target. r0 = task id of the vanilla task that was running at the hooked address. */
void OakIntro_Hook(u8 taskId)
{
    AllocState(OAK_ENTRY_NEW);
    DestroyVanillaSprites();
    gTasks[taskId].func = Task_OakIntro_Begin;
}

/* ---------------------------------------------------------------- background */

static void LoadBackground(void)
{
    u32 size;
    void *tiles = malloc_and_decompress(gGfx_background_Tiles, &size);

    LoadBgTiles(1, tiles, size, 0);
    CopyToBgTilemapBuffer(1, gGfx_background_Map, 0, 0);
    LoadPalette(gGfx_background_Pal, 0x00, 0x20);
    CopyBgTilemapBufferToVram(1);
    Free(tiles);
}

/* ---------------------------------------------------------------- video setup */

void CB2_OakIntro_Setup(void)
{
    struct OakIntro *oak = gOak;

    switch (oak->setupState)
    {
    case 0:
        gMain.vblankCallback = NULL;
        /* the naming screen leaves its own graphics behind: wipe VRAM and OAM (word writes only) */
        CpuFill32(0, (void *)0x06000000, 0x18000);
        CpuFill32(0x00A000A0, (void *)0x07000000, 0x400); /* every OAM entry: y = 160 (off screen) */
        sub_8070528();   /* reset palette fade */
        sub_8087E64();
        sub_8006B10();   /* reset sprites */
        sub_80088F0();   /* free sprite palettes */
        sub_80F6808();
        /* all sprites are gone now, forget the old ids */
        oak->boy.top = oak->boy.bottom = SPRITE_NONE;
        oak->girl.top = oak->girl.bottom = SPRITE_NONE;
        oak->npc.top = oak->npc.bottom = SPRITE_NONE;
        break;
    case 1:
        if (oak->bg1Map == NULL)
        {
            oak->bg1Map = AllocZeroed(0x800);
            oak->bg2Map = AllocZeroed(0x800);
        }
        ResetBgsAndClearDma3BusyFlags(FALSE);
        InitBgsFromTemplates(1, &gOakIntroBgTemplates, 3);
        SetBgTilemapBuffer(1, oak->bg1Map);
        SetBgTilemapBuffer(2, oak->bg2Map);
        ChangeBgX(1, 0, BG_COORD_SET);
        ChangeBgY(1, 0, BG_COORD_SET);
        ChangeBgX(2, 0, BG_COORD_SET);
        ChangeBgY(2, 0, BG_COORD_SET);
        break;
    case 2:
        SetGpuReg(0x40, 0);
        SetGpuReg(0x44, 0);
        SetGpuReg(0x48, 0);
        SetGpuReg(0x4A, 0);
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        SetGpuReg(0x54, 0);
        break;
    case 3:
        sub_8003ECC();   /* message box layer */
        sub_80F6C6C();
        sub_80F6C98();
        break;
    case 4:
        CreateTask(Task_OakIntro_Begin, 0);
        break;
    case 5:
        oak->setupState = 0;
        BeginNormalPaletteFade(1, 0, 16, 0, RGB_BLACK);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON);
        ShowBg(0);
        ShowBg(1);
        sub_8000B68(1);
        gMain.vblankCallback = VBlankCB_OakSpeech;
        SetMainCallback2(CB2_OakSpeech);
        return;
    }
    oak->setupState++;
}

/* ---------------------------------------------------------------- scene 1: hello */

static void Task_Hello_WaitMessage(u8 taskId);
static void Task_Hello_Done(u8 taskId);

/* Created by the setup callback; chooses the scene to start at. */
void Task_OakIntro_Begin(u8 taskId)
{
    struct Task *task = &gTasks[taskId];
    int i;

    for (i = 0; i < 16; i++)
        task->data[i] = 0;

    LoadBackground();

    switch (gOak->entry)
    {
    case OAK_ENTRY_NEW:
        /*
         * Partial unfade: everything but palettes 0-3 so the message box shows on black.
         * (The original also started a second, full fade-to-black right after this; the palette
         * fade engine ignores a new fade while one is running, so it never did anything.)
         */
        BeginNormalPaletteFade(PALETTES_ALL_BUT_TEXTBOX, 0, 16, 0, RGB_BLACK);
        HideBg(1);
        OakUI_ShowMessage(gText_OakIntro_HelloThere);
        task->func = Task_Hello_WaitMessage;
        break;
    case OAK_ENTRY_PLAYER_NAMED:
        task->func = Task_OakIntro_ResumeAfterPlayerName;
        break;
    case OAK_ENTRY_RIVAL_NAMED:
        task->func = Task_OakIntro_ResumeAfterRivalName;
        break;
    }
}

static void Task_Hello_WaitMessage(u8 taskId)
{
    if (IsTextPrinterActive(WINDOW_MESSAGE))
        return;
    HideMessageBox();
    ShowBg(1);
    OakUI_FadeFromBlack(0);
    PlayBGM(OAK_BGM);
    gTasks[taskId].tTimer = 0xA0;
    gTasks[taskId].func = Task_Hello_Done;
}

static void Task_Hello_Done(u8 taskId)
{
    if (PaletteFadeActive())
        return;
    if (gTasks[taskId].tTimer != 0)
    {
        gTasks[taskId].tTimer--;
        return;
    }
    /* Rowan fades in, then the speech (oak_intro_rowan.c) */
    OakIntro_SwapNpc(taskId, ACTOR_PROF, Task_OakIntro_RowanIntroduce);
}
