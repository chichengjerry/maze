/***********************************************
 * [ui_game.h]
 * ゲーム中UI処理
 ***********************************************/
#ifndef __UI_GAME_H__
#define __UI_GAME_H__

#include "main.h"
#include "core.h"
#include "d3d.h"
#include "gamemain.h"
#include "image.h"
#include "ui_digits.h"

#define UI_MAX_DIGITS			4

struct GAMEENDUI;

typedef struct MAINGAMEUI {
	static LPDIRECT3DTEXTURE9	pTexStamina;
	static LPDIRECT3DTEXTURE9	pTexStar;
	static LPDIRECT3DTEXTURE9	pTexHint;

	MAINGAME*					pGame;
	GAMEENDUI*					endUI;
	IMAGE*						background;
	FLOAT						bgOpacity;

	IMAGE*						staminaBar;
	IMAGE*						stamina;
	IMAGE*						hint[2];

	IMAGE*						stars[MAZE_STAR_NUM];
	MAINGAMEUI(MAINGAME* game);
	~MAINGAMEUI();

	void						Update();
	HRESULT						Draw();

} MAINGAMEUI;

enum {
	RESULTMENU_RETRY,
	RESULTMENU_BACK,

	RESULTMENU_COUNT
};

typedef struct GAMEENDUI {
	static LPDIRECT3DTEXTURE9	pTexStat;
	static LPDIRECT3DTEXTURE9	pTexEndMenu;
	static LPDIRECT3DTEXTURE9	pTexResult;

	MAINGAME*					pGame;
	IMAGE*						result;
	IMAGE*						resultMenu[RESULTMENU_COUNT];
	UINT						resultMenuSelected;
	IMAGE*						stat;
	IMAGE*						starGained[MAZE_STAR_NUM];
	DIGITIMAGE*					score[UI_MAX_DIGITS];
	DIGITIMAGE*					timeUsed[UI_MAX_DIGITS];
	DIGITIMAGE*					route[UI_MAX_DIGITS];

	GAMEENDUI(MAINGAME* game);
	~GAMEENDUI();

	void						Update();
	HRESULT						Draw();
} GAMEENDUI;

#endif // !__UI_GAME_H__
