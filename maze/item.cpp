#include "item.h"
#include "camera.h"
#include "game.h"
#include "maze.h"
#include "player.h"

LPDIRECT3DTEXTURE9 ITEM::pTex = NULL;
int ITEM::cnt = 0;

#define ITEM_LIGHT_OFFSET 0x1000

ITEM::ITEM(MAZE * maze, int x, int y)
{
	if (!cnt++) {
		D3D::LoadTexture(&pTex, _T("data/TEXTURE/glow.png"));
	}
	id = cnt;
	pMaze = maze;
	picked = FALSE;
	srt = SRT(pMaze->GetCellPosition(x, y));
	fGlowTime = 0.0f;
	fSize = 64.0f;

	glow = new EMITTER(pTex, 1, srt.pos, D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), fSize);

	mdl = new MODEL(_T("data/MODEL/star.x"), NULL);
	mdl->srt = srt;

	ZeroMemory(&light, sizeof(light));
	light.Type = D3DLIGHT_POINT;
	light.Diffuse = D3DXCOLOR(1.0f, 1.0f, 0.0f, 1.0f);
	light.Position = srt.pos;
	light.Range = 128.0f;
	
	LPDIRECT3DDEVICE9 pDevice = D3D::GetDevice();

	pDevice->SetLight(ITEM_LIGHT_OFFSET + id, &light);

	pDevice->LightEnable(ITEM_LIGHT_OFFSET + id, TRUE);
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
		LPDIRECT3DDEVICE9 pDevice = D3D::GetDevice();
		pDevice->LightEnable(ITEM_LIGHT_OFFSET + id, FALSE);

		player->AddStar();
		picked = TRUE;
	}
}

HRESULT ITEM::Draw(CAMERA* camera)
{
	LPDIRECT3DDEVICE9 pDevice = D3D::GetDevice();

	pDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
	if (!picked) {
		glow->Draw(camera);
		mdl->Draw(NULL);
	}
	pDevice->SetRenderState(D3DRS_FOGENABLE, TRUE);

	return D3D_OK;
}
