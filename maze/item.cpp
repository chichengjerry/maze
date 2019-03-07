#include "item.h"
#include "camera.h"
#include "game.h"
#include "maze.h"
#include "player.h"

LPDIRECT3DTEXTURE9 ITEM::pTex = NULL;
int ITEM::cnt = 0;

ITEM::ITEM(MAZE * maze, int x, int y)
{
	D3DXVECTOR3 pos;

	if (!cnt++) {
		D3D::LoadTexture(&pTex, _T("data/TEXTURE/glow.png"));
	}

	pMaze = maze;
	picked = FALSE;
	pos = pMaze->GetCellPosition(x, y);
	pos.y *= 0.5f;
	srt = SRT(pos);
	fGlowTime = 0.0f;
	fSize = 64.0f;

	glow = new EMITTER(pTex, 1, srt.pos, D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), fSize);

	mdl = new MODEL(_T("data/MODEL/star.x"), NULL);
	mdl->srt = srt;
}

ITEM::~ITEM()
{
	SAFE_DELETE(glow);
	SAFE_DELETE(mdl);
	if (!--cnt) {
		SAFE_RELEASE(pTex);
	}
}

void ITEM::Update(PLAYER* player)
{
	FLOAT delta = D3DXToRadian(90.0f) * D3D::fAnimationRate;
	FLOAT glowOpacity = cosf(fGlowTime) * 0.25f + 0.75f;

	glow->SetDiffuse(0, D3DXCOLOR(1.0f, 1.0f, 1.0f, glowOpacity));
	fGlowTime += delta;

	glow->particles[0].pos.y += cosf(fGlowTime) * 0.25f;
	mdl->srt.pos.y += cosf(fGlowTime) * 0.25f;
	mdl->srt.rot.y += delta;

	// check if pickable
	D3DXVECTOR3 d = player->srt.pos - srt.pos; d.y = 0.0f;

	if (!picked && D3DXVec3LengthSq(&d) < fSize * fSize * 0.25f) {
		picked = TRUE;
		player->AddStar();
	}
}

HRESULT ITEM::Draw(CAMERA* camera)
{
	if (!picked) {
		glow->Draw(camera);
		mdl->Draw(NULL);
	}

	return D3D_OK;
}
