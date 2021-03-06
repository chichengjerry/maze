/***********************************************
 * [maze.cpp]
 * 迷路処理
 ***********************************************/
#include "maze.h"
#include "camera.h"
#include "game.h"
#include "item.h"
#include "player.h"
#include "portal.h"
#include "trap.h"

using namespace std;

//
// シャッフル関数
//
template <typename T>
void shuffle_vector(vector<T>& vec) {
	for (int i = vec.size() - 1; i > 0; i--) {
		int r = GET_RANDOM(0, i);
		T tmp = vec[i];
		vec[i] = vec[r];
		vec[r] = tmp;
	}
}

//
// 初期化
//
MAZE2D::MAZE2D(int w, int h, unsigned int r_seed)
{
	width = w;
	height = h;
	generated = false;
	findingPath = false;
	
	if (r_seed == 0) r_seed = timeGetTime();
	srand(r_seed);

	pCell = new NODE[width * height];
}
//
// 解放
//
MAZE2D::~MAZE2D()
{
	delete[] pCell;
}
//
// 壁を延長する関数。
//
void MAZE2D::_extend_wall(int x, int y, vector<CELL>& wall)
{
	vector<int> directions;

	// 延伸できる方向をチェックする
	if (GetTypeFromIndex(x, y - 1) == 0 && !_is_current_wall(x, y - 2, wall)) // 上
		directions.push_back(0);
	if (GetTypeFromIndex(x + 1, y) == 0 && !_is_current_wall(x + 2, y, wall)) // 右
		directions.push_back(1);
	if (GetTypeFromIndex(x, y + 1) == 0 && !_is_current_wall(x, y + 2, wall)) // 下
		directions.push_back(2);
	if (GetTypeFromIndex(x - 1, y) == 0 && !_is_current_wall(x - 2, y, wall)) // 左
		directions.push_back(3);

	if (!directions.empty()) {
		// 壁を設置する
		_set_wall(x, y, wall);

		// 延伸方向をランダムで決める
		bool is_path = false;
		shuffle_vector(directions);

		switch (directions[0]) {
		case 0: // 上
			is_path = GetTypeFromIndex(x, y - 2) == 0;
			_set_wall(x, y - 1, wall);
			_set_wall(x, y - 2, wall);
			y -= 2;
			break;
		case 1: // 右
			is_path = GetTypeFromIndex(x + 2, y) == 0;
			_set_wall(x + 1, y, wall);
			_set_wall(x + 2, y, wall);
			x += 2;
			break;
		case 2: // 下
			is_path = GetTypeFromIndex(x, y + 2) == 0;
			_set_wall(x, y + 1, wall);
			_set_wall(x, y + 2, wall);
			y += 2;
			break;
		case 3: // 左
			is_path = GetTypeFromIndex(x - 2, y) == 0;
			_set_wall(x - 1, y, wall);
			_set_wall(x - 2, y, wall);
			x -= 2;
			break;
		}
		if (is_path) // 延伸できる
			_extend_wall(x, y, wall);

		// 延伸できる区間がない
	}
	else {
		CELL before = wall.back(); wall.pop_back();
		_extend_wall(before.x, before.y, wall);
	}
}
//
// 現在の場所は今延伸している壁かどうかをチェックする。
//
bool MAZE2D::_is_current_wall(int x, int y, vector<CELL>& wall)
{
	int length = wall.size();

	for (int i = 0; i < length; i++)
		if (x == wall[i].x && y == wall[i].y)
			return true;

	return false;
}
//
// 現在の場所に壁を設置する。
//
void MAZE2D::_set_wall(int x, int y, vector<CELL>& wall)
{
	SetCell(x, y, 1);
	if (!(x % 2) && !(y % 2))
		wall.push_back({ x, y });
}
//
// 壁伸ばし方法で迷路生成。
//
void MAZE2D::MethodWallExtend()
{
	if (generated) return;

	vector<CELL> startPoint, currentWall;

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			SetCell(x, y, 0);

			if (!(x % 2) && !(y % 2)) {
				startPoint.push_back({ x, y });
			}

			if (x == 0 || y == 0 || x == width - 1 || y == height - 1) {
				SetCell(x, y, 1);
			}
		}
	}

	shuffle_vector(startPoint);

	while (!startPoint.empty()) {
		CELL p = startPoint.back();	startPoint.pop_back();
		int x = p.x, y = p.y;
		if (GetTypeFromIndex(x, y) == 0) {
			currentWall.clear();
			_extend_wall(x, y, currentWall);
		}
	}

	generated = true;
}
//
// 隣のノードを探す。
//
void MAZE2D::_find_neighbours(NODE * neighbours[], NODE * current)
{
	// Check all available node nodes
	int cx = current->x;
	int cy = current->y;

	int offset[4][2] = {
		{ 0, -1 },
		{ 1, 0 },
		{ 0, 1 },
		{ -1, 0 },
	};

	for (int i = 0; i < 4; i++) {
		neighbours[i] = GetNode(cx + offset[i][0], cy + offset[i][1]);
		if (neighbours[i] && neighbours[i]->weight == -1)
			neighbours[i] = NULL;
	}
}
//
// ノードのポインタを取得する。
//
NODE * MAZE2D::GetNode(int x, int y)
{
	if (x < 0 || y < 0 || x >= width || y >= height) return NULL;

	return pCell + y * height + x;
}
//
// A* パスファインダーアルゴリズム
//
void MAZE2D::FindPath(NODE* start, NODE* goal, vector<NODE*> &path)
{
	static vector<NODE*> closedset;
	static vector<NODE*> openset;
	static NODE* neighbours[4];
	static NODE* current;

	if (!findingPath) { // パスファインドスタート
		findingPath = true;

		openset.clear();
		closedset.clear(); 
		current = start;

		NODE::_heap_push(openset, current);

		current->parent = NULL;
		current->g_score = 0;
		// マンハッタン距離を啓発関数に使う
		current->f_score = NODE::_manhattan_distance(start, goal);
	}

	// シングルスレッドでのインプリメンテーション
	// 一回の更新にすべてを計算するのは無理なので、代わりに一部だけを計算する
	int cnt = 16; // 計算のループ回数
	do {
		if (!openset.empty()) {
			// 最低コストのノードを取得する。
			current = NODE::_heap_remove(openset, 0);

			if (current == goal) {
				// 終点が見つかった
				path = NODE::_reconstruct_path(current);

				findingPath = false;
				return;
			}

			NODE::_heap_push(closedset, current);

			_find_neighbours(neighbours, current);

			for (int i = 0; i < sizeof(neighbours) / sizeof(NODE*); i++) {
				NODE* node = neighbours[i];
				if (node) {
					if (0 <= NODE::_heap_find(closedset, node))
						continue;

					unsigned g_score = current->g_score + NODE_LENGTH + node->weight;

					if (0 > NODE::_heap_find(openset, node)) { // 新たなところ
						NODE::_heap_push(openset, node);
					}
					else if (g_score > node->g_score)
						continue;

					// 新しいパスが見つかった
					node->parent = current;
					node->g_score = g_score;
					node->f_score = g_score + NODE::_manhattan_distance(node, goal);
				}
			}
		}
		else {
			findingPath = false;
		}
	} while (--cnt);

	return;
}

BYTE MAZE2D::GetTypeFromIndex(int x, int y)
{
	NODE* node = GetNode(x, y);

	if (!node) return -1;
	return node->type;
}

void MAZE2D::GetSize(int & w, int & h)
{
	w = width;
	h = height;
}

bool MAZE2D::SetCell(int x, int y, BYTE value)
{
	if (x < 0 || y < 0 || x >= width || y >= height) return false;
	int k = y * height + x;
	pCell[k].x = x;
	pCell[k].y = y;
	pCell[k].type = value;
	if(value > 0)
		pCell[k].weight = -1;
	return true;
}

LPDIRECT3DTEXTURE9 MAZE::pTexDirt = NULL;
LPDIRECT3DTEXTURE9 MAZE::pTexHedge = NULL;

MAZE::MAZE(int width, int height, UINT seed) : MAZE2D(width, height, seed)
{
	ZeroMemory(&portal, sizeof(portal));
	ZeroMemory(&items, sizeof(items));
	ZeroMemory(&traps, sizeof(traps));

	D3D::LoadTexture(&pTexHedge, _T("data/TEXTURE/hedge.jpg"));
	D3D::LoadTexture(&pTexDirt, _T("data/TEXTURE/dirt.jpg"));

	MethodWallExtend();
	_create_vertices();
}

MAZE::~MAZE()
{
	SAFE_RELEASE(pVtxBuffer);
	SAFE_RELEASE(pTexHedge);
	SAFE_RELEASE(pTexDirt);

	for (int i = 0; i < MAZE_STAR_NUM; i++) {
		SAFE_DELETE(items[i]);
	}

	for (int i = 0; i < 2; i++) {
		SAFE_DELETE(portal[i]);
	}

	for (int i = 0; i < MAZE_TRAP_NUM; i++) {
		SAFE_DELETE(traps[i]);
	}
}

HRESULT MAZE::Draw(CAMERA* pCamera)
{
	HRESULT hr;
	LPDIRECT3DDEVICE9 pDevice = D3D::GetDevice();

	D3DXMATRIX mtx;
	D3DXMatrixIdentity(&mtx);

	D3DCHECK(pDevice->SetTransform(D3DTS_WORLD, &mtx));
	D3DCHECK(pDevice->SetStreamSource(0, pVtxBuffer, 0, sizeof(VERTEX_3D)));
	D3DCHECK(pDevice->SetFVF(FVF_VERTEX_3D));

	for (int i = 0; i < nPolygon; i++) {
		if (i < nPathTotal) {
			// パスの描画
			D3DCHECK(pDevice->SetTexture(0, pTexDirt));
		}
		else {
			// 生垣の描画
			D3DCHECK(pDevice->SetTexture(0, pTexHedge));
		}

		D3DCHECK(pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, i * NUM_VERTEX, NUM_POLYGON));
	}

	for (int i = 0; i < MAZE_STAR_NUM; i++) {
		if (items[i]) items[i]->Draw(pCamera);
	}

	for (int i = 0; i < 2; i++) {
		if (portal[i]) portal[i]->Draw(pCamera);
	}

	for (int i = 0; i < MAZE_TRAP_NUM; i++) {
		if (traps[i]) traps[i]->Draw();
	}

	return D3D_OK;
}

void MAZE::Update(PLAYER * player)
{
	for (int i = 0; i < MAZE_STAR_NUM; i++) {
		if (items[i]) items[i]->Update(player);
	}

	if (IsAtExit(player)) {
		player->pGame->GameOver(GAME_RESULT_WIN);
	}
	for (int i = 0; i < 2; i++) {
		if (portal[i]) portal[i]->Update();
	}
	for (int i = 0; i < MAZE_TRAP_NUM; i++) {
		if (traps[i]) traps[i]->Update();
	}
}
//
// 迷路のポリゴンを生成する。
//
HRESULT MAZE::_create_vertices()
{
	HRESULT hr;
	LPDIRECT3DDEVICE9 pDevice = D3D::GetDevice();

	vector<D3DXVECTOR3> vertices;	// 頂点リスト
	vector<DWORD> indicies;			// インデックスリスト
	int vw = width + 1, vh = height + 1, offset = vh * vw;
	
	nPathTotal = 0;
	for (int y = 0; y < vh; y++)
		for (int x = 0; x < vw; x++)
			vertices.push_back({ (x - vw / 2) * MAZE_BLOCK_SIZE, 0.0f, (y - vh / 2) * MAZE_BLOCK_SIZE });
	for (int y = 0; y < vh; y++)
		for (int x = 0; x < vw; x++)
			vertices.push_back({ (x - vw / 2) * MAZE_BLOCK_SIZE, MAZE_BLOCK_HEIGHT, (y - vh / 2) * MAZE_BLOCK_SIZE });

	// インデックスリストを作る。
	// まずパスを作る
	for (int y = 0; y < vh; y++) {
		for (int x = 0; x < vw; x++) {
			DWORD a, b, c, d;
			if (GetTypeFromIndex(x, y) == 0) {
				a = y * vh + x;
				b = (y + 1) * vh + x;
				c = y * vh + (x + 1);
				d = (y + 1) * vh + (x + 1);
				indicies.push_back(a); indicies.push_back(b); indicies.push_back(c); indicies.push_back(d);
				nPathTotal += 1;
			}
		}
	}
	// 壁の天頂を作る
	for (int y = 0; y < vh; y++) {
		for (int x = 0; x < vw; x++) {
			DWORD a, b, c, d;
			if (GetTypeFromIndex(x, y) == 1) {
				a = offset + y * vh + x;
				b = offset + (y + 1) * vh + x;
				c = offset + y * vh + (x + 1);
				d = offset + (y + 1) * vh + (x + 1);
				indicies.push_back(a); indicies.push_back(b); indicies.push_back(c); indicies.push_back(d);
			}
		}
	}

	// 壁を作る
	for (int y = 0; y < vh; y++) {
		for (int x = 0; x < vw; x++) {
			if (GetTypeFromIndex(x, y) == 0) {
				DWORD a, b, c, d;
	
				if (GetTypeFromIndex(x, y - 1) == 1) { // 南
					c = y * vh + x + 1;
					d = y * vh + x;
					a = c + offset;
					b = d + offset;
					indicies.push_back(a); indicies.push_back(b); indicies.push_back(c); indicies.push_back(d);
				}
	
				if (GetTypeFromIndex(x + 1, y) == 1) { // 東
					c = (y + 1) * vh + x + 1;
					d = y * vh + x + 1;
					a = c + offset;
					b = d + offset;
					indicies.push_back(a); indicies.push_back(b); indicies.push_back(c); indicies.push_back(d);
				}
	
				if (GetTypeFromIndex(x, y + 1) == 1) { // 北
					c = (y + 1) * vh + x;
					d = (y + 1) * vh + x + 1;
					a = c + offset;
					b = d + offset;
					indicies.push_back(a); indicies.push_back(b); indicies.push_back(c); indicies.push_back(d);
				}
	
				if (GetTypeFromIndex(x - 1, y) == 1) { // 西
					c = y * vh + x;
					d = (y + 1) * vh + x;
					a = c + offset;
					b = d + offset;
					indicies.push_back(a); indicies.push_back(b); indicies.push_back(c); indicies.push_back(d);
				}
			}
		}
	}
	
	nPolygon = indicies.size() / 4;

	VERTEX_3D* pVtx;
	D3DCHECK(pDevice->CreateVertexBuffer(sizeof(VERTEX_3D) * NUM_VERTEX * nPolygon, D3DUSAGE_WRITEONLY, FVF_VERTEX_3D, D3DPOOL_MANAGED, &pVtxBuffer, NULL));
	D3DCHECK(pVtxBuffer->Lock(0, 0, (void**)&pVtx, 0));
	for (int i = 0; i < nPolygon; i++, pVtx += NUM_VERTEX) {
		pVtx[0].vtx = vertices[indicies[i * NUM_VERTEX + 0]];
		pVtx[1].vtx = vertices[indicies[i * NUM_VERTEX + 1]];
		pVtx[2].vtx = vertices[indicies[i * NUM_VERTEX + 2]];
		pVtx[3].vtx = vertices[indicies[i * NUM_VERTEX + 3]];

		pVtx[0].nor =
		pVtx[1].nor =
		pVtx[2].nor =
		pVtx[3].nor = D3DXVECTOR3(0.0f, 1.0f, 0.0f);

		pVtx[0].dif =
		pVtx[1].dif =
		pVtx[2].dif =
		pVtx[3].dif = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

		pVtx[0].tex = D3DXVECTOR2(0.0f, 0.0f);
		pVtx[1].tex = D3DXVECTOR2(0.0f, 1.0f);
		pVtx[2].tex = D3DXVECTOR2(1.0f, 0.0f);
		pVtx[3].tex = D3DXVECTOR2(1.0f, 1.0f);
	}
	D3DCHECK(pVtxBuffer->Unlock());

	return D3D_OK;
}

//
// 現在位置のブロック属性を取得する。
//
BYTE MAZE::GetTypeFromPosition(D3DXVECTOR3 pos)
{
	int x, y;

	return GetTypeFromPosition(pos, x, y);
}
//
// 現在位置のブロックの座標系且つ属性を取得する。
//
BYTE MAZE::GetTypeFromPosition(D3DXVECTOR3 pos, int & x, int & y)
{
	x = (int)floorf(pos.x / MAZE_BLOCK_SIZE) + (width + 1) / 2;
	y = (int)floorf(pos.z / MAZE_BLOCK_SIZE) + (height + 1) / 2;

	return GetTypeFromIndex(x, y);
}
//
// 現在位置インデックスからブロックの中心点を取得する。
//
D3DXVECTOR3 MAZE::GetCenterPositionFromIndex(int x, int y)
{
	FLOAT fx = MAZE_BLOCK_SIZE * ((x - (width + 1) / 2) + 0.5f);
	FLOAT fz = MAZE_BLOCK_SIZE * ((y - (height + 1) / 2) + 0.5f);

	return D3DXVECTOR3(fx, MAZE_BLOCK_HEIGHT * 0.5f, fz);
}
//
// 落とし穴があるかどうかをチェックする。
//
BOOL MAZE::HasTrap(int x, int y, TRAP* &pOut)
{
	for (int i = 0; i < MAZE_TRAP_NUM; ++i) {
		if (traps[i] && traps[i]->tx == x && traps[i]->ty == y) {
			pOut = traps[i];
			return TRUE;
		}
	}
	return FALSE;
}
//
// 終点を設置する。
//
void MAZE::SetGoal(int x, int y)
{
	goal = { x, y };
	portal[1] = new PORTAL(this, x, y);
}
//
// 落とし穴を設置する。
//
void MAZE::SetTraps()
{
	int i = 0;
	while (i < MAZE_TRAP_NUM) {
		int x, y;
		x = 2 * GET_RANDOM(1, MAZE_SIZE - 3) + 1;
		y = 2 * GET_RANDOM(1, MAZE_SIZE - 3) + 1;
		TRAP* trap;
		if (!HasTrap(x, y, trap)) {
			traps[i++] = new TRAP(this, x, y);
		}
	}
}
//
// 最短ルートにスターを配置する。
//
void MAZE::SetItems(PLAYER * player)
{
	if (!player->guideVtx.empty()) {
		int total = player->guideVtx.size();

		for (int i = 0; i < MAZE_STAR_NUM; i++) {
			int x, y;
			GetTypeFromPosition(player->guideVtx[total * (i + 1) / (MAZE_STAR_NUM + 2)], x, y);
			items[i] = new ITEM(this, x, y);
		}
	}
}
//
// 出口に到達したかをチェックする。
//
BOOL MAZE::IsAtExit(PLAYER * player)
{
	D3DXVECTOR3 d = player->srt.pos - GetCenterPositionFromIndex(goal.x, goal.y);
	d.y = 0.0f;
	return D3DXVec3LengthSq(&d) < MAZE_BLOCK_SIZE * MAZE_BLOCK_SIZE * 0.25f;
}
