#ifndef __STAR_H__
#define __STAR_H__

#include "main.h"
#include "core.h"
#include "d3d.h"
#include "gamemain.h"
#include "model.h"
#include "particle.h"

typedef struct STAR {
	static LPDIRECT3DTEXTURE9	pTex;
	static UINT					cnt;
	MAZE*						pMaze;
	EMITTER*					emitter;
	MODEL*						mdl;
	PLAYER*						player;
	BOOL						visible;
	SRT							srt;
	FLOAT						throttle;

	STAR(MAZE* maze, PLAYER* owner);
	~STAR();

	HRESULT						Draw();
	void						Update();
} STAR;

#endif // !__STAR_H__
