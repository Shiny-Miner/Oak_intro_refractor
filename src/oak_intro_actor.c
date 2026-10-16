/*
 * "Actors" are the big 64x128 characters: two 64x64 sprites on top of each other that share
 * one tile sheet and palette. All images come from the PNGs in graphics/ (see build/gen/graphics.h).
 *
 * sprite.data[]: [0] flags (ACTOR_F_*), [1] x to slide to or ACTOR_NO_TARGET, [2] frozen frame
 */
#include "oak_intro.h"

#define ACTOR_NO_TARGET 0x7FFF
#define ACTOR_HEIGHT_HALF 0x40

/* ---- animations. The 4-frame sheets hold two 64x64 halves per frame: top at +0x00, bottom at +0x40 tiles. */
#define RUN_STEP 10
/* frames each shrink pose stays (0x15 = 21 ticks, 4 poses = 84 frames; oak_intro_finish.c waits the same) */
#define SHRINK_STEP 0x15

static const struct AnimCmd sAnim_RunTop[] = {
    ANIMCMD_FRAME(0x000, RUN_STEP), ANIMCMD_FRAME(0x100, RUN_STEP),
    ANIMCMD_FRAME(0x080, RUN_STEP), ANIMCMD_FRAME(0x180, RUN_STEP), ANIMCMD_JUMP(0),
};
static const struct AnimCmd sAnim_RunBottom[] = {
    ANIMCMD_FRAME(0x040, RUN_STEP), ANIMCMD_FRAME(0x140, RUN_STEP),
    ANIMCMD_FRAME(0x0C0, RUN_STEP), ANIMCMD_FRAME(0x1C0, RUN_STEP), ANIMCMD_JUMP(0),
};
/*
 * Dawn's sheet is laid out the other way round from Lucas's: her frames 0/1 (left/right foot) are the
 * "up" bob and 2/3 the "down" bob. A step is down -> up, so her order is 2, 0, 3, 1.
 */
static const struct AnimCmd sAnim_RunTopGirl[] = {
    ANIMCMD_FRAME(0x100, RUN_STEP), ANIMCMD_FRAME(0x000, RUN_STEP),
    ANIMCMD_FRAME(0x180, RUN_STEP), ANIMCMD_FRAME(0x080, RUN_STEP), ANIMCMD_JUMP(0),
};
static const struct AnimCmd sAnim_RunBottomGirl[] = {
    ANIMCMD_FRAME(0x140, RUN_STEP), ANIMCMD_FRAME(0x040, RUN_STEP),
    ANIMCMD_FRAME(0x1C0, RUN_STEP), ANIMCMD_FRAME(0x0C0, RUN_STEP), ANIMCMD_JUMP(0),
};
static const struct AnimCmd sAnim_StillTop[] = { ANIMCMD_FRAME(0x00, 1), ANIMCMD_END };
static const struct AnimCmd sAnim_StillBottom[] = { ANIMCMD_FRAME(0x40, 1), ANIMCMD_END };
static const struct AnimCmd sAnim_ShrinkTop[] = {
    ANIMCMD_FRAME(0x000, SHRINK_STEP), ANIMCMD_FRAME(0x080, SHRINK_STEP),
    ANIMCMD_FRAME(0x100, SHRINK_STEP), ANIMCMD_FRAME(0x180, SHRINK_STEP), ANIMCMD_END,
};
static const struct AnimCmd sAnim_ShrinkBottom[] = {
    ANIMCMD_FRAME(0x040, SHRINK_STEP), ANIMCMD_FRAME(0x0C0, SHRINK_STEP),
    ANIMCMD_FRAME(0x140, SHRINK_STEP), ANIMCMD_FRAME(0x1C0, SHRINK_STEP), ANIMCMD_END,
};

static const struct AnimCmd *const sAnims_RunTop[] = { sAnim_RunTop };
static const struct AnimCmd *const sAnims_RunBottom[] = { sAnim_RunBottom };
static const struct AnimCmd *const sAnims_RunTopGirl[] = { sAnim_RunTopGirl };
static const struct AnimCmd *const sAnims_RunBottomGirl[] = { sAnim_RunBottomGirl };
static const struct AnimCmd *const sAnims_StillTop[] = { sAnim_StillTop };
static const struct AnimCmd *const sAnims_StillBottom[] = { sAnim_StillBottom };
static const struct AnimCmd *const sAnims_ShrinkTop[] = { sAnim_ShrinkTop };
static const struct AnimCmd *const sAnims_ShrinkBottom[] = { sAnim_ShrinkBottom };

/* 64x64, semi-transparent so the blend registers can fade it. */
static const struct OamData sOamData_Actor = {
    .objMode = ST_OAM_OBJ_BLEND,
    .shape = 0,
    .size = 3,
    .priority = 2,
};

static void SpriteCB_Actor(struct Sprite *sprite)
{
    u16 flags = sprite->data[0];
    s16 target = sprite->data[1];

    sprite->oam.objMode = (flags & ACTOR_F_OPAQUE) ? ST_OAM_OBJ_NORMAL : ST_OAM_OBJ_BLEND;
    sprite->invisible = (flags & ACTOR_F_HIDDEN) != 0;

    /* freeze the walking animation by restoring the frame we were on */
    if (flags & ACTOR_F_ANIMATE)
        sprite->data[2] = sprite->animCmdIndex;
    else
        sprite->animCmdIndex = sprite->data[2];

    if (target != ACTOR_NO_TARGET)
    {
        s16 diff = target - sprite->pos1.x;

        if (diff > -SLIDE_STEP && diff < SLIDE_STEP)
        {
            sprite->pos1.x = target;
            sprite->data[1] = ACTOR_NO_TARGET;
        }
        else
        {
            sprite->pos1.x += (diff > 0) ? SLIDE_STEP : -SLIDE_STEP;
        }
    }
}

/*
 * A running sheet is 4 frames x 128 tiles = 512 tiles, and OBJ VRAM holds only 1024. Two of them
 * (boy + girl) do not fit, so the actor that is not animating only loads frame 0 (128 tiles).
 */
#define STILL_SHEET_SIZE 0x1000

/*
 * The ROM's CreateSprite() does NOT copy the template, it stores the pointer (sprite->template) and
 * DestroySpriteAndFreeResources() reads template->tileTag from it again to free the tile sheet.
 * So templates must live forever (static const), never on the stack. A stack template made the
 * destroy free the wrong sheet (or none), VRAM filled up with leaked sheets, and the big 512-tile
 * sheets (girl running, both shrink sheets) then failed to load -> garbage tiles.
 */
#define ACTOR_TEMPLATE(tag, anims) { tag, tag, &sOamData_Actor, anims, NULL, gDummySpriteAffineAnimTable, SpriteCB_Actor }

struct ActorDef
{
    const u32 *tiles;
    u16 tilesSize;
    const u16 *palette;
    u16 tag;
    const struct SpriteTemplate *top;
    const struct SpriteTemplate *bottom;
    u16 flags;
};

#define DEFINE_ACTOR_TEMPLATES(name, tag, animsTop, animsBottom) \
    static const struct SpriteTemplate sTemplate_##name##_Top = ACTOR_TEMPLATE(tag, animsTop); \
    static const struct SpriteTemplate sTemplate_##name##_Bottom = ACTOR_TEMPLATE(tag, animsBottom)

DEFINE_ACTOR_TEMPLATES(BoyRun, 0x1000, sAnims_RunTop, sAnims_RunBottom);
DEFINE_ACTOR_TEMPLATES(GirlRun, 0x1001, sAnims_RunTopGirl, sAnims_RunBottomGirl);
DEFINE_ACTOR_TEMPLATES(BoyStill, 0x1000, sAnims_StillTop, sAnims_StillBottom);
DEFINE_ACTOR_TEMPLATES(GirlStill, 0x1001, sAnims_StillTop, sAnims_StillBottom);
DEFINE_ACTOR_TEMPLATES(BoyShrink, 0x1002, sAnims_ShrinkTop, sAnims_ShrinkBottom);
DEFINE_ACTOR_TEMPLATES(GirlShrink, 0x1003, sAnims_ShrinkTop, sAnims_ShrinkBottom);
DEFINE_ACTOR_TEMPLATES(Prof, 0x0300, sAnims_StillTop, sAnims_StillBottom);
DEFINE_ACTOR_TEMPLATES(Rival, 0x0500, sAnims_StillTop, sAnims_StillBottom);

#define ACTOR_DEF(kind, name, tiles, size, pal, tag, flags) \
    [kind] = { tiles, size, pal, tag, &sTemplate_##name##_Top, &sTemplate_##name##_Bottom, flags }

static const struct ActorDef sActorDefs[] = {
    ACTOR_DEF(ACTOR_BOY_RUN, BoyRun, gGfx_lucas_Tiles, gGfx_lucas_TilesSize, gGfx_lucas_Pal, 0x1000, ACTOR_F_ANIMATE),
    ACTOR_DEF(ACTOR_GIRL_RUN, GirlRun, gGfx_dawn_Tiles, gGfx_dawn_TilesSize, gGfx_dawn_Pal, 0x1001, ACTOR_F_ANIMATE),
    ACTOR_DEF(ACTOR_BOY_STILL, BoyStill, gGfx_lucas_Tiles, STILL_SHEET_SIZE, gGfx_lucas_Pal, 0x1000, 0),
    ACTOR_DEF(ACTOR_GIRL_STILL, GirlStill, gGfx_dawn_Tiles, STILL_SHEET_SIZE, gGfx_dawn_Pal, 0x1001, 0),
    ACTOR_DEF(ACTOR_BOY_SHRINK, BoyShrink, gGfx_lucasshrink_Tiles, gGfx_lucasshrink_TilesSize, gGfx_lucasshrink_Pal, 0x1002, ACTOR_F_ANIMATE),
    ACTOR_DEF(ACTOR_GIRL_SHRINK, GirlShrink, gGfx_dawnshrink_Tiles, gGfx_dawnshrink_TilesSize, gGfx_dawnshrink_Pal, 0x1003, ACTOR_F_ANIMATE),
    ACTOR_DEF(ACTOR_PROF, Prof, gGfx_prof_Tiles, gGfx_prof_TilesSize, gGfx_prof_Pal, 0x0300, 0),
    ACTOR_DEF(ACTOR_RIVAL, Rival, gGfx_rival_Tiles, gGfx_rival_TilesSize, gGfx_rival_Pal, 0x0500, 0),
};

static u8 CreateHalf(const struct ActorDef *def, const struct SpriteTemplate *template, s16 x, s16 y)
{
    u8 id = CreateSprite(template, x, y, 1);

    if (id != SPRITE_NONE)
    {
        gSprites[id].data[0] = def->flags;
        gSprites[id].data[1] = ACTOR_NO_TARGET;
        gSprites[id].data[2] = 0;
    }
    return id;
}

void Actor_Create(struct Actor *actor, enum ActorKind kind, s16 x, s16 y)
{
    const struct ActorDef *def = &sActorDefs[kind];
    struct CompressedSpriteSheet sheet = { def->tiles, def->tilesSize, def->tag };
    struct SpritePalette palette = { def->palette, def->tag };

    LoadCompressedSpriteSheet(&sheet);
    LoadSpritePalette(&palette);
    actor->top = CreateHalf(def, def->top, x, y);
    actor->bottom = CreateHalf(def, def->bottom, x, y + ACTOR_HEIGHT_HALF);
}

bool8 Actor_Exists(const struct Actor *actor)
{
    return actor->top != SPRITE_NONE && gSprites[actor->top].inUse;
}

void Actor_Destroy(struct Actor *actor)
{
    if (actor->top != SPRITE_NONE && gSprites[actor->top].inUse)
        DestroySpriteAndFreeResources(&gSprites[actor->top]);
    if (actor->bottom != SPRITE_NONE && gSprites[actor->bottom].inUse)
        DestroySpriteAndFreeResources(&gSprites[actor->bottom]);
    actor->top = SPRITE_NONE;
    actor->bottom = SPRITE_NONE;
}

void Actor_SetFlag(struct Actor *actor, u16 flag, bool8 on)
{
    u8 ids[2] = { actor->top, actor->bottom };
    int i;

    for (i = 0; i < 2; i++)
    {
        if (ids[i] == SPRITE_NONE)
            continue;
        if (on)
            gSprites[ids[i]].data[0] |= flag;
        else
            gSprites[ids[i]].data[0] &= ~flag;
    }
}

void Actor_SlideTo(struct Actor *actor, s16 x)
{
    gSprites[actor->top].data[1] = x;
    gSprites[actor->bottom].data[1] = x;
}

bool8 Actor_IsSliding(const struct Actor *actor)
{
    return gSprites[actor->top].data[1] != ACTOR_NO_TARGET;
}

/* ---------------------------------------------------------------- swapping the centre actor */

/* task.data[0..1] = next TaskFunc, data[2] = ActorKind to bring in */

static TaskFunc GetNext(const struct Task *task)
{
    return (TaskFunc)(((u32)(u16)task->data[1] << 16) | (u16)task->data[0]);
}

static void Task_SwapIn(u8 taskId)
{
    if (!OakUI_Blend_Step(16))
        return;
    gTasks[taskId].func = GetNext(&gTasks[taskId]);
}

static void SetNext(struct Task *task, TaskFunc next)
{
    task->data[0] = (u32)next & 0xFFFF;
    task->data[1] = (u32)next >> 16;
}

static void Task_FadeOutNpc(u8 taskId)
{
    if (!OakUI_Blend_Step(0))
        return;
    Actor_Destroy(&gOak->npc);
    gTasks[taskId].func = GetNext(&gTasks[taskId]);
}

void OakIntro_FadeOutNpc(u8 taskId, TaskFunc next)
{
    SetNext(&gTasks[taskId], next);
    OakUI_Blend_Set(16, 0);
    gTasks[taskId].func = Task_FadeOutNpc;
}

static void BringIn(u8 taskId)
{
    SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT2_ALL_BG);
    Actor_Create(&gOak->npc, (enum ActorKind)gTasks[taskId].data[2], ACTOR_CENTER_X, ACTOR_Y);
    OakUI_Blend_Set(0, 16);
    gTasks[taskId].func = Task_SwapIn;
}

static void Task_SwapOut(u8 taskId)
{
    if (!OakUI_Blend_Step(0))
        return;
    Actor_Destroy(&gOak->npc);
    BringIn(taskId);
}

void OakIntro_SwapNpc(u8 taskId, enum ActorKind kind, TaskFunc next)
{
    struct Task *task = &gTasks[taskId];

    SetNext(task, next);
    task->data[2] = kind;
    if (Actor_Exists(&gOak->npc))
    {
        OakUI_Blend_Set(16, 0);
        task->func = Task_SwapOut;
    }
    else
    {
        BringIn(taskId);
    }
}

enum ActorKind OakIntro_PlayerStillKind(void)
{
    return gSaveBlock2Ptr->playerGender == FEMALE ? ACTOR_GIRL_STILL : ACTOR_BOY_STILL;
}
