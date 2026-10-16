#ifndef GUARD_OAK_INTRO_H
#define GUARD_OAK_INTRO_H

#include "global.h"

/* How the setup callback should resume. */
enum OakEntry
{
    OAK_ENTRY_NEW,            /* start of the speech (hook or script) */
    OAK_ENTRY_PLAYER_NAMED,   /* back from the player's naming screen */
    OAK_ENTRY_RIVAL_NAMED,    /* back from the rival's naming screen  */
};

/* One 64x128 actor = two stacked 64x64 sprites. */
struct Actor
{
    u8 top;
    u8 bottom;
};

enum ActorKind
{
    ACTOR_BOY_RUN,
    ACTOR_GIRL_RUN,
    ACTOR_BOY_STILL,
    ACTOR_GIRL_STILL,
    ACTOR_BOY_SHRINK,
    ACTOR_GIRL_SHRINK,
    ACTOR_PROF,
    ACTOR_RIVAL,
};

/* Actor flags (sprite.data[0]) */
#define ACTOR_F_ANIMATE (1 << 0)  /* play the running animation, otherwise freeze the frame */
#define ACTOR_F_OPAQUE (1 << 1)   /* ignore the sprite blend (always fully visible)        */
#define ACTOR_F_HIDDEN (1 << 2)

struct OakIntro
{
    u8 entry;               /* enum OakEntry */
    u8 gender;              /* gender highlighted on the boy/girl screen */
    u8 windowId;            /* multichoice window currently open */
    u8 blendEva;            /* current sprite blend weight 0..16 */
    u8 blendTimer;
    u8 ballSprite;
    u8 monSprite;
    u8 setupState;          /* step of CB2_OakIntro_Setup (own counter, not gMain.state) */
    struct Actor boy;
    struct Actor girl;
    struct Actor npc;       /* professor, rival or the confirmed player */
    u16 *bg1Map;
    u16 *bg2Map;
};

/* vanilla task the new-game flow continues with (BPRE.ld) */
void Task_OakSpeech_Continue(u8 taskId);
void VBlankCB_OakSpeech(void);
void CB2_OakSpeech(void);

#define gOak (*(struct OakIntro **)OAK_STATE_PTR_ADDR)

/* oak_intro.c */
void OakIntro_Hook(u8 taskId);
void CB2_OakIntro_Setup(void);
void Task_OakIntro_Begin(u8 taskId);

/* oak_intro_ui.c : message box, multichoice, fades */
void OakUI_ShowMessage(const u8 *str);
u8 OakUI_ShowMultichoice(const struct WindowTemplate *template, const u8 *const *choices);
void OakUI_HideMultichoice(u8 windowId);
void OakUI_Blend_Set(u8 eva, u8 evb);
bool8 OakUI_Blend_Step(u8 targetEva);   /* TRUE when targetEva has been reached */
void OakUI_Blend_Reset(void);
void OakUI_FadeToBlack(void);
void OakUI_FadeFromBlack(s8 delay);
u16 OakUI_RandomBelow(u16 n);          /* 0..n-1, no division */

/* oak_intro_actor.c */
void Actor_Create(struct Actor *actor, enum ActorKind kind, s16 x, s16 y);
void Actor_Destroy(struct Actor *actor);
void Actor_SetFlag(struct Actor *actor, u16 flag, bool8 on);
void Actor_SlideTo(struct Actor *actor, s16 x);
bool8 Actor_IsSliding(const struct Actor *actor);
bool8 Actor_Exists(const struct Actor *actor);
/* Fade the current gOak->npc out (if any), replace it with `kind`, fade that in, then run `next`. */
void OakIntro_SwapNpc(u8 taskId, enum ActorKind kind, TaskFunc next);
/* Fade the current gOak->npc out and remove it, then run `next`. */
void OakIntro_FadeOutNpc(u8 taskId, TaskFunc next);
enum ActorKind OakIntro_PlayerStillKind(void);

/* Chain of screens: each Task_ function hands over to the next */
void Task_OakIntro_ShowBall(u8 taskId);          /* oak_intro_ball.c   */
void Task_OakIntro_RowanIntroduce(u8 taskId);    /* oak_intro_rowan.c  */
void Task_OakIntro_BallScene(u8 taskId);         /* oak_intro_ball.c   */
void Task_OakIntro_BallFadeOut(u8 taskId);
void Task_OakIntro_AfterBall(u8 taskId);         /* back in oak_intro_rowan.c */
void Task_OakIntro_TellMe(u8 taskId);
void Task_OakIntro_ChooseGender(u8 taskId);      /* oak_intro_gender.c */
void Task_OakIntro_AskName(u8 taskId);           /* oak_intro_name.c   */
void Task_OakIntro_ResumeAfterPlayerName(u8 taskId);
void Task_OakIntro_ResumeAfterRivalName(u8 taskId);
void Task_OakIntro_RivalConfirmed(u8 taskId);    /* oak_intro_finish.c */
void Task_OakIntro_Dummy(u8 taskId);

/* Task data slots used by the intro (the vanilla helpers at OakSpeech_FadeBg* use 0-5). */
#define tTimer data[6]
#define tWindowId data[13]

#endif // GUARD_OAK_INTRO_H
