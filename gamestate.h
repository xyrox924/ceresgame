#ifndef GAMESTATE_H
#define GAMESTATE_H

enum GameStates
{
    GS_TITLE,
    GS_GAME,
    GS_PAUSED,
    GS_OVER,
    GS_EXIT,
    GS_NEXT
};

extern char gameState;

#endif /* GAMESTATE_H */
