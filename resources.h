#ifndef TEXTURES_H
#define TEXTURES_H

#include "texture.h"

#include <SDL_mixer.h>

extern Texture texSlime;
extern Texture texMountains;
extern Texture texTitle;
extern Texture texGameover;
extern Texture texPaused;
extern Texture texFire;

extern Mix_Chunk *sfxOk;
extern Mix_Chunk *sfxBam;
extern Mix_Chunk *sfxGrass;
extern Mix_Music *sfxWind;

#endif /* TEXTURES_H */
