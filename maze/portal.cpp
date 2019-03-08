#include "portal.h"
#include "camera.h"
#include "maze.h"

LPDIRECT3DTEXTURE9 PORTAL::pTex = NULL;

PORTAL::PORTAL(MAZE * pMaze, int x, int y)
{
	if (!pTex) D3D::LoadTexture(&pTex, _T("data/TEXTURE/glow01.jpg"));

	maze = pMaze;
	pos = maze->GetCellPosition(x, y);
	pos.y = 0.25f * MAZE_BLOCK_HEIGHT;

	dots = new EMITTER(pTex, GM_MAX_PARTICLES, pos, D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXCOLOR(0.0f, 0.5f, 0.125f, 1.0f), 5.0f);

	for (int i = 0; i < dots->nParticlesCount; ++i) {
		FLOAT r = 32.0f + GET_RANDOM(0, 16);	// range [50.0, 100.0]
		FLOAT theta = D3DXToRadian(GET_RANDOM(1, 360));

		dots->particles[i].pos.x += r * cosf(theta);
		dots->particles[i].pos.y = 0.25f * MAZE_BLOCK_HEIGHT;
		dots->particles[i].pos.z += r * sinf(theta);

	}
}

PORTAL::~PORTAL()
{
	SAFE_RELEASE(pTex);
	SAFE_DELETE(dots);
}

void PORTAL::Update()
{
	D3DXVECTOR3 vec;
	D3DXVECTOR4 temp;
	D3DXMATRIX mtx;
	for (int i = 0; i < dots->nParticlesCount; ++i) {
		FLOAT d = 1.0f * GET_RANDOM(10, 30);
		vec = dots->particles[i].pos - dots->pos;
		D3DXMatrixRotationY(&mtx, D3DXToRadian(d) * D3D::fAnimationRate);
		D3DXVec3Transform(&temp, &vec, &mtx);
		vec = D3DXVECTOR3(temp.x, 0.0f, temp.z);
		dots->particles[i].pos = dots->pos + vec;
	}
}

HRESULT PORTAL::Draw(CAMERA* camera)
{
	return dots->Draw(camera);
}
