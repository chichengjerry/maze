#ifndef __PORTAL_H__
#define __PORTAL_H__

#include "main.h"
#include "core.h"
#include "gamemain.h"
#include "particle.h"

#define PORTAL_PARTICLES	256

typedef struct PORTAL {
	static LPDIRECT3DTEXTURE9	pTex;
	EMITTER*					dots;
	MAZE*						maze;
	D3DXVECTOR3					pos;

	PORTAL(MAZE* pMaze, int x, int y);
	~PORTAL();

	void						Update();
	HRESULT						Draw(CAMERA* camera);
} PORTAL;

#endif // !__PORTAL_H__
