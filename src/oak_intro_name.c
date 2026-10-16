/*
 * Scene 3: the player's name (naming screen), the professor introduces the rival,
 * the rival's name (menu of presets or the naming screen).
 *
 * The naming screen replaces the main callback and wipes sprites, so after it we come back
 * through CB2_OakIntro_Setup with gOak->entry set to OAK_ENTRY_*_NAMED.
 */
#include "oak_intro.h"

#define NUM_DEFAULT_NAMES 5
#define NUM_RIVAL_PRESETS 4 /* shown in the menu below "New Name!" */

static const u8 *const sBoyNames[NUM_DEFAULT_NAMES] = {
    gText_OakIntro_BoyName1, gText_OakIntro_BoyName2, gText_OakIntro_BoyName3,
    gText_OakIntro_BoyName4, gText_OakIntro_BoyName5,
};
static const u8 *const sGirlNames[NUM_DEFAULT_NAMES] = {
    gText_OakIntro_GirlName1, gText_OakIntro_GirlName2, gText_OakIntro_GirlName3,
    gText_OakIntro_GirlName4, gText_OakIntro_GirlName5,
};
static const u8 *const sRivalNames[NUM_DEFAULT_NAMES] = {
    gText_OakIntro_RivalName1, gText_OakIntro_RivalName2, gText_OakIntro_RivalName3,
    gText_OakIntro_RivalName4, gText_OakIntro_RivalName5,
};

static const struct WindowTemplate sYesNoWindow = { 0, 2, 2, 4, 4, 0xF, 0x130 };
static const struct WindowTemplate sRivalMenuWindow = { 0, 2, 2, 9, 10, 0xF, 0x130 };

static void Task_PlayerFadeOut(u8 taskId);
static void Task_PlayerOpenNaming(u8 taskId);
static void Task_PlayerConfirm(u8 taskId);
static void Task_PlayerConfirmInput(u8 taskId);
static void Task_PlayerRetryFade(u8 taskId);
static void Task_ProfIntroducesRival(u8 taskId);
static void Task_WaitIntroduceRival(u8 taskId);
static void Task_RivalAsk(u8 taskId);
static void Task_RivalSlideRight(u8 taskId);
static void Task_RivalMenuInput(u8 taskId);
static void Task_RivalOpenNaming(u8 taskId);
static void Task_RivalPresetChosen(u8 taskId);
static void Task_RivalConfirmMessage(u8 taskId);
static void Task_RivalConfirm(u8 taskId);
static void Task_RivalConfirmInput(u8 taskId);

/* ---------------------------------------------------------------- player name */

void Task_OakIntro_AskName(u8 taskId)
{
    OakUI_ShowMessage(gText_OakIntro_AskName);
    gTasks[taskId].func = Task_PlayerFadeOut;
}

static void Task_PlayerFadeOut(u8 taskId)
{
    if (IsTextPrinterActive(WINDOW_MESSAGE))
        return;
    OakUI_FadeToBlack();
    gTasks[taskId].func = Task_PlayerOpenNaming;
}

static void Task_PlayerOpenNaming(u8 taskId)
{
    if (PaletteFadeActive())
        return;

    /* prefill with a random default so the player can just press START */
    if (gSaveBlock2Ptr->playerGender == FEMALE)
        StringCopy(gSaveBlock2Ptr->playerName, sGirlNames[OakUI_RandomBelow(NUM_DEFAULT_NAMES)]);
    else
        StringCopy(gSaveBlock2Ptr->playerName, sBoyNames[OakUI_RandomBelow(NUM_DEFAULT_NAMES)]);

    gOak->entry = OAK_ENTRY_PLAYER_NAMED;
    gOak->setupState = 0;
    gTasks[taskId].func = Task_OakIntro_Dummy; /* the naming screen keeps running tasks */
    DoNamingScreen(NAMING_SCREEN_PLAYER, gSaveBlock2Ptr->playerName, gSaveBlock2Ptr->playerGender, 0, 0,
                   CB2_OakIntro_Setup);
}

void Task_OakIntro_ResumeAfterPlayerName(u8 taskId)
{
    /* the setup callback's own fade must finish first, a new fade is ignored while one runs */
    if (PaletteFadeActive())
        return;
    SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT2_ALL_BG);
    OakUI_Blend_Set(16, 0);
    Actor_Create(&gOak->npc, OakIntro_PlayerStillKind(), ACTOR_CENTER_X, ACTOR_Y);
    OakUI_FadeFromBlack(0);
    OakUI_ShowMessage(gText_OakIntro_ConfirmName);
    gTasks[taskId].func = Task_PlayerConfirm;
}

static void Task_PlayerConfirm(u8 taskId)
{
    static const u8 *const sChoices[] = { gText_OakIntro_Yes, gText_OakIntro_No, NULL };

    if (IsTextPrinterActive(WINDOW_MESSAGE))
        return;
    gTasks[taskId].tWindowId = OakUI_ShowMultichoice(&sYesNoWindow, sChoices);
    gTasks[taskId].func = Task_PlayerConfirmInput;
}

static void Task_PlayerConfirmInput(u8 taskId)
{
    u8 windowId = gTasks[taskId].tWindowId;

    switch (Menu_ProcessInputNoWrapClearOnChoose())
    {
    case 0: /* YES: professor takes over */
        OakUI_HideMultichoice(windowId);
        OakIntro_SwapNpc(taskId, ACTOR_PROF, Task_ProfIntroducesRival);
        break;
    case 1: /* NO: the original sent the player back to the boy/girl screen */
        OakUI_HideMultichoice(windowId);
        OakUI_Blend_Set(16, 0);
        gTasks[taskId].func = Task_PlayerRetryFade;
        break;
    }
}

static void Task_PlayerRetryFade(u8 taskId)
{
    if (!OakUI_Blend_Step(0))
        return;
    Actor_Destroy(&gOak->npc);
    OakUI_Blend_Reset();
    gTasks[taskId].func = Task_OakIntro_ChooseGender;
}

/* ---------------------------------------------------------------- rival */

static void Task_ProfIntroducesRival(u8 taskId)
{
    OakUI_ShowMessage(gText_OakIntro_IntroduceRival);
    gTasks[taskId].func = Task_WaitIntroduceRival;
}

static void Task_WaitIntroduceRival(u8 taskId)
{
    if (IsTextPrinterActive(WINDOW_MESSAGE))
        return;
    HideMessageBox();
    OakIntro_SwapNpc(taskId, ACTOR_RIVAL, Task_RivalAsk);
}

static void Task_RivalAsk(u8 taskId)
{
    OakUI_ShowMessage(gText_OakIntro_AskRivalName);
    gTasks[taskId].func = Task_RivalSlideRight;
}

/* Also the "NO, ask again" entry point. */
static void Task_RivalSlideRight(u8 taskId)
{
    if (IsTextPrinterActive(WINDOW_MESSAGE))
        return;
    if (gSprites[gOak->npc.top].pos1.x < ACTOR_RIGHT_X)
    {
        Actor_SlideTo(&gOak->npc, ACTOR_RIGHT_X);
        return;
    }
    if (Actor_IsSliding(&gOak->npc))
        return;
    {
        static const u8 *const sChoices[] = {
            gText_OakIntro_RivalCustom, gText_OakIntro_RivalName1, gText_OakIntro_RivalName2,
            gText_OakIntro_RivalName3, gText_OakIntro_RivalName4, NULL,
        };
        gTasks[taskId].tWindowId = OakUI_ShowMultichoice(&sRivalMenuWindow, sChoices);
    }
    gTasks[taskId].func = Task_RivalMenuInput;
}

static void Task_RivalMenuInput(u8 taskId)
{
    s8 response = Menu_ProcessInputNoWrapClearOnChoose();

    if (response < 0)
        return;
    OakUI_HideMultichoice(gTasks[taskId].tWindowId);
    if (response == 0)
    {
        StringCopy(gRivalName, sRivalNames[OakUI_RandomBelow(NUM_DEFAULT_NAMES)]);
        OakUI_FadeToBlack();
        gTasks[taskId].func = Task_RivalOpenNaming;
    }
    else
    {
        StringCopy(gRivalName, sRivalNames[response - 1]);
        gTasks[taskId].func = Task_RivalPresetChosen;
    }
}

static void Task_RivalOpenNaming(u8 taskId)
{
    if (PaletteFadeActive())
        return;
    gOak->entry = OAK_ENTRY_RIVAL_NAMED;
    gOak->setupState = 0;
    gTasks[taskId].func = Task_OakIntro_Dummy;
    DoNamingScreen(NAMING_SCREEN_RIVAL, gRivalName, 4, 0, 0, CB2_OakIntro_Setup);
}

static void Task_RivalPresetChosen(u8 taskId)
{
    /* slide the rival back to the middle, then confirm */
    Actor_SlideTo(&gOak->npc, ACTOR_CENTER_X);
    gTasks[taskId].func = Task_RivalConfirmMessage;
}

static void Task_RivalConfirmMessage(u8 taskId)
{
    if (Actor_IsSliding(&gOak->npc))
        return;
    OakUI_ShowMessage(gText_OakIntro_ConfirmRivalName);
    gTasks[taskId].func = Task_RivalConfirm;
}

void Task_OakIntro_ResumeAfterRivalName(u8 taskId)
{
    if (PaletteFadeActive())
        return;
    SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT2_ALL_BG);
    OakUI_Blend_Set(16, 0);
    Actor_Create(&gOak->npc, ACTOR_RIVAL, ACTOR_CENTER_X, ACTOR_Y);
    OakUI_FadeFromBlack(0);
    OakUI_ShowMessage(gText_OakIntro_ConfirmRivalName);
    gTasks[taskId].func = Task_RivalConfirm;
}

static void Task_RivalConfirm(u8 taskId)
{
    static const u8 *const sChoices[] = { gText_OakIntro_Yes, gText_OakIntro_No, NULL };

    if (IsTextPrinterActive(WINDOW_MESSAGE))
        return;
    gTasks[taskId].tWindowId = OakUI_ShowMultichoice(&sYesNoWindow, sChoices);
    gTasks[taskId].func = Task_RivalConfirmInput;
}

static void Task_RivalConfirmInput(u8 taskId)
{
    u8 windowId = gTasks[taskId].tWindowId;

    switch (Menu_ProcessInputNoWrapClearOnChoose())
    {
    case 0: /* YES: on to the farewell */
        OakUI_HideMultichoice(windowId);
        gTasks[taskId].func = Task_OakIntro_RivalConfirmed;
        break;
    case 1: /* NO: ask for a name again */
        OakUI_HideMultichoice(windowId);
        OakUI_ShowMessage(gText_OakIntro_AskRivalName);
        gTasks[taskId].func = Task_RivalSlideRight;
        break;
    }
}
