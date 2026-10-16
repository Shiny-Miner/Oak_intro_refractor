#ifndef GUARD_TASK_H
#define GUARD_TASK_H

#include "types.h"

typedef void (*TaskFunc)(u8 taskId);

struct Task
{
    TaskFunc func;
    bool8 isActive;
    u8 prev;
    u8 next;
    u8 priority;
    s16 data[16];
};

extern struct Task gTasks[];
_Static_assert(sizeof(struct Task) == 0x28, "struct Task must match the ROM");

u8 CreateTask(TaskFunc func, u8 priority);

#endif // GUARD_TASK_H
