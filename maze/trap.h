#ifndef __TRAP_H__
#define __TRAP_H__

#include "main.h"
#include "core.h"
#include "d3d.h"
#include "gamemain.h"

typedef struct TRAP {
	LPDIRECT3DVERTEXBUFFER9		pVtxBuffer;
	MAZE*						pMaze;
	D3DXVECTOR3					pos;
	int tx;
	int ty;

	HRESULT						_set_vertex();
	TRAP(MAZE* maze, int x, int y);
	~TRAP();

	void	Update();

	HRESULT Draw();
} TRAP;


#endif // !__TRAP_H__
