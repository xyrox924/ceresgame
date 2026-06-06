#ifndef SOLID_H
#define SOLID_H

#include <SDL.h>

#include "texture.h"

struct Solid // solids are only there for collision so i don't think they need anything else
{
	SDL_Rect body;

	Texture tex;
} typedef Solid;

Solid *createSolid(int x, int y, int w, int h, Texture t);
void destroySolid(Solid *s);

void renderSolid(SDL_Renderer *r, Solid *s);
void renderSolidF(SDL_Renderer *r, Solid *s);
void renderSolidOffsetF(SDL_Renderer *r, Solid *s, float x, float y);

#endif /* ACTOR_H */