#ifndef ANIMATION_H
#define ANIMATION_H

#include <stdbool.h>

#include <SDL.h>

#include "texture.h"

int animationFrameCount(const Texture *tex, int frameW);
bool animationCanRender(const Texture *tex, int frameW, int frameH);
int animationFrameIndex(float frame, int frameCount);
void animationAdvanceLoop(float *frame, float speed, int frameCount);
int animationTimedFrame(Uint32 elapsed, Uint32 frameTime, int frameCount);
bool renderAnimationFrame(SDL_Renderer *r, const Texture *tex, int frameW, int frameH, float frame, const SDL_Rect *dest, SDL_RendererFlip flip);

#endif /* ANIMATION_H */
