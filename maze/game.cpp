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
	bGameReady = FALSE;
	SetGame(iGameSeed);
}

MAINGAME::~MAINGAME()
{
	SAFE_DELETE(gameUI);
	SAFE_DELETE(maze);
	SAFE_DELETE(player);
	SAFE_DELETE(sky);

	DSOUND::StopAll();
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
	gameUI->Update();
}

HRESULT MAINGAME::Draw(void)
{
	LPDIRECT3DDEVICE9 pDevice = D3D::GetDevice();

	CAMERA* camera = player->camera;

	camera->SetCamera();

	sky->Draw(camera);

	D3D::EnableFog(sky->light->GetFogColor());
			
	maze->Draw(camera);

	D3D::DisableFog();

	player->Draw(camera);

	gameUI->Draw();

	return S_OK;
}

void MAINGAME::SetGame(unsigned int seed)
{
	bGameOver = FALSE;
	bGameReady = FALSE;
	int size = 2 * MAZE_SIZE + 1;

	if (maze && player && gameUI) {
		SAFE_DELETE(gameUI);
		SAFE_DELETE(maze);
		SAFE_DELETE(player);
		SAFE_DELETE(sky);
	}

	maze = new MAZE(size, size, seed);
	player = new PLAYER(this, 0, 1);

	int x = GET_RANDOM(0, 1);
	int y = GET_RANDOM(0, 1);

	//player->SetPosition(1, 1);
	//maze->SetGoal(31, 31);
	maze->SetGoal((1 - x) * (size - 3) + 1, (1 - y) * (size - 3) + 1);
	player->SetPosition(x * (size - 3) + 1, y * (size - 3) + 1);

	maze->SetTraps();

	gameUI = new MAINGAMEUI(this); 
	sky = new SKY();

	DSOUND::Play(BGM_GAME);
}

void MAINGAME::GameOver(int playerId, BOOL result)
{
	bGameOver = TRUE;
	iGameResult = result;
}
