/***********************************************
 * [model.h]
 * プレイヤー処理
 ***********************************************/
#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "main.h"
#include "core.h"
#include "gamemain.h"
#include "node.h"

#define YAW_MAX_ANGLE		D3DXToRadian(180.0f)
#define YAW_STEP			D3DXToRadian(90.0f)

#define PITCH_MAX_ANGLE		D3DXToRadian(90.0f)
#define PITCH_STEP			D3DXToRadian(90.0f)

#define MOVE_MAX_SPEED		256.0f
#define MOVE_MIN_SPEED		0.0f
#define MOVE_STEP			256.0f

#define PLAYER_GRAVITY		10.0f
#define PLAYER_JUMP_SPEED	(PLAYER_GRAVITY * 0.75f)
#define PLAYER_SIZE			32.0f

//
// プレイヤー情報
//

using namespace std;

typedef struct PLAYER {
	MAINGAME*				pGame;
	MAZE*					pMaze;
	CAMERA*					camera;
	STAR*					guideStar;
	
#if _DEBUG
	LPD3DXLINE				pGuideLine;
#endif // _DEBUG
	
	vector<NODE*>			path;
	vector<D3DXVECTOR3>		guideVtx;
	DWORD					id;
	BOOL					bDoOnceFlag;
	BOOL					bMoveable;
	BOOL					bFalling;
	BOOL					bJumping;
	FLOAT					fSize;
	FLOAT					fMoveSpeed;
	FLOAT					fJumpSpeed;
	FLOAT					fFallingSpeed;
	int						nlastX;
	int						nlastY;
	int						nStarGained;
	SRT						srt;

	FLOAT					fStamina;
	FLOAT					fStaminaMax;
	FLOAT					fTimeElapsed;

	BOOL					_collision_check(D3DXVECTOR3 pos, D3DXVECTOR3& dir);
	D3DXVECTOR3				_get_desired_direction(D3DXVECTOR3 input, D3DXVECTOR3 nor);
	void					_update_consume(void);
	void					_update_gravity(void);
	void					_update_line(vector<NODE*> path);
	void					_update_movement(void);
	void					_update_path(void);
	BOOL					_is_in_trap();

	PLAYER(MAINGAME* pGame);
	~PLAYER();

	HRESULT					Draw(CAMERA* camera);
	void					Update();

	void					AddStar(void);
	void					GameWin(void);
	void					GameLose(void);
	void					SetPosition(int x, int y);
} PLAYER;

#endif // !__PLAYER_H__
