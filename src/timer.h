#ifndef TIMER_H
#define TIMER_H

#include <SDL.h>
#include <stdbool.h>

struct Timer
{
    Uint64 lastTime;
    double interval; // in seconds
} typedef Timer;

void timerInit(Timer *t, double interval);
bool timerExpired(Timer *t);

#endif /* TIMER_H */