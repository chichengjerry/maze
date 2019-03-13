#include "player.h"
#include "camera.h"
#include "game.h"
#include "input.h"
#include "item.h"
#include "maze.h"
#include "portal.h"
#include "sound.h"
#include "star.h"

#define PLAYER_LIGHT_ID		0x10

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

BYTE KEYBOARDMAP[ACTION_COUNT] = { 
	DIK_LEFT, DIK_RIGHT, DIK_UP, DIK_DOWN,
	DIK_W, DIK_S, DIK_A, DIK_D,
	DIK_SPACE, DIK_E
};
DWORD GAMEPADMAP[ACTION_COUNT] = {
	BUTTON_ROTATE_LEFT, BUTTON_ROTATE_RIGHT, BUTTON_ROTATE_UP, BUTTON_ROTATE_DOWN,
	BUTTON_MOVE_UP, BUTTON_MOVE_DOWN, BUTTON_MOVE_LEFT, BUTTON_MOVE_RIGHT,
	BUTTON_RT, BUTTON_LB
};

PLAYER::PLAYER(MAINGAME* game, int nId, int nTotal)
{
	pGame = game;
	pMaze = game->maze;
	id = nId;
	bDoOnceFlag = TRUE;
	bJumping = FALSE;
	bMoveable = FALSE;
	fSize = PLAYER_SIZE;
	fJumpSpeed = 0.0f;
	fMoveSpeed = MOVE_MIN_SPEED;
	nStarGained = 0;
	fStamina = fStaminaMax = FLT_MAX;
	fTimeElapsed = 0.0f;

	RECT rect = { (LONG)(CL_WIDTH  * id / nTotal), 0, (LONG)(CL_WIDTH * (id + 1) / nTotal), CL_HEIGHT };
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

PLAYER::~PLAYER()
{
	SAFE_DELETE(camera);
	SAFE_DELETE(guideStar);
#if _DEBUG
	SAFE_RELEASE(pGuideLine);
#endif // _DEBUG
}

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

void PLAYER::Update()
{
	if (bMoveable) {
		fTimeElapsed += D3D::fAnimationRate;
	}
	if (bMoveable && bDoOnceFlag) {
		bDoOnceFlag = FALSE;

		pGame->maze->SetItems(this);
		fStamina = fStaminaMax = (FLOAT)guideVtx.size();
	}
	_update_consume();
	_update_movement();
	_update_jumping();
	_update_path();

	guideStar->Update();
	camera->Update();
}

inline D3DXVECTOR3 PLAYER::_get_desired_direction(D3DXVECTOR3 input, D3DXVECTOR3 nor)
{
	const static FLOAT threshold = cosf(D3DXToRadian(30.0f));

	FLOAT dot = D3DXVec3Dot(&input, &nor);

	if (dot < 0.0f)
	{
		// not running into the wall
		return input;
	}
	else if (dot < threshold)
	{
		D3DXVECTOR3 intoWall = nor * dot;
		D3DXVECTOR3 alongWall = input - intoWall;
		return alongWall;
	}

	// run "straight into the wall"
	return D3DXVECTOR3(0.0f, 0.0f, 0.0f);
}

inline BOOL PLAYER::_collision_check(D3DXVECTOR3 pos, D3DXVECTOR3& dir)
{
	BOOL hit = FALSE;
	D3DXVECTOR3 newPos = pos + dir;
	int x, z;
	FLOAT cx = newPos.x, cz = newPos.z;
	// Get player block
	pGame->maze->GetPositionCell(newPos, x, z);

	for (int i = -1; i <= 1; ++i) {
		for (int j = -1; j <= 1; ++j) {
			if (!(i || j)) continue;	// Skip the block player stands

			if (pGame->maze->GetCellType(x + j, z + i)) {	// Checked block is wall

				D3DXVECTOR3 blk = pGame->maze->GetCellPosition(x + j, z + i);
				FLOAT bx = blk.x - MAZE_BLOCK_SIZE * 0.5f;
				FLOAT bz = blk.z - MAZE_BLOCK_SIZE * 0.5f;
				FLOAT dx = cx - max(bx, min(cx, bx + MAZE_BLOCK_SIZE));
				FLOAT dz = cz - max(bz, min(cz, bz + MAZE_BLOCK_SIZE));

				// intersect circle against rectangle
				if (dx * dx + dz * dz < fSize * fSize) {
					hit = TRUE;
					if ((!pGame->maze->GetCellType(x + j, z) && !pGame->maze->GetCellType(x, z + i)) || // hit corner block
						abs(i) != abs(j) // hit wall block
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

inline void PLAYER::_update_jumping(void)
{
	const FLOAT accel = PLAYER_GRAVITY * D3D::fAnimationRate;

	if (!bMoveable || pGame->bGameOver) {
		return;
	}
	if (bJumping) {
		fJumpSpeed -= accel;
		srt.pos.y += fJumpSpeed;
		if (srt.pos.y < MAZE_BLOCK_HEIGHT * 0.5f) {
			srt.pos.y = MAZE_BLOCK_HEIGHT * 0.5f;
			bJumping = FALSE;
		}
	}
	else if (DINPUT::KeyTriggered(KEYBOARDMAP[ACTION_JUMP]) || DINPUT::ButtonTriggered(GAMEPADMAP[ACTION_JUMP])) {
		bJumping = TRUE;
		fJumpSpeed = PLAYER_JUMP_SPEED;
		DSOUND::Play(SOUND_MOVEMENT);
	}
}

inline void PLAYER::_update_line(vector<NODE*> path)
{
	int nGuideLength = path.size();
	bMoveable = TRUE;
	guideVtx.clear();
	for (int i = 0; i < nGuideLength; i++) {
		guideVtx.push_back(pMaze->GetCellPosition(path[i]->x, path[i]->y));
		guideVtx[i].y = 10.0f;
	}
}

inline void PLAYER::_update_movement()
{
	if (!bMoveable || pGame->bGameOver) {
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

	// 前後移動
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

	// do collision check and change the actual direction of player's movement
	_collision_check(srt.pos, dir);

	dir *= fMoveSpeed * rate;

	srt.pos += dir;

	if (fabsf(dir.x) > 0.001f || fabsf(dir.z) > 0.001f) {
		moveSoundTime += rate;
		if (!bJumping && moveSoundTime > 0.75f) {
			DSOUND::Play(SOUND_MOVEMENT);
			moveSoundTime -= 0.75f;
		}
		fStamina -= 1.0f * D3D::fAnimationRate;
		if (fStamina <= 0.0f) GameLose();
	}
	else {
		fMoveSpeed = 0.0f;
		moveSoundTime = 0.0f;
	}
}

inline void PLAYER::_update_path(void)
{
	int x, y;
	int w, h;
	pMaze->GetSize(w, h);
	pMaze->GetPositionCell(srt.pos, x, y);
	NODE* start = pMaze->GetNode(x, y);
	NODE* goal = pMaze->GetNode(pMaze->goal.x, pMaze->goal.y);

	pMaze->FindPath(start, goal, path);
	if (!path.empty())
		_update_line(path);
}

void PLAYER::AddStar(void)
{
	nStarGained += 1;
}

void PLAYER::GameWin(void)
{
	pGame->GameOver(0, GAME_RESULT_WIN);
}

void PLAYER::GameLose(void)
{
	pGame->GameOver(0, GAME_RESULT_LOSE);
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

	srt = SRT(pMaze->GetCellPosition(x, y));
	// Make player always facing to accel path.
	for (int i = 0; i < 4; i++) {
		if (pMaze->GetCellType(x + t[i][0], y + t[i][1]) == 0) {
			dir = pMaze->GetCellPosition(x + t[i][0], y + t[i][1]) - pMaze->GetCellPosition(x, y);
			rot_y = atan2f(-dir.z, dir.x);
			break;
		}
	}
	srt.rot.y = rot_y;

	pMaze->portal[0] = new PORTAL(pMaze, x, y);
}
