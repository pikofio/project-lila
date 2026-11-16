#ifndef DELAY_H
#define DELAY_H
//delay function... what else is to say here
/*
# made by Tomas Warcholek for project-lila
# 28/10/2026
*/


#include "timing.h"

void delay(uint32_t ticks)
{
    uint32_t start = systick_ticks;

    while ((uint32_t)(systick_ticks - start) < ticks);
}

#endif //and thats all my friends