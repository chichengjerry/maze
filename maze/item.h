#ifndef __ITEM_H__
#define __ITEM_H__

#include "main.h"
#include "d3d.h"
#include "gamemain.h"
#include "model.h"
#include "particle.h"

typedef struct ITEM{
	static LPDIRECT3DTEXTURE9	pTex;
	static int					cnt;
	MAZE*						pMaze;
	BOOL						picked;
	EMITTER*					glow;
	MODEL*						mdl;
	FLOAT						fGlowTime;
	FLOAT						fSize;
	SRT							srt;

	ITEM(MAZE * maze, int x, int y);
	~ITEM();

	void						Update(PLAYER* player);
	HRESULT						Draw(CAMERA* camera);
} ITEM;

#endif // !__ITEM_H__
