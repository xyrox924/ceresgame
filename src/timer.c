#include "timer.h"

void timerInit(Timer *t, double interval)
{
    t->lastTime = SDL_GetPerformanceCounter();
    t->interval = interval;
}

bool timerExpired(Timer *t)
{
    Uint64 now = SDL_GetPerformanceCounter();
    Uint64 freq = SDL_GetPerformanceFrequency();
    double elapsed = (double)(now - t->lastTime) / freq;

    //printf("elapsed: %.6f sec, interval: %.6f sec\n", elapsed, t->interval);

    if (elapsed >= t->interval) 
    {
        t->lastTime = now;
        return true;
    }
    return false;
}