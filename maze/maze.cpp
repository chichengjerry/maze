#include "maze.h"
#include "camera.h"

using namespace std;

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
// Constructor
//
MAZE2D::MAZE2D(int w, int h)
{
	width = w;
	height = h;
	generated = false;
	findingPath = false;

	pCell = new NODE[width * height];
}

MAZE2D::~MAZE2D()
{
	delete[] pCell;
}

void MAZE2D::_extend_wall(int x, int y, vector<CELL>& wall)
{
	vector<int> directions;

	// Up
	if (GetCellType(x, y - 1) == 0 && !_is_current_wall(x, y - 2, wall)) directions.push_back(0);
	// Right
	if (GetCellType(x + 1, y) == 0 && !_is_current_wall(x + 2, y, wall)) directions.push_back(1);
	// Bottom
	if (GetCellType(x, y + 1) == 0 && !_is_current_wall(x, y + 2, wall)) directions.push_back(2);
	// Left
	if (GetCellType(x - 1, y) == 0 && !_is_current_wall(x - 2, y, wall)) directions.push_back(3);

	if (!directions.empty()) {
		_set_wall(x, y, wall);

		bool is_path = false;
		shuffle_vector(directions);
		switch (directions[0]) {
		case 0: // Up
			is_path = GetCellType(x, y - 2) == 0;
			_set_wall(x, y - 1, wall);
			_set_wall(x, y - 2, wall);
			y -= 2;
			break;
		case 1: // Right
			is_path = GetCellType(x + 2, y) == 0;
			_set_wall(x + 1, y, wall);
			_set_wall(x + 2, y, wall);
			x += 2;
			break;
		case 2: // Bottom
			is_path = GetCellType(x, y + 2) == 0;
			_set_wall(x, y + 1, wall);
			_set_wall(x, y + 2, wall);
			y += 2;
			break;
		case 3: // Left
			is_path = GetCellType(x - 2, y) == 0;
			_set_wall(x - 1, y, wall);
			_set_wall(x - 2, y, wall);
			x -= 2;
			break;
		}
		if (is_path)
			_extend_wall(x, y, wall);

		// no available space for extending the wall
	}
	else {
		CELL before = wall.back(); wall.pop_back();
		_extend_wall(before.x, before.y, wall);
	}
}

NODE * MAZE2D::GetNode(int x, int y)
{
	if (x < 0 || y < 0 || x >= width || y >= height) return NULL;

	return pCell + y * height + x;
}

bool MAZE2D::_is_current_wall(int x, int y, vector<CELL>& wall)
{
	int length = wall.size();

	for (int i = 0; i < length; i++)
		if (x == wall[i].x && y == wall[i].y)
			return true;

	return false;
}

void MAZE2D::_set_wall(int x, int y, vector<CELL>& wall)
{
	SetCell(x, y, 1);
	if (!(x % 2) && !(y % 2))
		wall.push_back({ x, y });
}

void MAZE2D::GenerateMethodExtend()
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
		if (GetCellType(x, y) == 0) {
			currentWall.clear();
			_extend_wall(x, y, currentWall);
		}
	}

	generated = true;
}

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

void MAZE2D::FindPath(NODE* start, NODE* goal, vector<NODE*> &path)
{
	static vector<NODE*> closedset;
	static vector<NODE*> openset;
	static NODE* neighbours[4];
	static NODE* current;

	if (!findingPath) {
		findingPath = true;

		openset.clear();
		closedset.clear(); 
		current = start;

		// Add start point to open set
		NODE::_heap_push(openset, current);


		current->parent = NULL;
		current->g_score = 0;
		// Use manhattan distance
		current->f_score = NODE::_manhattan_distance(start, goal);
	}
	int cnt = 10;

	do {
		if (!openset.empty()) {
			// Get the node with lowest cost(weight)
			current = NODE::_heap_remove(openset, 0);

			// Is Goal
			if (current == goal) {
				// Reconstruct path
				path = NODE::_reconstruct_path(current);

				findingPath = false;
				return;
			}

			// Add current node to closed set
			NODE::_heap_push(closedset, current);

			_find_neighbours(neighbours, current);

			for (int i = 0; i < 4; i++) {
				NODE* node = neighbours[i];
				if (node) {
					if (0 <= NODE::_heap_find(closedset, node))
						continue;

					unsigned g_score = current->g_score + NODE_LENGTH + node->weight;

					if (0 > NODE::_heap_find(openset, node)) {
						NODE::_heap_push(openset, node);
					}
					else if (g_score > node->g_score)
						continue;

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

BYTE MAZE2D::GetCellType(int x, int y)
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

MAZE::MAZE(int width, int height) : MAZE2D(width, height)
{
	this->goal = goal;
	GenerateMethodExtend();

	D3D::LoadTexture(&pTexDirt, _T("data/TEXTURE/dirt.jpg"));
	D3D::LoadTexture(&pTexHedge, _T("data/TEXTURE/hedge.jpg"));

	_create_vertices();
}

MAZE::~MAZE()
{
	SAFE_RELEASE(pVtxBuffer);
	SAFE_RELEASE(pTexHedge);
	SAFE_RELEASE(pTexDirt);
}

HRESULT MAZE::_create_vertices()
{
	HRESULT hr;
	LPDIRECT3DDEVICE9 pDevice = D3D::GetDevice();

	//vertex list.
	vector<D3DXVECTOR3> vertices;

	int vw = width + 1, vh = height + 1, offset = vh * vw;
	for (int y = 0; y < vh; y++)
		for (int x = 0; x < vw; x++)
			vertices.push_back({ (x - vw / 2) * MAZE_BLOCK_SIZE, 0.0f, (y - vh / 2) * MAZE_BLOCK_SIZE });
	for (int y = 0; y < vh; y++)
		for (int x = 0; x < vw; x++)
			vertices.push_back({ (x - vw / 2) * MAZE_BLOCK_SIZE, MAZE_BLOCK_HEIGHT, (y - vh / 2) * MAZE_BLOCK_SIZE });

	// index list.
	vector<DWORD> indicies;

	int nTop = 0;

	// Generate path index list.
	for (int y = 0; y < vh; y++) {
		for (int x = 0; x < vw; x++) {
			DWORD a, b, c, d;
			if (GetCellType(x, y) == 0) {
				// add path
				a = y * vh + x;
				b = (y + 1) * vh + x;
				c = y * vh + (x + 1);
				d = (y + 1) * vh + (x + 1);
				indicies.push_back(a); indicies.push_back(b); indicies.push_back(c); indicies.push_back(d);
				nPath += 1;
			}
		}
	}
		
	for (int y = 0; y < vh; y++) {
		for (int x = 0; x < vw; x++) {
			DWORD a, b, c, d;
			if (GetCellType(x, y) == 1) {
				// add path
				a = offset + y * vh + x;
				b = offset + (y + 1) * vh + x;
				c = offset + y * vh + (x + 1);
				d = offset + (y + 1) * vh + (x + 1);
				indicies.push_back(a); indicies.push_back(b); indicies.push_back(c); indicies.push_back(d);
			}
		}
	}

	// Generate wall index list.
	for (int y = 0; y < vh; y++) {
		for (int x = 0; x < vw; x++) {
			if (GetCellType(x, y) == 0) {
				DWORD a, b, c, d;
	
				if (GetCellType(x, y - 1) == 1) {
					// add north wall
					c = y * vh + x + 1;
					d = y * vh + x;
					a = c + offset;
					b = d + offset;
					indicies.push_back(a); indicies.push_back(b); indicies.push_back(c); indicies.push_back(d);
				}
	
				if (GetCellType(x + 1, y) == 1) {
					// add east wall 
					c = (y + 1) * vh + x + 1;
					d = y * vh + x + 1;
					a = c + offset;
					b = d + offset;
					indicies.push_back(a); indicies.push_back(b); indicies.push_back(c); indicies.push_back(d);
				}
	
				if (GetCellType(x, y + 1) == 1) {
					// add south wall 
					c = (y + 1) * vh + x;
					d = (y + 1) * vh + x + 1;
					a = c + offset;
					b = d + offset;
					indicies.push_back(a); indicies.push_back(b); indicies.push_back(c); indicies.push_back(d);
				}
	
				if (GetCellType(x - 1, y) == 1) {
					// add west wall 
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

		D3DXVECTOR3 nor = i < (vh * vw) ? D3DXVECTOR3(0.0f, 1.0f, 0.0f) : D3DXVECTOR3(0.0f, 0.0f, -1.0f);
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
		if (i < nPath) {
			D3DCHECK(pDevice->SetTexture(0, pTexDirt));
		}
		else {
			D3DCHECK(pDevice->SetTexture(0, pTexHedge));
		}
	
		D3DCHECK(pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, i * NUM_VERTEX, NUM_POLYGON));
	}

	return D3D_OK;
}

BYTE MAZE::GetPositionCell(D3DXVECTOR3 & pos)
{
	int x, y;

	return GetPositionCell(pos, x, y);
}

BYTE MAZE::GetPositionCell(D3DXVECTOR3 & pos, int & x, int & y)
{
	x = (int)floorf(pos.x / MAZE_BLOCK_SIZE) + (width + 1) / 2;
	y = (int)floorf(pos.z / MAZE_BLOCK_SIZE) + (height + 1) / 2;

	return GetCellType(x, y);
}

D3DXVECTOR3 MAZE::GetCellPosition(int x, int y)
{
	FLOAT fx = MAZE_BLOCK_SIZE * ((x - (width + 1) / 2) + 0.5f);
	FLOAT fz = MAZE_BLOCK_SIZE * ((y - (height + 1) / 2) + 0.5f);
	return D3DXVECTOR3(fx, MAZE_BLOCK_HEIGHT * 0.5f, fz);
}

void MAZE::SetGoal(int x, int y)
{
	goal = { x, y };
}
