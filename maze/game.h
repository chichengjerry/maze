/***********************************************
 * [game.h]
 * ゲーム処理
 ***********************************************/

#ifndef __GAME_H__
#define __GAME_H__

#include "main.h"
#include "core.h"
#include "gamemain.h"

enum GAME_RESULT {
	GAME_RESULT_WIN,
	GAME_RESULT_LOSE,
	GAME_RESULT_DRAW,

	GAME_RESULT_NUM
};

enum GAME_LEVEL{
	LEVEL_EASY,
	LEVEL_HARD,
};

struct GAMESTAGE;

struct MAINGAME {
	GAMESTAGE*				pStage;
	MAINGAMEUI*				gameUI;
	PLAYER*					player;
	SKY*					sky;
	MAZE*					maze;
	BOOL					bGameOver;
	UINT					iGameResult;
	UINT					iGameSeed;
	INT						nTimeRemain;
	INT						nTimeLast;
	INT						gameLevel;
	BOOL					bGameReady;

	MAINGAME(GAMESTAGE* state, int level);
	~MAINGAME();

	void					Update(void);
	HRESULT					Draw(void);

	void					SetGame(unsigned int seed);
	void					GameOver(BOOL win);
};

#endif // !__GAME_H__
