#ifndef GUARD_GBA_H
#define GUARD_GBA_H

#include "types.h"

/* offsets for SetGpuReg() */
#define REG_OFFSET_DISPCNT 0x00
#define REG_OFFSET_BLDCNT 0x50
#define REG_OFFSET_BLDALPHA 0x52

#define DISPCNT_OBJ_1D_MAP (1 << 6)
#define DISPCNT_OBJ_ON (1 << 12)

/* Semi-transparent sprites are always the first blend target; these pick what is behind. */
#define BLDCNT_TGT2_BG0 (1 << 8)
#define BLDCNT_TGT2_BG1 (1 << 9)
#define BLDCNT_TGT2_BG2 (1 << 10)
#define BLDCNT_TGT2_BG3 (1 << 11)
#define BLDCNT_TGT2_BD (1 << 13)
#define BLDCNT_TGT2_ALL_BG (BLDCNT_TGT2_BG0 | BLDCNT_TGT2_BG1 | BLDCNT_TGT2_BG2 | BLDCNT_TGT2_BG3 | BLDCNT_TGT2_BD)

/* eva: weight of the sprite, evb: weight of what is behind it (both 0-16) */
#define BLDALPHA_BLEND(eva, evb) ((eva) | ((evb) << 8))

#define A_BUTTON 0x0001
#define B_BUTTON 0x0002
#define DPAD_RIGHT 0x0010
#define DPAD_LEFT 0x0020

#define PALETTES_ALL 0xFFFFFFFF
#define PALETTES_ALL_BUT_TEXTBOX 0xFFFFFFF0

#endif // GUARD_GBA_H
