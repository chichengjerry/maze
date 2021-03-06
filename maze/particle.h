/***********************************************
 * [particle.h]
 * パーティクル効果処理
 ***********************************************/
#ifndef __PARTICLE_H__
#define __PARTICLE_H__

#include "main.h"
#include "core.h"
#include "d3d.h"
#include "gamemain.h"

#define GM_MAX_PARTICLES	256

typedef enum{
	RENDERMODE_SOLID,
	RENDERMODE_ADD,
} PARTICLERENDERMODE;

typedef struct PARTICLE {
	D3DXVECTOR3				pos;
	D3DXVECTOR3				mov;
	D3DXCOLOR				col;
	FLOAT					scl;
	BOOL					bAlive;
} PARTICLE;

typedef struct EMITTER {
	LPDIRECT3DTEXTURE9		pTex;
	LPDIRECT3DVERTEXBUFFER9	pVtx;
	D3DXVECTOR3				pos;
	D3DXVECTOR3				mov;
	D3DXCOLOR				col;
	FLOAT					size;
	PARTICLERENDERMODE		renderMode;

	PARTICLE				particles[GM_MAX_PARTICLES];
	INT						nParticlesCount;
	BOOL					bAlive;

	EMITTER(LPDIRECT3DTEXTURE9 pTexBuf, INT count, D3DXVECTOR3 pos, D3DXVECTOR3 mov, D3DXCOLOR col, FLOAT size);
	~EMITTER();

	void					Update();
	HRESULT					SetVertex();
	HRESULT					SetDiffuse(INT idx, D3DXCOLOR color);
	HRESULT					SetTexture(INT idx, D3DXVECTOR2* pUV);
	HRESULT					Draw(CAMERA* camera);
} EMITTER;

#endif // !__PARTICLE_H__
