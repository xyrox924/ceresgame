#ifndef ACTOR_H
#define ACTOR_H

#include  <SDL.h>
#include <stdbool.h>

#include "cvector.h"
#include "texture.h"
#include "solid.h"
#include "timer.h"

enum
{
	ID_NONE,
	ID_PLAYER,
	ID_ENEMY,
	ID_KILLBOX,
	ID_END,
	ID_LEDGE
};

struct AnimState
{
	char startFrame, endFrame;
} typedef AnimState;

struct Actor // these are for colliding with solids
{
	SDL_FRect body;
	int offsetX, offsetY; // hitbox offsets (visual)

	float ox, oy;
	float vx, vy;
	float ax;

	// spritesheet usually
	Texture tex;
	bool ownsTexture;
	
	bool moving;
	bool jumping;
	bool spaceHeld;

	// animation stuff (new)
	int w;
	int h;
	float animSpeed;
	float frame;
	bool flip;
	bool animate;

	char id;
	bool dead;

	float dyingugh; // (kill yourself)
	bool superdead;

	vector(AnimState) as; // animstates
} typedef Actor;

Actor *createActorNoTex(float x, float y, float w, float h);
Actor *createActor(float x, float y, int w, int h, Texture t);
void destroyActor(Actor *a);

bool colliding(Actor *a, Solid *s);
void resolveCollisionX(Actor *a, Solid *s); // solids can't collide with each other so i'll define this here
void resolveCollisionY(Actor *a, Solid *s);
bool collidingRect(Actor *a, const SDL_Rect *s);
void resolveCollisionRectX(Actor *a, const SDL_Rect *s);
void resolveCollisionRectY(Actor *a, const SDL_Rect *s);


// player stuff really maybe put it somewhere else
void jump(Actor *a);
void moveX(Actor *a);
void moveY(Actor *a);
void renderActor(SDL_Renderer *r, Actor *a);
void renderActorOffset(SDL_Renderer *r, Actor *a, int x, int y);
// animated
void renderActorA(SDL_Renderer *r, Actor *a);
void renderActorOffsetA(SDL_Renderer *r, Actor *a, int x, int y);
void renderActorOffsetAP(SDL_Renderer *r, Actor *a, int x, int y);

#endif /* ACTOR_H */
