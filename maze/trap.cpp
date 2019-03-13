#include "trap.h"
#include "maze.h"

HRESULT TRAP::_set_vertex()
{
	HRESULT hr;

	LPDIRECT3DDEVICE9 pDevice = D3D::GetDevice();

	VERTEX_3D* pVtx;
	D3DCHECK(pDevice->CreateVertexBuffer(sizeof(VERTEX_3D) * NUM_VERTEX, D3DUSAGE_WRITEONLY, FVF_VERTEX_3D, D3DPOOL_MANAGED, &pVtxBuffer, NULL));
	D3DCHECK(pVtxBuffer->Lock(0, 0, (void**)&pVtx, 0));

	pVtx[0].vtx = pos + D3DXVECTOR3(-MAZE_BLOCK_SIZE / 2, 0.0f, -MAZE_BLOCK_SIZE / 2);
	pVtx[1].vtx = pos + D3DXVECTOR3(-MAZE_BLOCK_SIZE / 2, 0.0f,  MAZE_BLOCK_SIZE / 2);
	pVtx[2].vtx = pos + D3DXVECTOR3( MAZE_BLOCK_SIZE / 2, 0.0f, -MAZE_BLOCK_SIZE / 2);
	pVtx[3].vtx = pos + D3DXVECTOR3( MAZE_BLOCK_SIZE / 2, 0.0f,  MAZE_BLOCK_SIZE / 2);

	pVtx[0].nor =
	pVtx[1].nor =
	pVtx[2].nor =
	pVtx[3].nor = D3DXVECTOR3(0.0f, 1.0f, 0.0f);

	pVtx[0].dif =
	pVtx[1].dif =
	pVtx[2].dif =
	pVtx[3].dif = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);

	pVtx[0].tex = D3DXVECTOR2(0.0f, 0.0f);
	pVtx[1].tex = D3DXVECTOR2(0.0f, 1.0f);
	pVtx[2].tex = D3DXVECTOR2(1.0f, 0.0f);
	pVtx[3].tex = D3DXVECTOR2(1.0f, 1.0f);

	D3DCHECK(pVtxBuffer->Unlock());

	return D3D_OK;
}

TRAP::TRAP(MAZE * maze, int x, int y)
{
	pMaze = maze;
	tx = x;
	ty = y;
	pVtxBuffer = NULL;
	pos = pMaze->GetCellPosition(tx, ty);
	pos.y = 1.0f;

	_set_vertex();
}

TRAP::~TRAP()
{
	SAFE_RELEASE(pVtxBuffer);
}

void TRAP::Update()
{
}

HRESULT TRAP::Draw()
{
	HRESULT hr;
	LPDIRECT3DDEVICE9 pDevice = D3D::GetDevice();

	D3DXMATRIX mtx;
	D3DXMatrixIdentity(&mtx);

	D3DCHECK(pDevice->SetTransform(D3DTS_WORLD, &mtx));
	D3DCHECK(pDevice->SetStreamSource(0, pVtxBuffer, 0, sizeof(VERTEX_3D)));
	D3DCHECK(pDevice->SetFVF(FVF_VERTEX_3D));

	D3DCHECK(pDevice->SetTexture(0, NULL));

	D3DCHECK(pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, NUM_POLYGON));

	return D3D_OK;
}
