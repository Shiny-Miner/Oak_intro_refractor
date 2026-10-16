#ifndef GUARD_MAIN_H
#define GUARD_MAIN_H

#include "types.h"

typedef void (*MainCallback)(void);

struct Main
{
    /*0x00*/ MainCallback callback1;
    /*0x04*/ MainCallback callback2;
    /*0x08*/ MainCallback savedCallback;
    /*0x0C*/ MainCallback vblankCallback;
    /*0x10*/ MainCallback hblankCallback;
    /*0x14*/ MainCallback vcountCallback;
    /*0x18*/ MainCallback serialCallback;
    /*0x1C*/ u32 intrCheck;
    /*0x20*/ u32 *vblankCounter1;
    /*0x24*/ u32 vblankCounter2;
    /*0x28*/ u16 heldKeysRaw;
    /*0x2A*/ u16 newKeysRaw;
    /*0x2C*/ u16 heldKeys;
    /*0x2E*/ u16 newKeys;
    /*0x30*/ u16 newAndRepeatedKeys;
    /*0x32*/ u16 keyRepeatCounter;
    /*0x34*/ u16 watchedKeysPressed;
    /*0x36*/ u16 watchedKeysMask;
    /*0x38*/ u8 objCount;
    /*0x39*/ u8 pad39[3];
    /*0x3C*/ u32 oamBuffer[256];
    /*0x43C*/ u8 state;
    /*0x43D*/ u8 oamLoadDisabled;
};

extern struct Main gMain;
_Static_assert(sizeof(struct Main) == 0x440, "struct Main size");

void SetMainCallback2(MainCallback callback);

#endif // GUARD_MAIN_H
