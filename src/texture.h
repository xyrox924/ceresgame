#ifndef TEXTURE_H
#define TEXTURE_H

#include <SDL.h>
#include <SDL_image.h>

struct Texture
{
	SDL_Texture *data;
	int w, h;
} typedef Texture;

Texture loadTexture(SDL_Renderer *r, const char *path);
void renderTexture(SDL_Renderer *r, Texture t, int x, int y);
void destroyTexture(Texture *t);

#endif /* TEXTURE_H */