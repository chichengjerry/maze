#include "ui_game.h"
#include "camera.h"
#include "game.h"
#include "maze.h"
#include "player.h"

#define IMAGE_STRING		_T("data/TEXTURE/ui/blank.png")
#define IMAGE_RESULT		_T("data/TEXTURE/ui/blank.png")

LPDIRECT3DTEXTURE9	MAINGAMEUI::pTexStatus = NULL;
LPDIRECT3DTEXTURE9	MAINGAMEUI::pTexResult = NULL;

const DWORD DIGIT_GAP = 24;
const DWORD DIGIT_SIZE = 32;
const DWORD ELEMENT_SIZE = 48;
const DWORD TEXT_SIZE = 96;

MAINGAMEUI::MAINGAMEUI(MAINGAME* game)
{
	pGame = game;

	LPDIRECT3DDEVICE9 pDevice = D3D::GetDevice();

	D3D::LoadTexture(&pTexStatus, IMAGE_STRING);
	D3D::LoadTexture(&pTexResult, IMAGE_RESULT);

	D3DRECT rect = pGame->player->camera->GetRect();

	fadeOut = new IMAGE(NULL, &rect);
	fadeVal = 1.0f;
	fadeOut->SetDiffuseColor(D3DXCOLOR(0.0f, 0.0f, 0.0f, fadeVal));
}

MAINGAMEUI::~MAINGAMEUI()
{
	SAFE_RELEASE(pTexStatus);
	SAFE_RELEASE(pTexResult);

	SAFE_DELETE(fadeOut);
}

void MAINGAMEUI::Update()
{
	if (fadeVal >= 0.0f) {
		fadeVal -= 0.5f * D3D::fAnimationRate;
		fadeOut->SetDiffuseColor(D3DXCOLOR(0.0f, 0.0f, 0.0f, fadeVal));
	}
}

HRESULT MAINGAMEUI::Draw()
{
	LPDIRECT3DDEVICE9 pDevice = D3D::GetDevice();

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
