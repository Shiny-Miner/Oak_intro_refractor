/*
 * Scene 2: boy or girl. Both actors fade in, the player moves the highlight with LEFT/RIGHT,
 * A confirms, the other actor fades out, YES/NO asks for confirmation.
 */
#include "oak_intro.h"

#define BLEND_HALF_EVA 3
#define BLEND_HALF_EVB 10

static const struct WindowTemplate sYesNoWindow = { 0, 2, 2, 4, 4, 0xF, 0x130 };

static void Task_FadeInBoy(u8 taskId);
static void Task_ShowGirl(u8 taskId);
static void Task_FadeInGirl(u8 taskId);
static void Task_Chooser(u8 taskId);
static void Task_SlideChosen(u8 taskId);
static void Task_Confirm(u8 taskId);
static void Task_ConfirmInput(u8 taskId);
static void Task_RetryFade(u8 taskId);
static void Task_RetryFree(u8 taskId);
static void Task_Accepted(u8 taskId);

static struct Actor *Chosen(void)
{
    return gOak->gender == FEMALE ? &gOak->girl : &gOak->boy;
}

static struct Actor *Other(void)
{
    return gOak->gender == FEMALE ? &gOak->boy : &gOak->girl;
}

/*
 * Only the highlighted actor runs (512 tiles); the other one is a still frame (128 tiles).
 * Moving the highlight therefore recreates both.
 */
static void BuildPair(void)
{
    Actor_Destroy(&gOak->boy);
    Actor_Destroy(&gOak->girl);
    if (gOak->gender == FEMALE)
    {
        Actor_Create(&gOak->girl, ACTOR_GIRL_RUN, ACTOR_LEFT_X, ACTOR_Y);
        Actor_Create(&gOak->boy, ACTOR_BOY_STILL, ACTOR_RIGHT_X, ACTOR_Y);
    }
    else
    {
        Actor_Create(&gOak->boy, ACTOR_BOY_RUN, ACTOR_RIGHT_X, ACTOR_Y);
        Actor_Create(&gOak->girl, ACTOR_GIRL_STILL, ACTOR_LEFT_X, ACTOR_Y);
    }
    Actor_SetFlag(Chosen(), ACTOR_F_OPAQUE, TRUE);
}

void Task_OakIntro_ChooseGender(u8 taskId)
{
    gOak->gender = MALE;
    Actor_Create(&gOak->boy, ACTOR_BOY_RUN, ACTOR_RIGHT_X, ACTOR_Y);
    SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT2_ALL_BG);
    OakUI_Blend_Set(0, 16);
    gTasks[taskId].func = Task_FadeInBoy;
}

static void Task_FadeInBoy(u8 taskId)
{
    if (!OakUI_Blend_Step(16))
        return;
    /*
     * Make the boy opaque, then wait one frame: this lets his callback run once before the
     * blend registers are reset, which avoids a one-frame flash.
     */
    Actor_SetFlag(&gOak->boy, ACTOR_F_OPAQUE, TRUE);
    gTasks[taskId].func = Task_ShowGirl;
}

static void Task_ShowGirl(u8 taskId)
{
    OakUI_Blend_Set(0, 16);
    Actor_Create(&gOak->girl, ACTOR_GIRL_STILL, ACTOR_LEFT_X, ACTOR_Y);
    gTasks[taskId].func = Task_FadeInGirl;
}

static void Task_FadeInGirl(u8 taskId)
{
    if (!OakUI_Blend_Step(16))
        return;
    gTasks[taskId].tTimer = 0x1E;
    gTasks[taskId].func = Task_Chooser;
}

static void Task_Chooser(u8 taskId)
{
    struct Task *task = &gTasks[taskId];
    struct OakIntro *oak = gOak;

    if (task->tTimer > 0)
        task->tTimer--;
    if (task->tTimer == 1)
        OakUI_Blend_Set(BLEND_HALF_EVA, BLEND_HALF_EVB); /* dim whoever is not chosen */

    if (gMain.newKeys & (DPAD_LEFT | DPAD_RIGHT))
    {
        oak->gender = !oak->gender;
        BuildPair();
        PlaySE(OAK_SE_CHOICE);
    }

    /* the highlighted actor walks in place and is fully visible, the other one stands dimmed */
    Actor_SetFlag(Chosen(), ACTOR_F_ANIMATE | ACTOR_F_OPAQUE, TRUE);
    Actor_SetFlag(Other(), ACTOR_F_ANIMATE | ACTOR_F_OPAQUE, FALSE);

    if (gMain.newKeys & A_BUTTON)
    {
        Actor_SetFlag(Chosen(), ACTOR_F_ANIMATE, FALSE);
        Actor_SlideTo(Chosen(), ACTOR_CENTER_X);
        oak->blendEva = BLEND_HALF_EVA;
        oak->blendTimer = FADE_STEPS;
        task->func = Task_SlideChosen;
    }
}

static void Task_SlideChosen(u8 taskId)
{
    /* fade the other one out while the chosen one slides to the middle */
    bool8 faded = OakUI_Blend_Step(0);

    if (Actor_IsSliding(Chosen()) || !faded)
        return;
    OakUI_ShowMessage(gOak->gender == FEMALE ? gText_OakIntro_ConfirmGirl : gText_OakIntro_ConfirmBoy);
    gTasks[taskId].func = Task_Confirm;
}

static void Task_Confirm(u8 taskId)
{
    static const u8 *const sChoices[] = { gText_OakIntro_Yes, gText_OakIntro_No, NULL };

    gTasks[taskId].tWindowId = OakUI_ShowMultichoice(&sYesNoWindow, sChoices);
    gTasks[taskId].func = Task_ConfirmInput;
}

static void Task_ConfirmInput(u8 taskId)
{
    u8 windowId = gTasks[taskId].tWindowId;

    switch (Menu_ProcessInputNoWrapClearOnChoose())
    {
    case 0: /* YES */
        OakUI_HideMultichoice(windowId);
        gTasks[taskId].func = Task_Accepted;
        break;
    case 1: /* NO: fade the chosen actor out and start over */
        OakUI_HideMultichoice(windowId);
        Actor_SetFlag(Chosen(), ACTOR_F_OPAQUE, FALSE);
        Actor_SetFlag(Other(), ACTOR_F_HIDDEN, TRUE);
        OakUI_Blend_Set(16, 0);
        gTasks[taskId].func = Task_RetryFade;
        break;
    }
}

static void Task_RetryFade(u8 taskId)
{
    if (!OakUI_Blend_Step(0))
        return;
    OakUI_Blend_Reset();
    gTasks[taskId].tTimer = 0x1E;
    gTasks[taskId].func = Task_RetryFree;
}

static void Task_RetryFree(u8 taskId)
{
    if (gTasks[taskId].tTimer != 0)
    {
        gTasks[taskId].tTimer--;
        return;
    }
    Actor_Destroy(&gOak->boy);
    Actor_Destroy(&gOak->girl);
    gTasks[taskId].func = Task_OakIntro_ChooseGender;
}

static void Task_Accepted(u8 taskId)
{
    gSaveBlock2Ptr->playerGender = gOak->gender;
    gTasks[taskId].func = Task_OakIntro_AskName;
}
