#include "actor.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "animation.h"

#define TERMVEL 2.75f
#define GRAVITY 0.22f
#define JUMPSTRENGTH -4.5f
#define SLOW 0.9f // like air resistance or something.. so he doesn't keep going forever if i let go of the key
#define TURNSLOW 0.3f // lower is better
#define VXCUTOFF 0.29f // if smaller than this set vx to 0

static bool renderActorAnimationOffset(SDL_Renderer *r, Actor *a, int x, int y)
{
	SDL_Rect dest;
	SDL_RendererFlip flip;

	if (!a || !animationCanRender(&a->tex, a->w, a->h))
	{
		return false;
	}

	dest.x = (int)floor(a->body.x) + a->offsetX + x;
	dest.y = (int)floor(a->body.y) + a->offsetY + y;
	dest.w = a->w;
	dest.h = a->h;
	flip = a->flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

	return renderAnimationFrame(r, &a->tex, a->w, a->h, a->frame, &dest, flip);
}

static void updateEnemyAnimation(Actor *a)
{
	if (a->dead)
	{
		if (a->dyingugh > 2.9f)
		{
			a->superdead = true;
		}
		if (a->frame > 3.9f)
		{
			a->frame = 2;
		}
		a->dyingugh += a->animSpeed;
		a->frame += a->animSpeed * 0.95f;
	}
	else
	{
		if (a->frame > 1.9f)
		{
			a->frame = 0;
		}
	}

	a->frame += a->animSpeed;
}

static void updatePlayerAnimation(Actor *a)
{
	if (!a->jumping)
	{
		if (a->moving)
		{
			if (a->frame == 0)
			{
				a->frame = 0.8f;
			}
			if (a->frame > 2.9f)
			{
				a->frame = 1;
			}
			a->frame += a->animSpeed;
		}
		else
		{
			a->frame = 0;
		}
	}
	else
	{
		a->frame = 3;
	}
}

static void updateLoopingAnimation(Actor *a)
{
	animationAdvanceLoop(&a->frame, a->animSpeed, animationFrameCount(&a->tex, a->w));
}

void updateActorAnimation(Actor *a)
{
	if (!a || !a->animate)
	{
		return;
	}

	if (a->id == ID_PLAYER)
	{
		updatePlayerAnimation(a);
	}
	else if (a->id == ID_ENEMY)
	{
		updateEnemyAnimation(a);
	}
	else
	{
		updateLoopingAnimation(a);
	}
}

Actor *createActorNoTex(float x, float y, float w, float h)
{
	Actor *a = (Actor *)malloc(sizeof(Actor));

	if (a)
	{
		a->body.x = x;
		a->body.y = y;
		a->body.w = w;
		a->body.h = h;

		a->offsetX = 0;
		a->offsetY = 0;

		a->ox = 0;
		a->oy = 0;

		a->vx = 0;
		a->vy = 0;

		a->ax = 0;

		a->tex.data = NULL;
		a->tex.w = 0;
		a->tex.h = 0;
		a->ownsTexture = false;

		a->moving = false;
		a->jumping = false;
		a->spaceHeld = false;

		// animation stuff
		// make them the same size as the hitbox at first
		a->w = (int)w;
		a->h = (int)h;
		a->animSpeed = 0;
		a->frame = 0;
		a->flip = false;
		a->animate = false;

		a->id = ID_NONE;
		a->dead = false;
		a->dyingugh = 0;
		a->superdead = false;

		return a;
	}
	else
	{
		printf("Failed to create actor\n");

		return NULL;
	}
}

Actor *createActor(float x, float y, int w, int h, Texture t)
{
	Actor *a = (Actor *)malloc(sizeof(Actor));

	if (a)
	{
		a->body.x = x;
		a->body.y = y;
		a->body.w = (float)w;
		a->body.h = (float)h;

		a->offsetX = 0;
		a->offsetY = 0;

		a->ox = 0;
		a->oy = 0;

		a->vx = 0;
		a->vy = 0;

		a->ax = 0;

		a->tex = t;
		a->ownsTexture = true;

		a->moving = false;
		a->jumping = false;
		a->spaceHeld = false;

		// animation stuff
		// make them the same size as the hitbox at first
		a->w = w;
		a->h = h;
		a->animSpeed = 0;
		a->frame = 0;
		a->flip = false;
		a->animate = false;

		a->id = ID_ENEMY;
		a->dead = false;
		a->dyingugh = 0;
		a->superdead = false;

		return a;
	}
	else
	{
		printf("Failed to create actor\n");

		return NULL;
	}
}

void destroyActor(Actor *a)
{
	if (a->ownsTexture)
	{
		destroyTexture(&a->tex);
	}

	free(a);
}

bool colliding(Actor *a, Solid *s)
{
	// check for actual overlap (not just touching)
	if ((a->body.x + a->body.w > s->body.x && a->body.x < s->body.x + s->body.w) &&
		(a->body.y + a->body.h > s->body.y && a->body.y < s->body.y + s->body.h))
	{
		return true;
	}

	return false;
}

bool collidingRect(Actor *a, const SDL_Rect *s)
{
	if ((a->body.x + a->body.w > s->x && a->body.x < s->x + s->w) &&
		(a->body.y + a->body.h > s->y && a->body.y < s->y + s->h))
	{
		return true;
	}

	return false;
}

void resolveCollisionX(Actor *a, Solid *s)
{
	if (fabsf((float)s->body.x - a->body.x) < 64.0f)
	{
		if (colliding(a, s))
		{
			float overlapX1 = a->body.x + a->body.w - (float)s->body.x; // overlap from left
			float overlapX2 = (float)(s->body.x + s->body.w) - a->body.x; // overlap from right

			if (overlapX1 < overlapX2) // colliding from right of solid
			{
				if (a->vx > 0) { a->vx = 0; }
				a->body.x = (float)s->body.x - a->body.w; // keep to the left
			}
			else // colliding from left of solid
			{
				if (a->vx < 0) { a->vx = 0; }
				a->body.x = (float)(s->body.x + s->body.w); // keep to the right
			}
		}
	}
}

void resolveCollisionRectX(Actor *a, const SDL_Rect *s)
{
	if (fabsf((float)s->x - a->body.x) < 64.0f)
	{
		if (collidingRect(a, s))
		{
			float overlapX1 = a->body.x + a->body.w - (float)s->x;
			float overlapX2 = (float)(s->x + s->w) - a->body.x;

			if (overlapX1 < overlapX2)
			{
				if (a->vx > 0) { a->vx = 0; }
				a->body.x = (float)s->x - a->body.w;
			}
			else
			{
				if (a->vx < 0) { a->vx = 0; }
				a->body.x = (float)(s->x + s->w);
			}
		}
	}
}
	

void resolveCollisionY(Actor *a, Solid *s)
{
	if (colliding(a, s))
	{
		float overlapY1 = a->body.y + a->body.h - (float)s->body.y; // overlap from top
		float overlapY2 = (float)(s->body.y + s->body.h) - a->body.y; // overlap from bottom

		if (overlapY1 < overlapY2) // land on top
		{
			a->vy = 0;
			a->jumping = false;
			a->body.y = (float)s->body.y - a->body.h;
		}
		else // hitting from below
		{
			a->vy = 0;
			a->jumping = false;
			a->body.y = (float)(s->body.y + s->body.h);
		}
	}
}

void resolveCollisionRectY(Actor *a, const SDL_Rect *s)
{
	if (collidingRect(a, s))
	{
		float overlapY1 = a->body.y + a->body.h - (float)s->y;
		float overlapY2 = (float)(s->y + s->h) - a->body.y;

		if (overlapY1 < overlapY2)
		{
			a->vy = 0;
			a->jumping = false;
			a->body.y = (float)s->y - a->body.h;
		}
		else
		{
			a->vy = 0;
			a->jumping = false;
			a->body.y = (float)(s->y + s->h);
		}
	}
}


// player stuff really maybe put it somewhere else
void jump(Actor *a)
{
	a->vy = JUMPSTRENGTH; // jump strength
	//a->spaceHeld = true;
	a->jumping = true;
}

void moveX(Actor *a)
{
	a->ox = a->body.x;

	// ALL X MOVEMENT
	// acceleration is in main
	a->vx += a->ax;

	if (fabsf(a->vx) > TERMVEL)
	{
		if (a->vx > TERMVEL)
		{
			a->vx = TERMVEL;
		}
		else if (a->vx < -TERMVEL)
		{
			a->vx = -TERMVEL;
		}
	}

	if (a->ax > 0 && a->vx < 0) // if pressing right but moving left
	{
		a->vx *= TURNSLOW;
	}
	else if (a->ax < 0 && a->vx > 0) // if pressing left but moving right
	{
		a->vx *= TURNSLOW;
	}

	if (!a->moving)
	{
		a->ax = 0;

		if (fabsf(a->vx) < VXCUTOFF)
		{
			a->vx = 0;
		}

		a->vx *= SLOW; // 0.92 is fine? but it glides a bit. maybe just change the cutoff point to stop it
	}

	a->body.x += a->vx;
}

void moveY(Actor *a)
{
	a->oy = a->body.y;

	// ALL Y MOVEMENT
	if (a->jumping && !a->spaceHeld)
	{
		a->vy += (float)(GRAVITY / 2); // add gravity a second (technically first, but what i mean is twice in one frame) time if the player isn't holding space
	}

	a->vy += GRAVITY; // gravity
	a->body.y += a->vy;
}

void renderActor(SDL_Renderer *r, Actor *a)
{
	if (!a || !a->tex.data)
	{
		return;
	}

	// don't hardcode -5 okay
	SDL_Rect dest = { (int)floor(a->body.x), (int)floor(a->body.y) - 5, a->tex.w, a->tex.h };
	SDL_RenderCopy(r, a->tex.data, NULL, &dest);
}

void renderActorOffset(SDL_Renderer *r, Actor *a, int x, int y)
{
	if (!a || !a->tex.data)
	{
		return;
	}

	SDL_Rect dest = { (int)floor(a->body.x) + a->offsetX + x, (int)floor(a->body.y) + a->offsetY + y, a->tex.w, a->tex.h };
	SDL_RenderCopy(r, a->tex.data, NULL, &dest);

#ifdef DEBUG_H
	// CRINGE DEBUG
	SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
	SDL_Rect hitbox = { (int)floor(a->body.x) + x, (int)floor(a->body.y) + y, a->body.w, a->body.h };
	SDL_RenderDrawRect(r, &hitbox);
#endif /* DEBUG_H */
}

void renderActorOffsetA(SDL_Renderer *r, Actor *a, int x, int y)
{
	if (!renderActorAnimationOffset(r, a, x, y))
	{
		return;
	}

#ifdef DEBUG_H
	// CRINGE DEBUG
	SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
	SDL_Rect hitbox = { (int)floor(a->body.x) + x, (int)floor(a->body.y) + y, a->body.w, a->body.h };
	SDL_RenderDrawRect(r, &hitbox);
#endif /* DEBUG_H */

}

void renderActorOffsetAP(SDL_Renderer *r, Actor *a, int x, int y)
{
	if (!renderActorAnimationOffset(r, a, x, y))
	{
		return;
	}

#ifdef DEBUG_H
	// CRINGE DEBUG
	SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
	SDL_Rect hitbox = { (int)floor(a->body.x) + x, (int)floor(a->body.y) + y, a->body.w, a->body.h };
	SDL_RenderDrawRect(r, &hitbox);
#endif

}
