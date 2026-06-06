#ifndef MAP_H
#define MAP_H

#include "cJSON.h"

#include <stdbool.h>

#include <SDL.h>

#include "cvector.h"
#include "texture.h"

struct Actor typedef Actor; // just here i guess

struct Object
{
	SDL_FRect body;
	char name[32];
	char type[32];
	int id;
} typedef Object;

struct Layer
{
	int w, h;
	char type[32];
	int id;

	int objectCount;
	Object *objects;

	int tileCount;
	int *tileData;
	SDL_Rect *tiles;
} typedef Layer;

struct Map
{
	int x, y;
	int w, h;
	int tw, th; // tile width and height
	Texture tileset;

	vector(Actor *) actors;

	int layerCount;
	Layer *layers;
} typedef Map;

Map *loadMap(const char *path, Texture tileset);
void loadLayer(cJSON *in, Layer *l, int tsw, int tw, int mw, int mh);
void populateMap(Map *m);
void depopulateMap(Map *m);
void doMapStuff(Map *m, Actor *p);
void updateMapAnimations(Map *m);

void renderMap(SDL_Renderer *r, Map *m);
void renderMapOffset(SDL_Renderer *r, Map *m, int x, int y, Actor *a);
void resolveMapCollisionsX(Actor *a, Map *m);
void resolveMapCollisionsY(Actor *a, Map *m);
void freeMap(Map *m);

#endif /* MAP_H */
