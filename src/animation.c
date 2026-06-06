#include "animation.h"

int animationFrameCount(const Texture *tex, int frameW)
{
	if (!tex || !tex->data || frameW <= 0 || tex->w < frameW)
	{
		return 0;
	}

	return tex->w / frameW;
}

bool animationCanRender(const Texture *tex, int frameW, int frameH)
{
	return tex && tex->data && frameW > 0 && frameH > 0 && tex->w >= frameW && tex->h >= frameH && animationFrameCount(tex, frameW) > 0;
}

int animationFrameIndex(float frame, int frameCount)
{
	int index = (int)frame;

	if (frameCount <= 0)
	{
		return 0;
	}

	if (index < 0)
	{
		return 0;
	}

	if (index >= frameCount)
	{
		index %= frameCount;
	}

	return index;
}

void animationAdvanceLoop(float *frame, float speed, int frameCount)
{
	if (!frame || frameCount <= 0)
	{
		return;
	}

	*frame += speed;
	if (*frame >= frameCount || *frame < 0)
	{
		*frame = 0;
	}
}

int animationTimedFrame(Uint32 elapsed, Uint32 frameTime, int frameCount)
{
	int frame;

	if (frameCount <= 0 || frameTime == 0)
	{
		return 0;
	}

	frame = (int)(elapsed / frameTime);
	if (frame >= frameCount)
	{
		frame = frameCount - 1;
	}

	return frame;
}

bool renderAnimationFrame(SDL_Renderer *r, const Texture *tex, int frameW, int frameH, float frame, const SDL_Rect *dest, SDL_RendererFlip flip)
{
	int frameCount;
	int frameIndex;
	SDL_Rect src;

	if (!r || !dest || !animationCanRender(tex, frameW, frameH))
	{
		return false;
	}

	frameCount = animationFrameCount(tex, frameW);
	frameIndex = animationFrameIndex(frame, frameCount);
	src.x = frameIndex * frameW;
	src.y = 0;
	src.w = frameW;
	src.h = frameH;

	return SDL_RenderCopyEx(r, tex->data, &src, dest, 0, NULL, flip) == 0;
}
