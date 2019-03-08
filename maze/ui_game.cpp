#include "ui_game.h"
#include "camera.h"
#include "game.h"
#include "maze.h"
#include "player.h"

#define IMAGE_STRING		_T("data/TEXTURE/ui/blank.png")
#define IMAGE_STAR			_T("data/TEXTURE/ui/star.png")
#define STAMINA_WIDTH		480.0f

LPDIRECT3DTEXTURE9	MAINGAMEUI::pTexStatus = NULL;
LPDIRECT3DTEXTURE9	MAINGAMEUI::pTexStar = NULL;

const DWORD DIGIT_GAP = 24;
const DWORD DIGIT_SIZE = 32;
const DWORD ELEMENT_SIZE = 48;
const DWORD TEXT_SIZE = 96;

const D3DXCOLOR COLOR_SET[3] = {
	D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f),
	D3DXCOLOR(1.0f, 1.0f, 0.0f, 1.0f),
	D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f),
};

MAINGAMEUI::MAINGAMEUI(MAINGAME* game)
{
	pGame = game;

	LPDIRECT3DDEVICE9 pDevice = D3D::GetDevice();

	D3D::LoadTexture(&pTexStatus, IMAGE_STRING);
	D3D::LoadTexture(&pTexStar, IMAGE_STAR);

	D3DRECT rect = pGame->player->camera->GetRect();

	fadeOut = new IMAGE(NULL, &rect);
	fadeVal = 1.0f;
	fadeOut->SetDiffuseColor(D3DXCOLOR(0.0f, 0.0f, 0.0f, fadeVal));
	
	rect = { 32, CL_HEIGHT - 64, (LONG)STAMINA_WIDTH, 32 };
	stamina = new IMAGE(NULL, &rect);

	for (int i = 0; i < MAZE_STAR_NUM; ++i) {
		rect = { CL_WIDTH - 96 - i * 64, CL_HEIGHT - 96, 64, 64 };
		stars[i] = new IMAGE(pTexStar, &rect);
		starVisible[i] = FALSE;
	}
}

MAINGAMEUI::~MAINGAMEUI()
{
	SAFE_RELEASE(pTexStatus);
	SAFE_RELEASE(pTexStar);

	SAFE_DELETE(fadeOut);
	SAFE_DELETE(stamina);

	for (int i = 0; i < MAZE_STAR_NUM; ++i) {
		SAFE_DELETE(stars[i]);
	}
}

void MAINGAMEUI::Update()
{
	if (fadeVal >= 0.0f) {
		fadeVal -= 0.5f * D3D::fAnimationRate;
		fadeOut->SetDiffuseColor(D3DXCOLOR(0.0f, 0.0f, 0.0f, fadeVal));
	}
	
	FLOAT percent = pGame->player->fStamina / pGame->player->fStaminaMax;
	FLOAT width = floorf(STAMINA_WIDTH * percent);
	D3DRECT rect = { 32, CL_HEIGHT - 64, (LONG)width, 32 };
	stamina->SetVertex(&rect);

	D3DXCOLOR dif;
	if (percent > 0.75f) dif = COLOR_SET[2];
	else if(percent > 0.5f) D3DXColorLerp(&dif, &COLOR_SET[1], &COLOR_SET[2], (percent - 0.5f) * 4.0f);
	else if(percent > 0.25f) D3DXColorLerp(&dif, &COLOR_SET[0], &COLOR_SET[1], (percent - 0.25f) * 4.0f);
	else dif = COLOR_SET[0];

	stamina->SetDiffuseColor(dif);
	stamina->Update();

	for (int n = pGame->player->nStarGained; n; --n) {
		starVisible[n - 1] = TRUE;
	}
}

HRESULT MAINGAMEUI::Draw()
{
	LPDIRECT3DDEVICE9 pDevice = D3D::GetDevice();

	stamina->Draw();

	for (int i = 0; i < MAZE_STAR_NUM; ++i) {
		if(stars[i] && starVisible[i]) stars[i]->Draw();
	}

	if (fadeVal >= 0.0f) fadeOut->Draw();

	PLAYER* player = pGame->player;

#if _DEBUG

	WCHAR str[256];

	if (!pGame->bGameOver) {
		swprintf_s(str, _T(" Star gained: %d \n Energy: %.2f, TimeElapsed: %.0f"),
			player->nStarGained,
			player->fStamina / player->fStaminaMax,
			player->fTimeElapsed);
	}
	else {
		FLOAT timePenalty = min(0.0f, player->fStaminaMax * 2.0f - player->fTimeElapsed);
		FLOAT staminaBonus = (1.5f + player->fStamina / player->fStaminaMax);
		FLOAT score = max(0, (floorf(timePenalty) + player->nStarGained * 100.0f) * staminaBonus);

		swprintf_s(str, _T(" %s \n Time Penalty: %.0f, Stamina Bonus: %.2f, Final Score: %.0f"),
			pGame->iGameResult ? _T("GAME OVER.") : _T("Congratulations!"),
			timePenalty, staminaBonus, score);
	}
	D3D::ShowText(str, 0, 0);

#endif // _DEBUG

	return S_OK;
}
