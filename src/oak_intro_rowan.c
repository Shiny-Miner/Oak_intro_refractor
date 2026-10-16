/*
 * Scene 1b: Rowan's speech.
 *   introduction -> advice menu (Rowan steps aside, any choice continues)
 *   -> "this world..." -> Poke Ball + Pokemon (oak_intro_ball.c) -> "we live alongside Pokemon..."
 *   -> Pokemon fades -> "tell me about yourself" -> "boy or girl?" -> Rowan fades out
 *   -> boy/girl chooser (oak_intro_gender.c)
 */
#include "oak_intro.h"

static const struct WindowTemplate sAdviceWindow = { 0, 2, 2, 12, 6, 0xF, 0x130 };

static void Task_AdviceSlideAside(u8 taskId);
static void Task_AdviceShowMenu(u8 taskId);
static void Task_AdviceInput(u8 taskId);
static void Task_SayWait(u8 taskId);
static void Task_PokeballText(u8 taskId);
static void Task_PokemonLeaves(u8 taskId);
static void Task_BoyOrGirl(u8 taskId);
static void Task_RowanLeaves(u8 taskId);

/* "next" is kept in data[2..3] (data[0..1] belong to the swap helpers in oak_intro_actor.c) */
static void Say(u8 taskId, const u8 *text, TaskFunc next)
{
    struct Task *task = &gTasks[taskId];

    OakUI_ShowMessage(text);
    task->data[2] = (u32)next & 0xFFFF;
    task->data[3] = (u32)next >> 16;
    task->func = Task_SayWait;
}

static void Task_SayWait(u8 taskId)
{
    struct Task *task = &gTasks[taskId];

    if (IsTextPrinterActive(WINDOW_MESSAGE))
        return;
    task->func = (TaskFunc)(((u32)(u16)task->data[3] << 16) | (u16)task->data[2]);
}

void Task_OakIntro_RowanIntroduce(u8 taskId)
{
    Say(taskId, gText_OakIntro_Introduce, Task_AdviceSlideAside);
}

static void Task_AdviceSlideAside(u8 taskId)
{
    Actor_SlideTo(&gOak->npc, ACTOR_RIGHT_X); /* room for the menu on the left; he stays there */
    gTasks[taskId].func = Task_AdviceShowMenu;
}

static void Task_AdviceShowMenu(u8 taskId)
{
    static const u8 *const sChoices[] = {
        gText_OakIntro_InfoControls, gText_OakIntro_InfoAdventure, gText_OakIntro_InfoNone, NULL,
    };

    if (Actor_IsSliding(&gOak->npc))
        return;
    gTasks[taskId].tWindowId = OakUI_ShowMultichoice(&sAdviceWindow, sChoices);
    gTasks[taskId].func = Task_AdviceInput;
}

/* The original treated all three entries the same, so they do too. Hook the info screens in here. */
static void Task_AdviceInput(u8 taskId)
{
    if (Menu_ProcessInputNoWrapClearOnChoose() < 0)
        return;
    OakUI_HideMultichoice(gTasks[taskId].tWindowId);
    gTasks[taskId].func = Task_PokeballText;
}

static void Task_PokeballText(u8 taskId)
{
    /* Rowan must ignore the blend registers while the Pokemon fades in and out */
    Actor_SetFlag(&gOak->npc, ACTOR_F_OPAQUE, TRUE);
    Say(taskId, gText_OakIntro_Pokeball, Task_OakIntro_BallScene);
}

/* The ball scene continues here once the Pokemon has landed. */
void Task_OakIntro_AfterBall(u8 taskId)
{
    Say(taskId, gText_OakIntro_FirstSpeech, Task_PokemonLeaves);
}

static void Task_PokemonLeaves(u8 taskId)
{
    Task_OakIntro_BallFadeOut(taskId); /* continues with Task_OakIntro_TellMe */
}

void Task_OakIntro_TellMe(u8 taskId)
{
    Say(taskId, gText_OakIntro_TellMe, Task_BoyOrGirl);
}

static void Task_BoyOrGirl(u8 taskId)
{
    Say(taskId, gText_OakIntro_BoyOrGirl, Task_RowanLeaves);
}

static void Task_RowanLeaves(u8 taskId)
{
    HideMessageBox();
    Actor_SetFlag(&gOak->npc, ACTOR_F_OPAQUE, FALSE);
    OakIntro_FadeOutNpc(taskId, Task_OakIntro_ChooseGender);
}
