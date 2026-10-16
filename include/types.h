#ifndef GUARD_TYPES_H
#define GUARD_TYPES_H

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef u8 bool8;
typedef u16 bool16;
typedef u32 bool32;

#define TRUE 1
#define FALSE 0
#define NULL ((void *)0)

#define ARRAY_COUNT(arr) ((int)(sizeof(arr) / sizeof((arr)[0])))
#define UNUSED __attribute__((unused))

#define RGB(r, g, b) ((r) | ((g) << 5) | ((b) << 10))
#define RGB_BLACK RGB(0, 0, 0)
#define RGB_WHITE RGB(31, 31, 31)

#endif // GUARD_TYPES_H
