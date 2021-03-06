/***********************************************
 * [model.h]
 * モデル処理
 ***********************************************/
#ifndef __MODEL_H__
#define __MODEL_H__

#include "main.h"
#include "core.h"
#include "d3d.h"
#include "util.h"

//
// モデル情報
//
typedef struct MODEL {
	DWORD					nMatNum;
	LPD3DXBUFFER			pMatBuf;
	LPD3DXMESH				pMesh;
	LPDIRECT3DTEXTURE9		pTex;

	D3DXMATRIX				mtx;
	SRT						srt;

	MODEL(LPCWSTR modelSrc, LPCWSTR texSrc);
	~MODEL();

	HRESULT					LoadModel(LPCWSTR modelSrc, LPCWSTR textureSrc);
	HRESULT					Draw(D3DXMATRIX* pParentMatrix);
	void					Update();
} MODEL;

#endif // !__MODEL_H__
