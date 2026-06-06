#include "texture.h"

#include <stdio.h>

Texture loadTexture(SDL_Renderer *r, const char *path)
{
	Texture texture = { 0 };

	SDL_Texture *temp = IMG_LoadTexture(r, path);
	if (!temp)
	{
		printf("Failed to load texture %s: %s\n", path, IMG_GetError());
		return texture;
	}

	texture.data = temp;
	if (SDL_QueryTexture(texture.data, NULL, NULL, &texture.w, &texture.h) != 0)
	{
		printf("Failed to query texture %s: %s\n", path, SDL_GetError());
		SDL_DestroyTexture(texture.data);
		texture.data = NULL;
		texture.w = 0;
		texture.h = 0;
	}

	return texture;
}

void renderTexture(SDL_Renderer *r, Texture t, int x, int y)
{
	if (!r || !t.data)
	{
		return;
	}

	SDL_Rect dest = { x, y, t.w, t.h };
	SDL_RenderCopy(r, t.data, NULL, &dest);
}

void destroyTexture(Texture *t)
{
	if (!t || !t->data)
	{
		return;
	}

	SDL_DestroyTexture(t->data);
	t->data = NULL;
	t->w = 0;
	t->h = 0;
}
