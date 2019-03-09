#include "game.h"
#include "camera.h"
#include "light.h"
#include "maze.h"
#include "player.h"
#include "sky.h"
#include "sound.h"
#include "ui_game.h"
#include "stage.h"

MAINGAME::MAINGAME(GAMESTAGE* state, int level)
{
	iGameSeed = timeGetTime();
	pStage = state;
	gameLevel = level;
	maze = NULL;
	player = NULL;
	gameUI = NULL;
	sky = NULL;

	SetGame(iGameSeed);
}

MAINGAME::~MAINGAME()
{
	SAFE_DELETE(gameUI);
	SAFE_DELETE(maze);
	SAFE_DELETE(player);
	SAFE_DELETE(sky);
}

void MAINGAME::Update(void)
{
#if _DEBUG
	// Reset the current maze
	if (DINPUT::KeyTriggered(DIK_F5)) {
		SetGame(iGameSeed);
	}
#endif
	if (DINPUT::KeyTriggered(DIK_ESCAPE) || DINPUT::ButtonTriggered(BUTTON_BACK)) {
		pStage->SetStage(STAGE_TITLE);
	}
	else if (!bGameOver) {
		sky->Update();
		player->Update();
		maze->Update(player);
	}
	else {
		if (DINPUT::KeyTriggered(DIK_RETURN) || DINPUT::ButtonTriggered(BUTTON_START)) {
			pStage->SetStage(STAGE_TITLE);
		}
	}

	gameUI->Update();
}

HRESULT MAINGAME::Draw(void)
{
	LPDIRECT3DDEVICE9 pDevice = D3D::GetDevice();
	D3DVIEWPORT9 vp0;

	pDevice->GetViewport(&vp0);
	pDevice->EndScene();

	CAMERA* camera = player->camera;

	camera->SetCamera();

	D3DVIEWPORT9 vp = camera->viewport;

	RECT rect = { (LONG)vp.X, (LONG)vp.Y, (LONG)(vp.X + vp.Width), (LONG)(vp.Y + vp.Height) };

	pDevice->Clear(0, (D3DRECT*)&rect, (D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER), D3DCOLOR_RGBA(255, 255, 255, 255), 1.0f, 0);

	pDevice->BeginScene();

	pDevice->SetViewport(&vp);

	sky->Draw(camera);

	D3D::EnableFog(sky->light->GetFogColor());
			
	maze->Draw(camera);

	D3D::DisableFog();

	player->Draw(camera);

	gameUI->Draw();

	pDevice->EndScene();

	pDevice->Present(&rect, &rect, NULL, NULL);

	pDevice->SetViewport(&vp0);

	pDevice->BeginScene();

	return S_OK;
}

void MAINGAME::SetGame(unsigned int seed)
{
	bGameOver = FALSE;

	int size = 2 * MAZE_SIZE + 1;

	if (maze && player && gameUI) {
		SAFE_DELETE(gameUI);
		SAFE_DELETE(maze);
		SAFE_DELETE(player);
		SAFE_DELETE(sky);
	}

	maze = new MAZE(size, size, seed);

	int x = GET_RANDOM(0, 1);
	int y = GET_RANDOM(0, 1);

	maze->SetGoal((1 - x) * (size - 3) + 1, (1 - y) * (size - 3) + 1);
	player = new PLAYER(this, 0, 1);
	player->SetPosition(x * (size - 3) + 1, y * (size - 3) + 1);
	
	gameUI = new MAINGAMEUI(this); 
	sky = new SKY();
}

void MAINGAME::GameOver(int playerId, BOOL result)
{
	bGameOver = TRUE;
	iGameResult = result;
}
