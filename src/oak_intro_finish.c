/*
 * Scene 4: the rival is confirmed, the professor says goodbye, the player shrinks into
 * the next scene and control goes back to the vanilla new-game flow.
 */
#include "oak_intro.h"

#define SHRINK_FRAMES (0x15 * 4) /* four frames of 0x15 ticks, see sAnim_Shrink* */

static void Task_FinalSpeech(u8 taskId);
static void Task_FinalSpeechWait(u8 taskId);
static void Task_ShowShrink(u8 taskId);
static void Task_Shrink(u8 taskId);
static void Task_HandOver(u8 taskId);

void Task_OakIntro_RivalConfirmed(u8 taskId)
{
    OakIntro_SwapNpc(taskId, ACTOR_PROF, Task_FinalSpeech);
}

static void Task_FinalSpeech(u8 taskId)
{
    OakUI_ShowMessage(gText_OakIntro_FinalSpeech);
    gTasks[taskId].func = Task_FinalSpeechWait;
}

static void Task_FinalSpeechWait(u8 taskId)
{
    if (IsTextPrinterActive(WINDOW_MESSAGE))
        return;
    HideMessageBox();
    FadeOutBGM(4);
    /* the professor fades out while the player fades in */
    OakIntro_SwapNpc(taskId, OakIntro_PlayerStillKind(), Task_ShowShrink);
}

static void Task_ShowShrink(u8 taskId)
{
    enum ActorKind kind = gSaveBlock2Ptr->playerGender == FEMALE ? ACTOR_GIRL_SHRINK : ACTOR_BOY_SHRINK;

    Actor_Destroy(&gOak->npc);
    Actor_Create(&gOak->npc, kind, ACTOR_CENTER_X, ACTOR_Y);
    gTasks[taskId].tTimer = SHRINK_FRAMES;
    gTasks[taskId].func = Task_Shrink;
}

static void Task_Shrink(u8 taskId)
{
    if (gTasks[taskId].tTimer-- > 0)
        return;
    BeginNormalPaletteFade(PALETTES_ALL, 2, 0, 16, RGB_BLACK);
    gTasks[taskId].func = Task_HandOver;
}

static void Task_HandOver(u8 taskId)
{
    if (PaletteFadeActive())
        return;
    /*
     * Not freed on purpose: the BG tilemap buffers in gOak are still registered with the BG
     * engine and the vanilla code that takes over may touch them (about 4 KB of heap).
     */
    gTasks[taskId].func = Task_OakSpeech_Continue;
}
