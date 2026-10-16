#ifndef GUARD_SAVE_H
#define GUARD_SAVE_H

#include "types.h"

#define PLAYER_NAME_LENGTH 7
#define MALE 0
#define FEMALE 1

struct SaveBlock2
{
    u8 playerName[PLAYER_NAME_LENGTH + 1];
    u8 playerGender;
};

#define RIVAL_NAME_OFFSET 0x3A4C /* from gSaveBlock1Ptr */

extern struct SaveBlock2 *gSaveBlock2Ptr;
extern u8 *gSaveBlock1Ptr;
#define gRivalName ((u8 *)(gSaveBlock1Ptr + RIVAL_NAME_OFFSET))

/* naming screen */
#define NAMING_SCREEN_PLAYER 0
#define NAMING_SCREEN_RIVAL 4
void DoNamingScreen(u8 templateId, u8 *destBuffer, u16 monSpecies, u16 monGender, u32 monPersonality,
                    void (*returnCallback)(void));

#endif // GUARD_SAVE_H
