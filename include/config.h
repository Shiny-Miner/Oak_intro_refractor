#ifndef GUARD_CONFIG_H
#define GUARD_CONFIG_H

/* Everything you are likely to want to tweak lives here (like CFRU's src/config.h). */

/* Frames between two steps of a sprite fade, and pixels a sprite slides per frame. */
#define FADE_STEPS 3
#define SLIDE_STEP 2

/* Message speed: 0 = instant ... 8 = slow. */
#define OAK_TEXT_SPEED 1

/* Songs / sound effects */
#define OAK_BGM 0x124
#define OAK_SE_CHOICE 5

/* Pokemon the professor releases from the ball (species number). */
#define OAK_POKEMON 1

/* Where the big (64x128) actors stand. */
#define ACTOR_CENTER_X 0x77
#define ACTOR_LEFT_X 0x30
#define ACTOR_RIGHT_X 0xC0
#define ACTOR_Y 0x20

/* Pokeball roll-in position (centre of the 16x16 sprite) */
#define BALL_START_X 0x60
#define BALL_START_Y 0x75
#define BALL_STOP_X 0x48

/*
 * The intro keeps its state in a heap block. Its address is parked in this word of
 * script-variable RAM (gSpecialVar_0x8004/0x8005) so it survives the naming screen.
 * Pick any 4-byte aligned word nothing else uses while the intro runs.
 */
#define OAK_STATE_PTR_ADDR 0x020370C0

#endif // GUARD_CONFIG_H
