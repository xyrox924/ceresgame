#ifndef RESOURCES_H
#define RESOURCES_H

#include <stdbool.h>

#include <SDL.h>
#include <SDL_mixer.h>

#include "texture.h"

extern Texture texSlime;
extern Texture texMountains;
extern Texture texTitle;
extern Texture texGameover;
extern Texture texPaused;
extern Texture texBuildings;
extern Texture texFire;
extern Texture texExit;
extern Texture texNext;
extern Texture texYouWin;
extern Texture texTileset;
extern Texture texPlayer;

extern Mix_Chunk *sfxOk;
extern Mix_Chunk *sfxBam;
extern Mix_Music *sfxWind;

extern bool audioReady;

bool loadGameResources(SDL_Renderer *renderer);
void unloadGameResources(void);
void playSfx(Mix_Chunk *sfx);
void playMusic(Mix_Music *music);

#endif /* RESOURCES_H */
