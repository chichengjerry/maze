/***********************************************
 * [ui_game.cpp]
 * ゲーム中UI処理
 ***********************************************/
#include "ui_game.h"
#include "camera.h"
#include "game.h"
#include "maze.h"
#include "player.h"
#include "sound.h"
#include "stage.h"

#define IMAGE_HINT				_T("data/TEXTURE/ui/hint.png")
#define IMAGE_STAMINA			_T("data/TEXTURE/ui/stamina.png")
#define IMAGE_STAR				_T("data/TEXTURE/ui/star.png")
#define STAMINA_WIDTH			512
#define STAMINA_HEIGHT			128
#define STAMINA_BAR_WIDTH		420
#define STAMINA_BAR_HEIGHT		30
#define HINT_NUM				2

LPDIRECT3DTEXTURE9	MAINGAMEUI::pTexStamina = NULL;
LPDIRECT3DTEXTURE9	MAINGAMEUI::pTexStar = NULL;
LPDIRECT3DTEXTURE9	MAINGAMEUI::pTexHint = NULL;

const D3DXCOLOR COLOR_SET[3] = {
	D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f),
	D3DXCOLOR(1.0f, 1.0f, 0.0f, 1.0f),
	D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f),
};

MAINGAMEUI::MAINGAMEUI(MAINGAME* game)
{
	pGame = game;

	D3D::LoadTexture(&pTexHint, IMAGE_HINT);
	D3D::LoadTexture(&pTexStamina, IMAGE_STAMINA);
	D3D::LoadTexture(&pTexStar, IMAGE_STAR);

	D3DRECT rect = pGame->player->camera->GetRect();

	background = new IMAGE(NULL, &rect);
	bgOpacity = 1.0f;
	background->SetDiffuseColor(D3DXCOLOR(0.0f, 0.0f, 0.0f, bgOpacity));
	
	rect = { 0, CL_HEIGHT - STAMINA_HEIGHT, STAMINA_WIDTH, STAMINA_HEIGHT };
	stamina = new IMAGE(pTexStamina, &rect);
	rect = { STAMINA_BAR_HEIGHT * 2, CL_HEIGHT - STAMINA_BAR_HEIGHT, STAMINA_BAR_WIDTH, STAMINA_BAR_HEIGHT };
	staminaBar = new IMAGE(NULL, &rect);

	for (int i = 0; i < MAZE_STAR_NUM; ++i) {
		rect = { 220 + i * 50, CL_HEIGHT - 114, 50, 50 };
		stars[i] = new IMAGE(pTexStar, &rect);
	}

	for (int i = 0; i < HINT_NUM; ++i) {
		rect = { CL_WIDTH - 340, CL_HEIGHT - 116 + i * 48, 320, 48 };
		hint[i] = new IMAGE(pTexHint, &rect);
		hint[i]->nFrameIndex = i;
		hint[i]->nFrameTotal = HINT_NUM;
		hint[i]->SetTexture();
	}

	endUI = NULL;
}

MAINGAMEUI::~MAINGAMEUI()
{
	SAFE_DELETE(background);
	SAFE_DELETE(endUI);
	SAFE_DELETE(stamina);
	SAFE_DELETE(staminaBar);

	for (int i = 0; i < HINT_NUM; ++i) {
		SAFE_DELETE(hint[i]);
	}
	for (int i = 0; i < MAZE_STAR_NUM; ++i) {
		SAFE_DELETE(stars[i]);
	}

	SAFE_RELEASE(pTexHint);
	SAFE_RELEASE(pTexStamina);
	SAFE_RELEASE(pTexStar);
}

void MAINGAMEUI::Update()
{
	FLOAT percent = pGame->player->fStamina / pGame->player->fStaminaMax;
	FLOAT width = floorf(STAMINA_BAR_WIDTH * percent);
	D3DRECT rect = { STAMINA_BAR_HEIGHT * 2, CL_HEIGHT - STAMINA_BAR_HEIGHT * 2, (LONG)width, STAMINA_BAR_HEIGHT };
	
	D3DXCOLOR dif;
	if (percent > 0.75f) dif = COLOR_SET[2];
	else if(percent > 0.5f) D3DXColorLerp(&dif, &COLOR_SET[1], &COLOR_SET[2], (percent - 0.5f) * 4.0f);
	else if(percent > 0.25f) D3DXColorLerp(&dif, &COLOR_SET[0], &COLOR_SET[1], (percent - 0.25f) * 4.0f);
	else dif = COLOR_SET[0];

	staminaBar->SetVertex(&rect);
	staminaBar->SetDiffuseColor(dif);
	stamina->SetDiffuseColor(dif);

	if (pGame->bGameOver) {
		if (!endUI) {
			endUI = new GAMEENDUI(pGame);
		}
		else if (bgOpacity < 1.0f) {
			bgOpacity += 1.0f * D3D::fAnimationRate;
		}
		else {
			bgOpacity = 1.0f;
		}
	}
	else {
		SAFE_DELETE(endUI);

		if (bgOpacity > 0.0f) {
			bgOpacity -= 0.5f * D3D::fAnimationRate;
		}
		else bgOpacity = 0.0f;
	}
	background->SetDiffuseColor(D3DXCOLOR(0.0f, 0.0f, 0.0f, bgOpacity));

	if (endUI) endUI->Update();
}

HRESULT MAINGAMEUI::Draw()
{
	LPDIRECT3DDEVICE9 pDevice = D3D::GetDevice();
	PLAYER* player = pGame->player;
	stamina->Draw();
	staminaBar->Draw();

	for (int i = 0; i < MAZE_STAR_NUM; ++i) {
		if(stars[i] && i < player->nStarGained) stars[i]->Draw();
	}

	if (player->fStamina / player->fStaminaMax < 0.25f && player->nStarGained) hint[0]->Draw();
	if (!player->bJumping) hint[1]->Draw();
	if (bgOpacity >= 0.0f) background->Draw();

	if (endUI) endUI->Draw();

#if _DEBUG
	WCHAR str[256];

	if (!pGame->bGameOver) {
		swprintf_s(str, _T(" x: %f, y: %f, z: %f \n Star gained: %d \n Energy: %.2f, TimeElapsed: %.0f"),
			player->srt.pos.x, player->srt.pos.y, player->srt.pos.z,
			player->nStarGained,
			player->fStamina / player->fStaminaMax,
			player->fTimeElapsed);
		D3D::ShowText(str, 0, 0);
	}
#endif // _DEBUG

	return S_OK;
}

#define IMAGE_STAT			_T("data/TEXTURE/ui/stat.png")
#define IMAGE_RESULT		_T("data/TEXTURE/ui/result.png")
#define IMAGE_RESULTMENU		_T("data/TEXTURE/ui/menu-end.png")

LPDIRECT3DTEXTURE9	GAMEENDUI::pTexStat = NULL;
LPDIRECT3DTEXTURE9	GAMEENDUI::pTexEndMenu = NULL;
LPDIRECT3DTEXTURE9	GAMEENDUI::pTexResult = NULL;

GAMEENDUI::GAMEENDUI(MAINGAME * game)
{
	const DWORD DIGIT_GAP = 35;
	const DWORD DIGIT_SIZE = 60;

	D3D::LoadTexture(&pTexStat, IMAGE_STAT);
	D3D::LoadTexture(&pTexEndMenu, IMAGE_RESULTMENU);
	D3D::LoadTexture(&pTexResult, IMAGE_RESULT);

	pGame = game;
	ZeroMemory(&stat, sizeof(stat));
	ZeroMemory(&timeUsed, sizeof(timeUsed));
	ZeroMemory(&score, sizeof(score));
	ZeroMemory(&starGained, sizeof(starGained));

	D3DRECT rect = { CL_WIDTH / 2 - 256, 0, 512, 256 };
	result = new IMAGE(pTexResult, &rect);
	result->nFrameTotal = 2;

	if (pGame->iGameResult == GAME_RESULT_WIN) {
		result->nFrameIndex = 1;
		result->SetDiffuseColor(D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f));
		// Game clear
		rect = { CL_WIDTH / 2 - 512, CL_HEIGHT / 4, 512, 320 };
		stat = new IMAGE(pTexStat, &rect);

		for (int i = 0; i < MAZE_STAR_NUM; i++) {
			rect = { (LONG)(CL_WIDTH * 3 / 4 + (i - 2) * DIGIT_SIZE), (CL_HEIGHT / 4), DIGIT_SIZE, DIGIT_SIZE };
			starGained[i] = new IMAGE(pGame->gameUI->pTexStar, &rect);
		}

		for (int i = 0; i < UI_MAX_DIGITS; ++i) {
			rect = { (LONG)(CL_WIDTH * 3 / 4 + i * DIGIT_GAP), (CL_HEIGHT / 4 + 80), DIGIT_SIZE, DIGIT_SIZE };
			timeUsed[i] = new DIGITIMAGE(0, &rect);
			rect = { (LONG)(CL_WIDTH * 3 / 4 + i * DIGIT_GAP), (CL_HEIGHT / 4 +  80 * 3), DIGIT_SIZE, DIGIT_SIZE };
			score[i] = new DIGITIMAGE(0, &rect);
		}
	}
	else {
		result->nFrameIndex = 0;
		result->SetDiffuseColor(D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f));
		// Game over
	}
	result->SetTexture();

	for (int i = 0; i < RESULTMENU_COUNT; ++i) {
		rect = { CL_WIDTH / 2 - 256, CL_HEIGHT / 2 + 96 * (i + 1), 512, 128 };
		resultMenu[i] = new IMAGE(pTexEndMenu, &rect);
		resultMenu[i]->nFrameIndex = i;
		resultMenu[i]->nFrameTotal = RESULTMENU_COUNT;
		resultMenu[i]->SetTexture();
	}
	resultMenuSelected = 0;
}

GAMEENDUI::~GAMEENDUI()
{
	SAFE_DELETE(result);
	SAFE_DELETE(stat);
	for (int i = 0; i < UI_MAX_DIGITS; ++i) {
		SAFE_DELETE(score[i]);
		SAFE_DELETE(timeUsed[i]);
	}
	for (int i = 0; i < MAZE_STAR_NUM; i++) {
		SAFE_DELETE(starGained[i]);
	}

	for (int i = 0; i < RESULTMENU_COUNT; ++i) {
		SAFE_DELETE(resultMenu[i]);
	}
}

void _update_digit(DIGITIMAGE* p[], int num) {

	for (int i = 0; i < UI_MAX_DIGITS; ++i) {
		INT k = num % 10;
		p[UI_MAX_DIGITS - i - 1]->SetDigit(k);
		num /= 10;
	}
}

void GAMEENDUI::Update()
{
	if (DINPUT::KeyTriggered(DIK_UP) || DINPUT::ButtonTriggered(BUTTON_MOVE_UP)) {
		resultMenuSelected = (resultMenuSelected + RESULTMENU_COUNT - 1) % RESULTMENU_COUNT;
		DSOUND::Play(SOUND_CLICK);
	}
	if (DINPUT::KeyTriggered(DIK_DOWN) || DINPUT::ButtonTriggered(BUTTON_MOVE_DOWN)) {
		resultMenuSelected = (resultMenuSelected + 1) % RESULTMENU_COUNT;
		DSOUND::Play(SOUND_CLICK);
	}
	if (DINPUT::KeyTriggered(DIK_RETURN) || DINPUT::ButtonTriggered(BUTTON_START | BUTTON_A)) {
		switch (resultMenuSelected) {
		case RESULTMENU_RETRY:
			pGame->SetGame(pGame->iGameSeed);
			break;
		case RESULTMENU_BACK:
			pGame->pStage->SetStage(STAGE_TITLE);
			break;
		}
		DSOUND::Play(SOUND_CLICK);
		return;
	}

	if (pGame->iGameResult == GAME_RESULT_WIN) {
		// Game clear
		PLAYER* player = pGame->player;

		FLOAT fTimePenalty = floorf(min(0.0f, player->fStaminaMax - player->fTimeElapsed) * 5.0f);
		FLOAT fStaminaBonus = floorf(100.f * (1.0f + player->fStamina / player->fStaminaMax));
		FLOAT fScore = roundf(max(0, fTimePenalty + fStaminaBonus + player->nStarGained * 200.0f));

		_update_digit(timeUsed, (int)player->fTimeElapsed);
		_update_digit(score, (int)fScore);
	}
	else {
		// Game over
	}

	for (int i = 0; i < RESULTMENU_COUNT; i++) {
		if (resultMenuSelected == i) {
			resultMenu[i]->SetDiffuseColor(D3DXCOLOR(D3DCOLOR_RGBA(238, 229, 109, 255)));
		}
		else {
			resultMenu[i]->SetDiffuseColor(D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f));
		}
		resultMenu[i]->Update();
	}

}

void _draw_digit(DIGITIMAGE* p[]) {
	for (int i = 0, leadZero = 1; i < UI_MAX_DIGITS; ++i) {
		if (p[i]->digit == 0 && leadZero) continue;
		leadZero = 0;
		if (p[i]) p[i]->Draw();
	}
}

HRESULT GAMEENDUI::Draw()
{
	result->Draw();

	if (pGame->iGameResult == GAME_RESULT_WIN) {
		stat->Draw();
		for (int i = 0; i < MAZE_STAR_NUM; i++) {
			if(starGained[MAZE_STAR_NUM - 1 - i] && i < pGame->player->nStarGained) starGained[MAZE_STAR_NUM - 1 - i]->Draw();
		}

		_draw_digit(timeUsed);
		_draw_digit(score);
	}

	for (int i = 0; i < RESULTMENU_COUNT; ++i) {
		if(resultMenu[i]) resultMenu[i]->Draw();
	}
#if _DEBUG
	PLAYER* player = pGame->player;
	WCHAR str[256];

	FLOAT fTimePenalty = floorf(min(0.0f, player->fStaminaMax - player->fTimeElapsed) * 5.0f);
	FLOAT fStaminaBonus = floorf(100.f * (1.0f + player->fStamina / player->fStaminaMax));
	FLOAT fScore = roundf(max(0, fTimePenalty + fStaminaBonus + player->nStarGained * 200.0f));

	swprintf_s(str, _T(" %s \n Time Penalty: %.0f, Stamina Bonus: %.2f, Final Score: %.0f"),
		pGame->iGameResult ? _T("GAME OVER.") : _T("Congratulations!"),
		fTimePenalty, fStaminaBonus, fScore);

	D3D::ShowText(str, 0, 0);
#endif // _DEBUG

	return D3D_OK;
}
