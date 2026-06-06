#include "resources.h"

#include <stdio.h>

Texture texSlime;
Texture texMountains;
Texture texTitle;
Texture texGameover;
Texture texPaused;
Texture texBuildings;
Texture texFire;
Texture texExit;
Texture texNext;
Texture texYouWin;
Texture texTileset;
Texture texPlayer;

Mix_Chunk *sfxOk;
Mix_Chunk *sfxBam;
Mix_Music *sfxWind;

bool audioReady = false;

static bool requireTexture(Texture t, const char *path)
{
	if (!t.data)
	{
		printf("Required texture missing: %s\n", path);
		return false;
	}

	return true;
}

static Mix_Chunk *loadSfx(const char *path)
{
	Mix_Chunk *sfx;

	if (!audioReady)
	{
		return NULL;
	}

	sfx = Mix_LoadWAV(path);
	if (!sfx)
	{
		printf("Warning: failed to load sound %s: %s\n", path, Mix_GetError());
	}

	return sfx;
}

static Mix_Music *loadMusic(const char *path)
{
	Mix_Music *music;

	if (!audioReady)
	{
		return NULL;
	}

	music = Mix_LoadMUS(path);
	if (!music)
	{
		printf("Warning: failed to load music %s: %s\n", path, Mix_GetError());
	}

	return music;
}

bool loadGameResources(SDL_Renderer *renderer)
{
	bool ok = true;

	texSlime = loadTexture(renderer, "res/slime.png");
	texMountains = loadTexture(renderer, "res/mountains.png");
	texTitle = loadTexture(renderer, "res/title.png");
	texGameover = loadTexture(renderer, "res/gameover.png");
	texPaused = loadTexture(renderer, "res/paused.png");
	texBuildings = loadTexture(renderer, "res/buildings.png");
	texFire = loadTexture(renderer, "res/fire.png");
	texExit = loadTexture(renderer, "res/exit.png");
	texNext = loadTexture(renderer, "res/next.png");
	texYouWin = loadTexture(renderer, "res/youwin.png");
	texTileset = loadTexture(renderer, "res/tiles.png");
	texPlayer = loadTexture(renderer, "res/player.png");

	if (!requireTexture(texSlime, "res/slime.png")) { ok = false; }
	if (!requireTexture(texMountains, "res/mountains.png")) { ok = false; }
	if (!requireTexture(texTitle, "res/title.png")) { ok = false; }
	if (!requireTexture(texGameover, "res/gameover.png")) { ok = false; }
	if (!requireTexture(texPaused, "res/paused.png")) { ok = false; }
	if (!requireTexture(texBuildings, "res/buildings.png")) { ok = false; }
	if (!requireTexture(texFire, "res/fire.png")) { ok = false; }
	if (!requireTexture(texExit, "res/exit.png")) { ok = false; }
	if (!requireTexture(texNext, "res/next.png")) { ok = false; }
	if (!requireTexture(texYouWin, "res/youwin.png")) { ok = false; }
	if (!requireTexture(texTileset, "res/tiles.png")) { ok = false; }
	if (!requireTexture(texPlayer, "res/player.png")) { ok = false; }

	if (!ok)
	{
		unloadGameResources();
		return false;
	}

	sfxOk = loadSfx("res/ok.wav");
	sfxBam = loadSfx("res/bam.wav");
	sfxWind = loadMusic("res/wind1.wav");

	return true;
}

void unloadGameResources(void)
{
	if (sfxOk) { Mix_FreeChunk(sfxOk); sfxOk = NULL; }
	if (sfxBam) { Mix_FreeChunk(sfxBam); sfxBam = NULL; }
	if (sfxWind) { Mix_FreeMusic(sfxWind); sfxWind = NULL; }

	destroyTexture(&texBuildings);
	destroyTexture(&texSlime);
	destroyTexture(&texMountains);
	destroyTexture(&texTitle);
	destroyTexture(&texPaused);
	destroyTexture(&texGameover);
	destroyTexture(&texFire);
	destroyTexture(&texExit);
	destroyTexture(&texNext);
	destroyTexture(&texYouWin);
	destroyTexture(&texTileset);
	destroyTexture(&texPlayer);
}

void playSfx(Mix_Chunk *sfx)
{
	if (audioReady && sfx)
	{
		if (Mix_PlayChannel(-1, sfx, 0) == -1)
		{
			printf("Warning: failed to play sound: %s\n", Mix_GetError());
		}
	}
}

void playMusic(Mix_Music *music)
{
	if (audioReady && music && Mix_PlayingMusic() == 0)
	{
		if (Mix_PlayMusic(music, -1) != 0)
		{
			printf("Warning: failed to play music: %s\n", Mix_GetError());
		}
	}
}
