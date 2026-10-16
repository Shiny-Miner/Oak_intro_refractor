/*
 * Scene 1c: Rowan shows a Poke Ball. It rolls in, opens, the Pokemon hops out
 * (data from the ROM's own ball and Pokemon tables), then fades away.
 *
 *   Task_OakIntro_BallScene -> roll -> spawn Pokemon -> open + hop -> remove ball -> wait for the hop
 *   Task_OakIntro_BallFadeOut removes the Pokemon.
 *
 * While this runs, Rowan is ACTOR_F_OPAQUE so BLDALPHA only affects the Pokemon (see rowan.c).
 * The scene continues with Task_OakIntro_AfterBall (hop finished) and, after
 * Task_OakIntro_BallFadeOut, with Task_OakIntro_TellMe (both in oak_intro_rowan.c).
 */
#include "oak_intro.h"

#define BALL_SIZE 16
#define POKEMON_SIZE 64
/* feet level with the bottom of the ball, so it looks like it leaves the ball: 0x75 + 8 - 32 = 0x5D */
#define POKEMON_START_Y (BALL_START_Y + BALL_SIZE / 2 - POKEMON_SIZE / 2)

#define BALL_ROLL_FRAMES 0x20      /* ball rolls (1 px/frame) before the Pokemon sprite is created */
#define BALL_OPEN_AT 51            /* frames after the ball appears at which its lid opens */
#define BALL_RELEASE_WAIT 0x23     /* from Pokemon creation to the hop */
#define BALL_REMOVE_WAIT 0x20      /* from the hop to removing the ball */

/* Pokemon jump state, kept in sprite.data[] */
#define sPhase data[0]    /* 0 waiting, 1 up, 2 down, 3 done */
#define sTimer data[1]
#define sStepper data[2]
#define sExp data[3]

/* ---- ball: closed for BALL_OPEN_AT frames, then the open frame */
static const struct AnimCmd sAnim_Ball[] = {
    ANIMCMD_FRAME(0, BALL_OPEN_AT), ANIMCMD_FRAME(4, 10), ANIMCMD_END,
};
static const struct AnimCmd *const sAnims_Ball[] = { sAnim_Ball };

static const struct AffineAnimCmd sAffineAnim_BallRoll[] = {
    AFFINEANIMCMD_FRAME(0, 0, 0x18, 0x10),
    AFFINEANIMCMD_FRAME(0, 0, 0x08, 0x10),
    AFFINEANIMCMD_END,
};
static const struct AffineAnimCmd *const sAffineAnims_BallRoll[] = { sAffineAnim_BallRoll };

static void SpriteCB_Ball(struct Sprite *sprite)
{
    if (sprite->pos1.x > BALL_STOP_X)
        sprite->pos1.x--;
}

static const struct SpriteTemplate sBallTemplate = {
    .tileTag = 0xD6D8,
    .paletteTag = 0xD6D8,
    .oam = &gBallOamData,
    .anims = sAnims_Ball,
    .images = NULL,
    .affineAnims = sAffineAnims_BallRoll,
    .callback = SpriteCB_Ball,
};

/* ---- Pokemon: 64x64, affine + semi-transparent, hops once */
static const struct OamData sOamData_Pokemon = {
    .affineMode = 1,
    .objMode = ST_OAM_OBJ_BLEND,
    .shape = 0,
    .size = 3,
    .priority = 1,
};

static void SpriteCB_Pokemon(struct Sprite *sprite)
{
    switch (sprite->sPhase)
    {
    case 1:
        if (sprite->sTimer)
        {
            sprite->pos1.y -= sprite->sExp;
            if (++sprite->sStepper == 5)
            {
                sprite->pos1.x += 7 - sprite->sExp;
                sprite->sExp++;
                sprite->sStepper = 0;
            }
            sprite->sTimer--;
        }
        else
        {
            sprite->sPhase++;
            sprite->sStepper = 0;
            sprite->sExp = 1;
            sprite->sTimer = 0x15;
        }
        break;
    case 2:
        if (sprite->sTimer)
        {
            sprite->pos1.y += sprite->sExp;
            if (++sprite->sStepper == 5)
            {
                sprite->pos1.x++;
                sprite->sExp++;
                sprite->sStepper = 0;
            }
            sprite->sTimer--;
        }
        else
        {
            sprite->sPhase++;
        }
        break;
    }
}

static const struct SpriteTemplate sPokemonTemplate = {
    .tileTag = OAK_POKEMON,
    .paletteTag = OAK_POKEMON,
    .oam = &sOamData_Pokemon,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gUnk_08234944,
    .callback = SpriteCB_Pokemon,
};

static void Task_Roll(u8 taskId);
static void Task_SpawnPokemon(u8 taskId);
static void Task_Release(u8 taskId);
static void Task_RemoveBall(u8 taskId);
static void Task_WaitHop(u8 taskId);

void Task_OakIntro_BallScene(u8 taskId)
{
    HideMessageBox();
    LoadCompressedSpriteSheet(&gBallSpriteSheets[0]);
    LoadCompressedSpritePalette(&gBallSpritePalettes[0]);
    gOak->ballSprite = CreateSprite(&sBallTemplate, BALL_START_X, BALL_START_Y, 1);

    SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT2_ALL_BG);
    OakUI_Blend_Set(0, 16); /* the Pokemon is not visible yet */
    gTasks[taskId].tTimer = BALL_ROLL_FRAMES;
    gTasks[taskId].func = Task_Roll;
}

static void Task_Roll(u8 taskId)
{
    if (gTasks[taskId].tTimer-- > 0)
        return;
    gTasks[taskId].func = Task_SpawnPokemon;
}

static void Task_SpawnPokemon(u8 taskId)
{
    struct Sprite *ball = &gSprites[gOak->ballSprite];

    LoadCompressedSpriteSheet(&gMonFrontPicTable[OAK_POKEMON]);
    LoadCompressedSpritePalette(&gMonPaletteTable[OAK_POKEMON]);
    gOak->monSprite = CreateSprite(&sPokemonTemplate, ball->pos1.x, POKEMON_START_Y, 1);
    gSprites[gOak->monSprite].sPhase = 0;
    gTasks[taskId].tTimer = BALL_RELEASE_WAIT;
    gTasks[taskId].func = Task_Release;
}

static void Task_Release(u8 taskId)
{
    struct Sprite *ball = &gSprites[gOak->ballSprite];
    struct Sprite *mon = &gSprites[gOak->monSprite];

    if (gTasks[taskId].tTimer-- > 0)
        return;
    mon->sPhase = 1;
    mon->sTimer = 0x15;
    mon->sExp = 1;
    mon->sStepper = 0;
    sub_804B8C8(ball->pos1.x, ball->pos1.y, 0, 0); /* particles */
    sub_804B8F0(1, 0, 0xFF);                        /* flash */
    OakUI_Blend_Set(16, 0);                         /* Pokemon appears */
    gTasks[taskId].tTimer = BALL_REMOVE_WAIT;
    gTasks[taskId].func = Task_RemoveBall;
}

static void Task_RemoveBall(u8 taskId)
{
    if (gTasks[taskId].tTimer-- > 0)
        return;
    DestroySpriteAndFreeResources(&gSprites[gOak->ballSprite]);
    gOak->ballSprite = SPRITE_NONE;
    gTasks[taskId].func = Task_WaitHop;
}

static void Task_WaitHop(u8 taskId)
{
    if (gSprites[gOak->monSprite].sPhase < 3)
        return;
    gTasks[taskId].func = Task_OakIntro_AfterBall;
}

/* ---------------------------------------------------------------- Pokemon leaves */

static void Task_FadeOutPokemon(u8 taskId)
{
    if (!OakUI_Blend_Step(0))
        return;
    DestroySpriteAndFreeResources(&gSprites[gOak->monSprite]);
    gOak->monSprite = SPRITE_NONE;
    gTasks[taskId].func = Task_OakIntro_TellMe;
}

void Task_OakIntro_BallFadeOut(u8 taskId)
{
    gTasks[taskId].func = Task_FadeOutPokemon;
}
