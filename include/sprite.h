#ifndef GUARD_SPRITE_H
#define GUARD_SPRITE_H

#include "types.h"

#define ST_OAM_OBJ_NORMAL 0
#define ST_OAM_OBJ_BLEND 1

struct OamData
{
    u32 y : 8;
    u32 affineMode : 2;
    u32 objMode : 2;
    u32 mosaic : 1;
    u32 bpp : 1;
    u32 shape : 2;
    u32 x : 9;
    u32 matrixNum : 5;
    u32 size : 2;
    u32 tileNum : 10;
    u32 priority : 2;
    u32 paletteNum : 4;
    u32 affineParam : 16;
};

/* One animation command: { image, duration } for a frame, or { -2, target } to jump, { -1, 0 } to end. */
struct AnimCmd
{
    s16 a;
    s16 b;
};
#define ANIMCMD_FRAME(image, duration) { image, duration }
#define ANIMCMD_JUMP(target) { -2, target }
#define ANIMCMD_END { -1, 0 }

/* 8 bytes per command, as the ROM expects. */
struct AffineAnimCmd
{
    s16 xScale;
    s16 yScale;
    u8 rotation;
    u8 duration;
    u16 padding;
};
#define AFFINEANIMCMD_FRAME(x, y, rot, duration) { x, y, rot, duration, 0 }
#define AFFINEANIMCMD_END { 0x7FFF, 0, 0, 0, 0 }

struct SpriteFrameImage;
struct Sprite;
typedef void (*SpriteCallback)(struct Sprite *);

struct SpriteTemplate
{
    u16 tileTag;
    u16 paletteTag;
    const struct OamData *oam;
    const struct AnimCmd *const *anims;
    const struct SpriteFrameImage *images;
    const struct AffineAnimCmd *const *affineAnims;
    SpriteCallback callback;
};

struct CompressedSpriteSheet
{
    const u32 *data;
    u16 size; // decompressed size
    u16 tag;
};

struct SpritePalette
{
    const u16 *data;
    u16 tag;
};

struct CompressedSpritePalette
{
    const u32 *data;
    u16 tag;
};

struct Coords16
{
    s16 x;
    s16 y;
};

struct Sprite
{
    /*0x00*/ struct OamData oam;
    /*0x08*/ const struct AnimCmd *const *anims;
    /*0x0C*/ const struct SpriteFrameImage *images;
    /*0x10*/ const struct AffineAnimCmd *const *affineAnims;
    /*0x14*/ const struct SpriteTemplate *template;
    /*0x18*/ const void *subspriteTables;
    /*0x1C*/ SpriteCallback callback;
    /*0x20*/ struct Coords16 pos1;
    /*0x24*/ struct Coords16 pos2;
    /*0x28*/ s8 centerToCornerVecX;
    /*0x29*/ s8 centerToCornerVecY;
    /*0x2A*/ u8 animNum;
    /*0x2B*/ u8 animCmdIndex;
    /*0x2C*/ u8 animDelayCounter;
    /*0x2D*/ u8 animLoopCounter;
    /*0x2E*/ s16 data[8];
    /*0x3E*/ u8 inUse : 1;
    /*0x3E*/ u8 coordOffsetEnabled : 1;
    /*0x3E*/ u8 invisible : 1;
    /*0x3E*/ u8 flags_3 : 5;
    /*0x3F*/ u8 flags2;
    /*0x40*/ u16 sheetTileStart;
    /*0x42*/ u8 subspriteTableNum;
    /*0x43*/ u8 subpriority;
};

#define MAX_SPRITES 64
_Static_assert(sizeof(struct OamData) == 8, "OamData size");
_Static_assert(sizeof(struct AffineAnimCmd) == 8, "AffineAnimCmd size");
_Static_assert(sizeof(struct Sprite) == 0x44, "struct Sprite must match the ROM");
#define SPRITE_NONE 0xFF

extern struct Sprite gSprites[];
extern const struct AnimCmd *const gDummySpriteAnimTable[];
extern const struct AffineAnimCmd *const gDummySpriteAffineAnimTable[];

u8 CreateSprite(const struct SpriteTemplate *template, s16 x, s16 y, u8 subpriority);
void DestroySpriteAndFreeResources(struct Sprite *sprite);
u8 LoadSpritePalette(const struct SpritePalette *palette);
u8 LoadCompressedSpritePalette(const struct CompressedSpritePalette *palette);
void sub_804B8C8(u8 x, u8 y, u8 a, u8 b);   /* Poke Ball open particles */
void sub_804B8F0(u8 a, u8 b, u8 c);          /* glow / flash */

extern const struct CompressedSpriteSheet gBallSpriteSheets[];
extern const struct CompressedSpritePalette gBallSpritePalettes[];
extern const struct OamData gBallOamData;
extern const struct CompressedSpriteSheet gMonFrontPicTable[];
extern const struct CompressedSpritePalette gMonPaletteTable[];
extern const struct AffineAnimCmd *const gUnk_08234944[];
u16 LoadCompressedSpriteSheet(const struct CompressedSpriteSheet *sheet);
void sub_8006B10(void);
void sub_80088F0(void);
void sub_8000B68(u8 arg);
void SetGpuReg(u8 reg, u16 value);
u16 Random(void);

#endif // GUARD_SPRITE_H
