#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>

#include "cJSON.h"

#include "config.h"

#include "gamestate.h"
#include "resources.h"
#include "cvector.h"
#include "solid.h"
#include "actor.h"
#include "camera.h"
#include "map.h"
#include "timer.h"

#define PLAYER_START_X (16.0f * 15.0f)
#define PLAYER_START_Y 100.0f

#define EXIT_FRAME_COUNT 6
#define EXIT_FRAME_W 320
#define EXIT_FRAME_H 180
#define EXIT_FRAME_TIME 120
#define NEXT_DISPLAY_TIME 900

// system
SDL_Window *w;
SDL_Renderer *r;
bool running = false;

bool qPressed = false;
char gameState = GS_TITLE;

// resources
Texture texSlime;
Texture texMountains;
Texture texTitle;
Texture texGameover;
Texture texPaused;
Texture texBuildings;
Texture texFire;
Texture texExit;
Texture texNext;
Texture texTileset;

Mix_Chunk *sfxOk;
Mix_Chunk *sfxBam;
Mix_Chunk *sfxGrass;
Mix_Music *sfxWind;

// level
Solid *bg;
Solid *bg2;
Map *m;
Actor *a;
Camera c;

void init();
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
void deinit();

int main(int argc, char *argv[])
{
    init();

    // resource loading
    texSlime = loadTexture(r, "res/slime.png");
    texMountains = loadTexture(r, "res/mountains.png");
    texTitle = loadTexture(r, "res/title.png");
    texGameover = loadTexture(r, "res/gameover.png");
    texPaused = loadTexture(r, "res/paused.png");
    texBuildings = loadTexture(r, "res/buildings.png");
    texFire = loadTexture(r, "res/fire.png");
    texExit = loadTexture(r, "res/exit.png");
    texNext = loadTexture(r, "res/next.png");
    texTileset = loadTexture(r, "res/tiles.png");

    sfxOk = Mix_LoadWAV("res/ok.wav");
    sfxBam = Mix_LoadWAV("res/bam.wav");
    sfxGrass = Mix_LoadWAV("res/grass.wav");
    sfxWind = Mix_LoadMUS("res/wind1.wav");

    // entity creation
    bg = createSolid(0, -10, texMountains.w, texMountains.h, texMountains);
    bg2 = createSolid(0, 70, texMountains.w, texMountains.h, texBuildings);

    a = createActor(PLAYER_START_X, PLAYER_START_Y, 16, 16, loadTexture(r, "res/player.png"));
    if (a)
    {
        a->w = 16;
        a->h = 21;
        a->animSpeed = 0.05f;
        a->offsetY = -4;
        a->offsetX = 0;
        a->animate = true;
        a->id = ID_PLAYER;
    }

    loadLevel("res/map.tmj");

    running = m && a;
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

    if (m)
    {
        depopulateMap(m);
        freeMap(m);
    }

    if (a)
    {
        destroyActor(a);
    }

    Mix_FreeChunk(sfxOk);
    Mix_FreeChunk(sfxBam);
    Mix_FreeChunk(sfxGrass);
    Mix_FreeMusic(sfxWind);

    destroyTexture(&texBuildings);
    destroyTexture(&texSlime);
    destroyTexture(&texMountains);
    
    destroyTexture(&texTitle);
    destroyTexture(&texPaused);
    destroyTexture(&texGameover);
    destroyTexture(&texFire);

    destroyTexture(&texExit);
    destroyTexture(&texNext);
    destroyTexture(&texTileset);

    deinit();

    return 0;
}

void init()
{
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096);

    w = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, 0);
    r = SDL_CreateRenderer(w, -1, SDL_RENDERER_ACCELERATED);
    SDL_RenderSetLogicalSize(r, SW, SH);
    SDL_SetRenderDrawColor(r, 34, 35, 35, 255);
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
            Mix_PlayChannel(-1, sfxOk, 0);
            if (Mix_PlayingMusic() == 0)
            {
                //Play the music
                Mix_PlayMusic(sfxWind, -1);
            }
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
                    Mix_PlayChannel(-1, sfxOk, 0);
                    gameState = GS_PAUSED;
                }
                else if (gameState == GS_PAUSED)
                {
                    Mix_PlayChannel(-1, sfxOk, 0);
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
        if (loadLevel("res/map2.tmj"))
        {
            transitionStartTime = SDL_GetTicks();
            gameState = GS_NEXT;
        }
        else
        {
            gameState = GS_OVER;
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

    if (!texExit.data || texExit.w < EXIT_FRAME_W * EXIT_FRAME_COUNT || texExit.h < EXIT_FRAME_H)
    {
        SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
        SDL_RenderFillRect(r, &dest);
        return;
    }

    int frame = (SDL_GetTicks() - transitionStartTime) / EXIT_FRAME_TIME;
    if (frame >= EXIT_FRAME_COUNT)
    {
        frame = EXIT_FRAME_COUNT - 1;
    }

    SDL_Rect src = { frame * EXIT_FRAME_W, 0, EXIT_FRAME_W, EXIT_FRAME_H };
    SDL_RenderCopy(r, texExit.data, &src, &dest);
}

void renderNextScreen()
{
    SDL_Rect screen = { 0, 0, SW, SH };

    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_RenderFillRect(r, &screen);

    renderTexture(r, texNext, SW / 2 - texNext.w / 2, SH / 2 - texNext.h / 2);
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

    SDL_RenderPresent(r);
}

void deinit()
{
    IMG_Quit();
    Mix_Quit();
    SDL_DestroyRenderer(r);
    SDL_DestroyWindow(w);
    SDL_Quit();
}
