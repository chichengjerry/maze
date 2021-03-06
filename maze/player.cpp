/***********************************************
 * [player.cpp]
 * プレイヤー処理
 ***********************************************/
#include "player.h"
#include "camera.h"
#include "game.h"
#include "input.h"
#include "item.h"
#include "maze.h"
#include "portal.h"
#include "sound.h"
#include "star.h"
#include "trap.h"

#define PLAYER_LIGHT_ID		0x10

//
// アクションリスト
//
typedef enum ACTION {
	ACTION_TURNLEFT,
	ACTION_TURNRIGHT,
	ACTION_LOOKUP,
	ACTION_LOOKDOWN,
	ACTION_MOVE_FORWARD,
	ACTION_MOVE_BACKWARD,
	ACTION_MOVE_LEFT,
	ACTION_MOVE_RIGHT,
	ACTION_JUMP,
	ACTION_CONSUME,

	ACTION_COUNT
} ACTION;

//
// キーボードマップ
//
BYTE KEYBOARDMAP[ACTION_COUNT] = { 
	DIK_LEFT, DIK_RIGHT, DIK_UP, DIK_DOWN,
	DIK_W, DIK_S, DIK_A, DIK_D,
	DIK_SPACE, DIK_E
};
//
// ゲームパッドマップ
//
DWORD GAMEPADMAP[ACTION_COUNT] = {
	BUTTON_ROTATE_LEFT, BUTTON_ROTATE_RIGHT, BUTTON_ROTATE_UP, BUTTON_ROTATE_DOWN,
	BUTTON_MOVE_UP, BUTTON_MOVE_DOWN, BUTTON_MOVE_LEFT, BUTTON_MOVE_RIGHT,
	BUTTON_RT, BUTTON_LB
};
//
// 初期化
//
PLAYER::PLAYER(MAINGAME* game)
{
	pGame = game;
	pMaze = game->maze;
	bDoOnceFlag = TRUE;
	bJumping = FALSE;
	bMoveable = FALSE;
	bFalling = TRUE;
	fSize = PLAYER_SIZE;
	fJumpSpeed = 0.0f;
	fFallingSpeed = 0.0f;
	fMoveSpeed = MOVE_MIN_SPEED;
	nStarGained = 0;
	fStamina = fStaminaMax = FLT_MAX;
	fTimeElapsed = 0.0f;

	RECT rect = { 0, 0, CL_WIDTH, CL_HEIGHT };
	camera = new CAMERA(this, &rect);


#if _DEBUG
	LPDIRECT3DDEVICE9 pDevice = D3D::GetDevice();
	pGuideLine = NULL;
	D3DXCreateLine(pDevice, &pGuideLine);
	pGuideLine->SetWidth(10.0f);
	pGuideLine->SetAntialias(TRUE);
#endif
	guideVtx.clear();

	guideStar = new STAR(pMaze, this);
}
//
// 解放
//
PLAYER::~PLAYER()
{
	SAFE_DELETE(camera);
	SAFE_DELETE(guideStar);
#if _DEBUG
	SAFE_RELEASE(pGuideLine);
#endif // _DEBUG
}
//
// 描画
//
HRESULT PLAYER::Draw(CAMERA* camera)
{
#if _DEBUG
	D3DXMATRIX mtx;
	D3DXMATRIX* mtxView = &camera->mtxView;
	D3DXMATRIX* mtxProj = &camera->mtxProj;

	D3DXMatrixMultiply(&mtx, mtxView, mtxProj);
	if (guideVtx.size() > 1) {
		pGuideLine->Begin();
		pGuideLine->DrawTransform((D3DXVECTOR3*)&guideVtx[0], guideVtx.size(), &mtx, D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f));
		pGuideLine->End();
	}
#endif // _DEBUG
	guideStar->Draw();

	return S_OK;
}
//
// 更新
//
void PLAYER::Update()
{
	if (pGame->bGameReady) {
		fTimeElapsed += D3D::fAnimationRate;
	}
	if (pGame->bGameReady && bDoOnceFlag) {
		bDoOnceFlag = FALSE;

		pGame->maze->SetItems(this);
		fStamina = fStaminaMax = (FLOAT)guideVtx.size();
	}
	_update_gravity();
	_update_consume();
	_update_movement();
	_update_path();

	guideStar->Update();
	camera->Update();
}
//
// 実際の移動方向を計算する。
//
inline D3DXVECTOR3 PLAYER::_get_desired_direction(D3DXVECTOR3 input, D3DXVECTOR3 nor)
{
	const static FLOAT DIRECTION_THRESHOLD = cosf(D3DXToRadian(30.0f));

	FLOAT dot = D3DXVec3Dot(&input, &nor);

	if (dot < 0.0f)
	{
		// 壁の裏と逆方向
		return input;
	}
	else if (dot < DIRECTION_THRESHOLD)
	{
		D3DXVECTOR3 intoWall = nor * dot;
		D3DXVECTOR3 alongWall = input - intoWall;
		return alongWall;
	}

	// 壁に直面する
	return D3DXVECTOR3(0.0f, 0.0f, 0.0f);
}
//
// 壁との当たり判定をする。当たった場合、プレイヤーの移動方向を調整する
//
inline BOOL PLAYER::_collision_check(D3DXVECTOR3 pos, D3DXVECTOR3& dir)
{
	BOOL hit = FALSE;
	D3DXVECTOR3 newPos = pos + dir;
	int x, z;
	FLOAT cx = newPos.x, cz = newPos.z;
	// プレイヤーのいるブロックをゲット
	pGame->maze->GetTypeFromPosition(newPos, x, z);

	for (int i = -1; i <= 1; ++i) {
		for (int j = -1; j <= 1; ++j) {
			if (!(i || j)) continue;	// 今いるブロックをスキップ（i == 0 && j == 0）

			if (pGame->maze->GetTypeFromIndex(x + j, z + i)) {
				// 壁のあるブロックで、ブロックと円の当たり判定
				D3DXVECTOR3 blk = pGame->maze->GetCenterPositionFromIndex(x + j, z + i);
				FLOAT bx = blk.x - MAZE_BLOCK_SIZE * 0.5f;
				FLOAT bz = blk.z - MAZE_BLOCK_SIZE * 0.5f;
				FLOAT dx = cx - max(bx, min(cx, bx + MAZE_BLOCK_SIZE));
				FLOAT dz = cz - max(bz, min(cz, bz + MAZE_BLOCK_SIZE));

				if (dx * dx + dz * dz < fSize * fSize) {
					hit = TRUE;
					if (
						abs(i) != abs(j) || // 側面のブロックを当たった
						(!pGame->maze->GetTypeFromIndex(x + j, z) && !pGame->maze->GetTypeFromIndex(x, z + i)) //コーナーのブロックを当たった
						)
					{
						// get normal of current wall
						D3DXVECTOR3 nor = D3DXVECTOR3((FLOAT)j, 0.0f, (FLOAT)i);
						D3DXVec3Normalize(&nor, &nor);

						// made player slide along the wall smoothly
						dir = _get_desired_direction(dir, nor);
					}
				}
			}
		}
	}
	return hit;
}
//
// スターの使用状況を更新する。
//
inline void PLAYER::_update_consume(void)
{
	if (DINPUT::KeyTriggered(KEYBOARDMAP[ACTION_CONSUME]) ||
		DINPUT::ButtonTriggered(GAMEPADMAP[ACTION_CONSUME]))
	{
		if (fStamina / fStaminaMax >= 0.25f) return;
		if (nStarGained > 0) {
			DSOUND::Play(SOUND_RESTORE);
			nStarGained -= 1;
			switch (pGame->gameLevel) {
			case LEVEL_HARD:
				fStamina += fStaminaMax * 0.25f;
				fStamina = min(fStamina, fStaminaMax);
				break;
			case LEVEL_EASY:
			default:
				fStamina = fStaminaMax;
				break;
			}
		}
	}
}
//
// プレイヤーの垂直方向の位置情報を更新する。
//
inline void PLAYER::_update_gravity(void)
{
	const FLOAT accel = PLAYER_GRAVITY * D3D::fAnimationRate;

	if (pGame->bGameOver) {
		return;
	}

	// 地面にいる
	if (!bFalling) {
		fFallingSpeed = 0.0f;

		if (!bJumping) {
			fJumpSpeed = 0.0f;

			if (bMoveable && DINPUT::KeyTriggered(KEYBOARDMAP[ACTION_JUMP]) || DINPUT::ButtonTriggered(GAMEPADMAP[ACTION_JUMP])) {
				bJumping = TRUE;
				fJumpSpeed = PLAYER_JUMP_SPEED;
				DSOUND::Play(SOUND_MOVEMENT);
			}
		}
	}
	else {
		// 墜落している
		fFallingSpeed += accel;
	}

	if (bJumping) {
		fJumpSpeed -= accel;
	}

	srt.pos.y += (fJumpSpeed - fFallingSpeed);

	if (srt.pos.y <= 0.0f) {
		if (_is_in_trap()) {
			// 落とし穴に墜落させる
			bFalling = TRUE;
			bMoveable = FALSE;
			if (srt.pos.y < -0.375f * MAZE_BLOCK_HEIGHT)
				pGame->GameOver(GAME_RESULT_LOSE);
		}
		else {
			// 着地した
			srt.pos.y = 0.0f;
			bFalling = FALSE;
			bJumping = FALSE;
		}
	}
	else {
		bFalling = TRUE;
	}
}
//
// ガイドラインを更新する。
//
inline void PLAYER::_update_line(vector<NODE*> path)
{
	if (!pGame->bGameReady) {
		pGame->bGameReady = TRUE;
		bMoveable = TRUE;
	}

	int nGuideLength = path.size();
	guideVtx.clear();
	for (int i = 0; i < nGuideLength; i++) {
		guideVtx.push_back(pMaze->GetCenterPositionFromIndex(path[i]->x, path[i]->y));
		guideVtx[i].y = 10.0f;
	}
}
//
// プレイヤーの動きを更新する。
//
inline void PLAYER::_update_movement()
{
	if (!pGame->bGameReady || !bMoveable || pGame->bGameOver) {
		return;
	}

	FLOAT rate = D3D::fAnimationRate;
	static float moveSoundTime = 0.0f;
	BOOL bMovement = FALSE;
	BOOL bSideMovement = FALSE;

	D3DXVECTOR3 dir = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

	// 前後移動
	if (DINPUT::KeyPressed(KEYBOARDMAP[ACTION_MOVE_FORWARD]) && DINPUT::KeyPressed(KEYBOARDMAP[ACTION_MOVE_BACKWARD]) ||
		DINPUT::ButtonPressed(GAMEPADMAP[ACTION_MOVE_FORWARD]) && DINPUT::ButtonPressed(GAMEPADMAP[ACTION_MOVE_BACKWARD])) {
	}
	else if (DINPUT::KeyPressed(KEYBOARDMAP[ACTION_MOVE_FORWARD]) ||
			DINPUT::ButtonPressed(GAMEPADMAP[ACTION_MOVE_FORWARD])) {
		bMovement = TRUE;
		dir.x = 1.0f;
	}
	else if (DINPUT::KeyPressed(KEYBOARDMAP[ACTION_MOVE_BACKWARD]) ||
		DINPUT::ButtonPressed(GAMEPADMAP[ACTION_MOVE_BACKWARD])) {
		bMovement = TRUE;
		dir.x = -1.0f;
	}

	// 左右移動
	if (DINPUT::KeyPressed(KEYBOARDMAP[ACTION_MOVE_LEFT]) && DINPUT::KeyPressed(KEYBOARDMAP[ACTION_MOVE_RIGHT]) ||
		DINPUT::ButtonPressed(GAMEPADMAP[ACTION_MOVE_LEFT]) && DINPUT::ButtonPressed(GAMEPADMAP[ACTION_MOVE_RIGHT])) {
	}
	else if (DINPUT::KeyPressed(KEYBOARDMAP[ACTION_MOVE_LEFT]) ||
		DINPUT::ButtonPressed(GAMEPADMAP[ACTION_MOVE_LEFT])) {
		bSideMovement = TRUE;
		dir.z = 1.0f;
	}
	else if (DINPUT::KeyPressed(KEYBOARDMAP[ACTION_MOVE_RIGHT]) ||
		DINPUT::ButtonPressed(GAMEPADMAP[ACTION_MOVE_RIGHT])) {
		bSideMovement = TRUE;
		dir.z = -1.0f;
	}

	// 垂直方向調整
	if (DINPUT::KeyPressed(KEYBOARDMAP[ACTION_LOOKUP]) && DINPUT::KeyPressed(KEYBOARDMAP[ACTION_LOOKDOWN]) ||
		DINPUT::ButtonPressed(GAMEPADMAP[ACTION_LOOKUP]) && DINPUT::ButtonPressed(GAMEPADMAP[ACTION_LOOKDOWN])) {
	}
	else if (DINPUT::KeyPressed(KEYBOARDMAP[ACTION_LOOKDOWN]) ||
		DINPUT::ButtonPressed(GAMEPADMAP[ACTION_LOOKDOWN]) ||
		DINPUT::MouseY() > 5) {
			
		srt.rot.x -= PITCH_STEP * rate;
		if (srt.rot.x <= -PITCH_MAX_ANGLE)
			srt.rot.x = -PITCH_MAX_ANGLE;
	}
	else if (DINPUT::KeyPressed(KEYBOARDMAP[ACTION_LOOKUP]) ||
		DINPUT::ButtonPressed(GAMEPADMAP[ACTION_LOOKUP]) ||
		DINPUT::MouseY() < -5) {
			
		srt.rot.x += PITCH_STEP * rate;
		if (srt.rot.x >= PITCH_MAX_ANGLE)
			srt.rot.x = PITCH_MAX_ANGLE;
	}

	// 水平方向調整
	if (DINPUT::KeyPressed(KEYBOARDMAP[ACTION_TURNLEFT]) && DINPUT::KeyPressed(KEYBOARDMAP[ACTION_TURNRIGHT]) ||
		DINPUT::ButtonPressed(GAMEPADMAP[ACTION_TURNLEFT]) && DINPUT::ButtonPressed(GAMEPADMAP[ACTION_TURNRIGHT])) {
	}
	else if (DINPUT::KeyPressed(KEYBOARDMAP[ACTION_TURNLEFT]) ||
		DINPUT::ButtonPressed(GAMEPADMAP[ACTION_TURNLEFT]) ||
		DINPUT::MouseX() < -5) {
		srt.rot.y -= YAW_STEP * rate;
	}
	else if (DINPUT::KeyPressed(KEYBOARDMAP[ACTION_TURNRIGHT]) ||
		DINPUT::ButtonPressed(GAMEPADMAP[ACTION_TURNRIGHT]) ||
		DINPUT::MouseX() > 5) {
		srt.rot.y += YAW_STEP * rate;
	}

	D3DXVECTOR4 temp;
	D3DXMATRIX mtx;
	D3DXVec3Normalize(&dir, &dir);
	D3DXMatrixRotationY(&mtx, srt.rot.y);
	D3DXVec3Transform(&temp, &dir, &mtx);

	dir = D3DXVECTOR3(temp.x, temp.y, temp.z);

	if (bMovement || bSideMovement) {
		fMoveSpeed += MOVE_STEP * rate;
		if (fMoveSpeed >= MOVE_MAX_SPEED)
			fMoveSpeed = MOVE_MAX_SPEED;
	}

	// 当たり判定をし、プレイヤーを望ましい方向に移動させる
	_collision_check(srt.pos, dir);

	// 移動させる
	dir *= fMoveSpeed * rate;
	srt.pos += dir;

	if (fabsf(dir.x) > 0.001f || fabsf(dir.z) > 0.001f) {
		moveSoundTime += rate;
		if (!bJumping && moveSoundTime > 0.75f) {
			DSOUND::Play(SOUND_MOVEMENT);
			moveSoundTime -= 0.75f;
		}
		fStamina -= 1.0f * rate;

		// スタミナのないときゲームオーバー
		if (fStamina <= 0.0f)
			pGame->GameOver(GAME_RESULT_LOSE);
	}
	else {
		fMoveSpeed = 0.0f;
		moveSoundTime = 0.0f;
	}
}

//
// プレイヤーとゴールの間の最短ルートを更新する。
//
inline void PLAYER::_update_path(void)
{
	int x, y;
	int w, h;
	pMaze->GetSize(w, h);
	pMaze->GetTypeFromPosition(srt.pos, x, y);
	NODE* start = pMaze->GetNode(x, y);
	NODE* goal = pMaze->GetNode(pMaze->goal.x, pMaze->goal.y);

	pMaze->FindPath(start, goal, path);
	if (!path.empty())
		_update_line(path);
}
//
// プレイヤーは落とし穴の位置にいるかどうかをチェック
//
BOOL PLAYER::_is_in_trap()
{
	int x, y;
	pMaze->GetTypeFromPosition(srt.pos, x, y);
	TRAP* trap;
	if (pMaze->HasTrap(x, y, trap)) {
		D3DXVECTOR3 d = srt.pos - trap->pos;
		d.y = 0.0f;
		FLOAT r = (MAZE_BLOCK_SIZE - fSize) / 2;
		return D3DXVec3LengthSq(&d) < r * r;
	}

	return FALSE;
}
//
// プレイヤーは落とし穴の位置にいるかどうかをチェック
//
void PLAYER::AddStar(void)
{
	nStarGained += 1;
}
//
// ゲームクリア
//
void PLAYER::GameWin(void)
{
}
//
// ゲームオーバー。
//
void PLAYER::GameLose(void)
{
	pGame->GameOver(GAME_RESULT_LOSE);
}

void PLAYER::SetPosition(int x, int y)
{
	int t[4][2] = {
		{ 0 , -1 },
		{ -1 , 0 },
		{ 0 ,  1 },
		{  1 , 0 },
	};

	FLOAT rot_y = 0.0f;
	D3DXVECTOR3 dir;

	srt = SRT(pMaze->GetCenterPositionFromIndex(x, y));
	// Make player always facing to accel path.
	for (int i = 0; i < 4; i++) {
		if (pMaze->GetTypeFromIndex(x + t[i][0], y + t[i][1]) == 0) {
			dir = pMaze->GetCenterPositionFromIndex(x + t[i][0], y + t[i][1]) - pMaze->GetCenterPositionFromIndex(x, y);
			rot_y = atan2f(-dir.z, dir.x);
			break;
		}
	}
	srt.rot.y = rot_y;
	srt.pos.y = 0.0f;

	if(!pMaze->portal[0]) pMaze->portal[0] = new PORTAL(pMaze, x, y);
}
