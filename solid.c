#include "solid.h"

#include <stdio.h>
#include <stdlib.h>

Solid *createSolid(int x, int y, int w, int h, Texture t)
{
	Solid *s = (Solid *)malloc(sizeof(Solid));

	if (s)
	{
		s->body.x = x;
		s->body.y = y;
		s->body.w = w;
		s->body.h = h;

		s->tex = t;

		return s;
	}
	else
	{
		printf("Failed to create actor\n");

		return NULL;
	}
}

void destroySolid(Solid *s)
{
	SDL_DestroyTexture(s->tex.data);
	free(s);
}

void renderSolid(SDL_Renderer *r, Solid *s)
{
	SDL_Rect dest = { s->body.x, s->body.y, s->tex.w, s->tex.h };
	SDL_RenderCopy(r, s->tex.data, NULL, &dest);
}

void renderSolidF(SDL_Renderer *r, Solid *s)
{
	SDL_FRect dest = { (float)s->body.x, (float)s->body.y, (float)s->tex.w, (float)s->tex.h };
	SDL_RenderCopyF(r, s->tex.data, NULL, &dest);
}

void renderSolidOffsetF(SDL_Renderer *r, Solid *s, float x, float y)
{
	SDL_FRect dest = { (float)s->body.x + x, (float)s->body.y + y, (float)s->tex.w, (float)s->tex.h };
	SDL_RenderCopyF(r, s->tex.data, NULL, &dest);
}
