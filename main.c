#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>

#include "config.h"

#include "gamestate.h"
#include "resources.h"
#include "solid.h"
#include "actor.h"
#include "animation.h"
#include "camera.h"
#include "map.h"

#define PLAYER_START_X (16.0f * 15.0f)
#define PLAYER_START_Y 100.0f

#define EXIT_FRAME_COUNT 6
#define EXIT_FRAME_W 320
#define EXIT_FRAME_H 180
#define EXIT_FRAME_TIME 120
#define NEXT_DISPLAY_TIME 900
#define LEVEL_1 1
#define LEVEL_2 2

// system
SDL_Window *w;
SDL_Renderer *r;
bool running = false;
bool sdlReady = false;
bool imgReady = false;

bool qPressed = false;
char gameState = GS_TITLE;
int currentLevel = LEVEL_1;

// level
Solid *bg;
Solid *bg2;
Map *m;
Actor *a;
Camera c;

bool init();
bool createGameObjects();
void unloadGameObjects();
void takeInput(SDL_Event e);
void pollEvents();
void update();
void render();
bool loadLevel(const char *path);
void resetPlayer();
void startLevelTransition();
void updateLevelTransition();
void renderGameWorld();
void renderExitAnimation();
void renderNextScreen();
void renderWinScreen();
void deinit();

int main(int argc, char *argv[])
{
    if (!init())
    {
        deinit();
        return 1;
    }

    if (!loadGameResources(r))
    {
        deinit();
        return 1;
    }

    if (!createGameObjects())
    {
        unloadGameObjects();
        unloadGameResources();
        deinit();
        return 1;
    }

    currentLevel = LEVEL_1;
    running = loadLevel("res/map.tmj") && m && a;
    while (running)
    {
        Uint32 frameStart = SDL_GetTicks();

        pollEvents();
        update();
        render();

        Uint32 frameTime = SDL_GetTicks() - frameStart;

        if (frameTime < FRAME_DELAY)
        {
            SDL_Delay(FRAME_DELAY - frameTime);
        }
    }

    printf("exiting game\n");

    unloadGameObjects();
    unloadGameResources();
    deinit();

    return 0;
}

bool init()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }
    sdlReady = true;

    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG)
    {
        printf("IMG_Init failed: %s\n", IMG_GetError());
        return false;
    }
    imgReady = true;

    if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) != 0)
    {
        printf("Warning: audio disabled: %s\n", Mix_GetError());
    }
    else
    {
        audioReady = true;
    }

    w = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, 0);
    if (!w)
    {
        printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
        return false;
    }

    r = SDL_CreateRenderer(w, -1, SDL_RENDERER_ACCELERATED);
    if (!r)
    {
        printf("SDL_CreateRenderer failed: %s\n", SDL_GetError());
        return false;
    }

    if (SDL_RenderSetLogicalSize(r, SW, SH) != 0)
    {
        printf("SDL_RenderSetLogicalSize failed: %s\n", SDL_GetError());
        return false;
    }

    SDL_SetRenderDrawColor(r, 34, 35, 35, 255);

    return true;
}

bool createGameObjects()
{
    bg = createSolid(0, -10, texMountains.w, texMountains.h, texMountains);
    bg2 = createSolid(0, 70, texMountains.w, texMountains.h, texBuildings);
    if (!bg || !bg2)
    {
        return false;
    }

    a = createActor(PLAYER_START_X, PLAYER_START_Y, 16, 16, texPlayer);
    if (!a)
    {
        return false;
    }
    a->ownsTexture = false;

    a->w = 16;
    a->h = 21;
    a->animSpeed = 0.05f;
    a->offsetY = -4;
    a->offsetX = 0;
    a->animate = true;
    a->id = ID_PLAYER;

    return true;
}

void unloadGameObjects()
{
    if (m)
    {
        depopulateMap(m);
        freeMap(m);
        m = NULL;
    }

    if (a)
    {
        destroyActor(a);
        a = NULL;
    }

    if (bg)
    {
        free(bg);
        bg = NULL;
    }

    if (bg2)
    {
        free(bg2);
        bg2 = NULL;
    }
}

Uint32 jumpQueuedTime;
bool jumpQueued;
Uint32 transitionStartTime;

#define JUMP_BUFFER_TIME 110 // MS

void takeInput(SDL_Event e)
{
    const Uint8 *keys = SDL_GetKeyboardState(NULL);

    if (gameState == GS_TITLE)
    {
        if (keys[SDL_SCANCODE_RETURN])
        {
            gameState = GS_GAME;
            playSfx(sfxOk);
            playMusic(sfxWind);
        }
    }
    else if (gameState == GS_GAME)
    {
        if (keys[SDL_SCANCODE_D])
        {
            a->ax = 0.3f;
            a->moving = true;
            a->flip = false;
        }
        else if (keys[SDL_SCANCODE_A])
        {
            a->ax = -0.3f;
            a->moving = true;
            a->flip = true;
        }
        else
        {
            a->moving = false;
        }

        // debug camera controls
        if (keys[SDL_SCANCODE_UP])
        {
            c.y -= 5;
        }
        else if (keys[SDL_SCANCODE_DOWN])
        {
            c.y += 5;
        }
        if (keys[SDL_SCANCODE_RIGHT])
        {
            c.x += 5;
        }
        else if (keys[SDL_SCANCODE_LEFT])
        {
            c.x -= 5;
        }

        if (keys[SDL_SCANCODE_SPACE])
        {
            a->spaceHeld = true;
            if (!jumpQueued)
            {
                jumpQueued = true;
                jumpQueuedTime = SDL_GetTicks();
            }
        }
        else
        {
            if (a->jumping)
            {
                a->spaceHeld = false;
            }
        }
    }
}

void pollEvents()
{
    // input stuff
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        switch (e.type)
        {
        case SDL_KEYDOWN:
            if (e.key.keysym.sym == SDLK_q && !qPressed)
            {
                qPressed = true;
                if (gameState == GS_GAME)
                {
                    playSfx(sfxOk);
                    gameState = GS_PAUSED;
                }
                else if (gameState == GS_PAUSED)
                {
                    playSfx(sfxOk);
                    gameState = GS_GAME;
                }
            }

            if (e.key.keysym.sym == SDLK_ESCAPE)
            {
                running = false;
            }

            takeInput(e);
            break;
        case SDL_KEYUP:
            if (e.key.keysym.sym == SDLK_q)
            {
                qPressed = false;
            }

            takeInput(e);
            break;
        case SDL_QUIT:
            running = false;
            break;
        }
    }
}

bool loadLevel(const char *path)
{
    Map *newMap = loadMap(path, texTileset);
    if (!newMap)
    {
        printf("Failed to load level: %s\n", path);
        return false;
    }

    populateMap(newMap);
    newMap->x = -3 * 16;
    newMap->y = -24;

    if (m)
    {
        depopulateMap(m);
        freeMap(m);
    }

    m = newMap;
    resetPlayer();

    return true;
}

void resetPlayer()
{
    if (!a)
    {
        return;
    }

    a->body.x = PLAYER_START_X;
    a->body.y = PLAYER_START_Y;
    a->ox = a->body.x;
    a->oy = a->body.y;
    a->vx = 0;
    a->vy = 0;
    a->ax = 0;
    a->moving = false;
    a->jumping = false;
    a->spaceHeld = false;
    a->frame = 0;

    jumpQueued = false;
    jumpQueuedTime = 0;

    c.x = (int)(a->body.x - SW / 2);
    c.y = (int)(a->body.y - SH / 2 + 16);
    c.deadzoneX = 30;
    c.deadzoneY = 48;
}

void startLevelTransition()
{
    transitionStartTime = SDL_GetTicks();
}

void updateLevelTransition()
{
    Uint32 elapsed = SDL_GetTicks() - transitionStartTime;

    if (elapsed >= EXIT_FRAME_COUNT * EXIT_FRAME_TIME)
    {
        if (currentLevel == LEVEL_1)
        {
            if (loadLevel("res/map2.tmj"))
            {
                currentLevel = LEVEL_2;
                transitionStartTime = SDL_GetTicks();
                gameState = GS_NEXT;
            }
            else
            {
                gameState = GS_OVER;
            }
        }
        else
        {
            playSfx(sfxOk);
            gameState = GS_WIN;
        }
    }
}

void update()
{
    if (gameState == GS_TITLE)
    {

    }
    else if (gameState == GS_GAME)
    {
        moveX(a);
        resolveMapCollisionsX(a, m);

        moveY(a);
        resolveMapCollisionsY(a, m);

        if (!a->jumping)
        {
            Uint32 currentTime = SDL_GetTicks();

            if (jumpQueued && (currentTime - jumpQueuedTime <= JUMP_BUFFER_TIME))
            {
                jump(a);
                jumpQueued = false;
            }
        }

        if (jumpQueued && SDL_GetTicks() - jumpQueuedTime > JUMP_BUFFER_TIME)
        {
            jumpQueued = false;
        }

        updateCamera(&c, a, m->w, m->h);

        char oldState = gameState;
        doMapStuff(m, a);
        if (oldState != GS_EXIT && gameState == GS_EXIT)
        {
            startLevelTransition();
        }

        if (gameState == GS_GAME)
        {
            updateActorAnimation(a);
            updateMapAnimations(m);
        }
    }
    else if (gameState == GS_PAUSED)
    {

    }
    else if (gameState == GS_OVER)
    {

    }
    else if (gameState == GS_EXIT)
    {
        updateLevelTransition();
    }
    else if (gameState == GS_NEXT)
    {
        if (SDL_GetTicks() - transitionStartTime >= NEXT_DISPLAY_TIME)
        {
            gameState = GS_GAME;
        }
    }
    else if (gameState == GS_WIN)
    {

    }
}

void renderGameWorld()
{
    renderSolidOffsetF(r, bg, (float)-c.x / 205.0f, (float)-c.y);
    renderSolidOffsetF(r, bg2, (float)-c.x / 90.0f, (float)-c.y);
    renderActorOffsetAP(r, a, -c.x, -c.y);

    renderMapOffset(r, m, -c.x, -c.y, a);
}

void renderExitAnimation()
{
    SDL_Rect dest = { 0, 0, SW, SH };
    int frame;

    if (!animationCanRender(&texExit, EXIT_FRAME_W, EXIT_FRAME_H) || animationFrameCount(&texExit, EXIT_FRAME_W) < EXIT_FRAME_COUNT)
    {
        SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
        SDL_RenderFillRect(r, &dest);
        return;
    }

    frame = animationTimedFrame(SDL_GetTicks() - transitionStartTime, EXIT_FRAME_TIME, EXIT_FRAME_COUNT);
    renderAnimationFrame(r, &texExit, EXIT_FRAME_W, EXIT_FRAME_H, (float)frame, &dest, SDL_FLIP_NONE);
}

void renderNextScreen()
{
    SDL_Rect screen = { 0, 0, SW, SH };

    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_RenderFillRect(r, &screen);

    renderTexture(r, texNext, SW / 2 - texNext.w / 2, SH / 2 - texNext.h / 2);
}

void renderWinScreen()
{
    SDL_Rect screen = { 0, 0, SW, SH };

    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_RenderFillRect(r, &screen);

    renderTexture(r, texYouWin, SW / 2 - texYouWin.w / 2, SH / 2 - texYouWin.h / 2);
}

void render()
{
    // render
    SDL_SetRenderDrawColor(r, 34, 35, 35, 255);
    SDL_RenderClear(r);

    if (gameState == GS_TITLE)
    {
        renderTexture(r, texTitle, 0, 0);
    }
    else if (gameState == GS_GAME)
    {
        renderGameWorld();
    }
    else if (gameState == GS_PAUSED)
    {
        renderGameWorld();
        renderTexture(r, texPaused, SW / 2 - texPaused.w / 2, SH / 2 - texPaused.h / 2);
    }
    else if (gameState == GS_OVER)
    {
        renderTexture(r, texGameover, SW / 2 - texGameover.w / 2, SH / 2 - texGameover.h / 2);
    }
    else if (gameState == GS_EXIT)
    {
        renderGameWorld();
        renderExitAnimation();
    }
    else if (gameState == GS_NEXT)
    {
        renderNextScreen();
    }
    else if (gameState == GS_WIN)
    {
        renderWinScreen();
    }

    SDL_RenderPresent(r);
}

void deinit()
{
    if (r)
    {
        SDL_DestroyRenderer(r);
        r = NULL;
    }

    if (w)
    {
        SDL_DestroyWindow(w);
        w = NULL;
    }

    if (audioReady)
    {
        Mix_CloseAudio();
        audioReady = false;
    }
    Mix_Quit();

    if (imgReady)
    {
        IMG_Quit();
        imgReady = false;
    }

    if (sdlReady)
    {
        SDL_Quit();
        sdlReady = false;
    }
}
