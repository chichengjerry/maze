/***********************************************
 * [maze.h]
 * 迷路処理
 ***********************************************/
#ifndef __MAZE_H__
#define __MAZE_H__

#include "main.h"
#include "d3d.h"
#include "gamemain.h"
#include "node.h"

using namespace std;

struct MAZE2D {
	int						width;
	int						height;
	bool					generated;
	NODE*					pCell;
	bool					findingPath;

	void					_extend_wall(int x, int y, vector<CELL>& wall);
	bool					_is_current_wall(int x, int y, vector<CELL>& wall);
	void					_set_wall(int x, int y, vector<CELL>& wall);
	void					_find_neighbours(NODE* neighbours[], NODE* current);

	MAZE2D(int width, int height, unsigned int r_seed);
	~MAZE2D();

	void					MethodWallExtend();
	void					FindPath(NODE* start, NODE* goal, vector<NODE*> &path);
	BYTE					GetTypeFromIndex(int x, int y);
	NODE*					GetNode(int x, int y);
	void					GetSize(int & w, int& h);
	bool					SetCell(int x, int y, BYTE value);
};

struct MAZE : public MAZE2D {

	LPDIRECT3DVERTEXBUFFER9	pVtxBuffer;
	static LPDIRECT3DTEXTURE9
							pTexHedge;
	static LPDIRECT3DTEXTURE9
							pTexDirt;
	int						nPolygon;
	int						nPathTotal;
	HRESULT					_create_vertices();
	ITEM*					items[MAZE_STAR_NUM];
	PLAYER*					player;
	CELL					goal;
	PORTAL*					portal[2];
	TRAP*					traps[MAZE_TRAP_NUM];

	MAZE(int width, int height, UINT seed);
	~MAZE();
	HRESULT					Draw(CAMERA* camera);
	BYTE					GetTypeFromPosition(D3DXVECTOR3 pos);
	BYTE					GetTypeFromPosition(D3DXVECTOR3 pos, int &x, int& y);
	D3DXVECTOR3				GetCenterPositionFromIndex(int x, int y);
	BOOL					HasTrap(int x, int y, TRAP* &pOut);
	void					SetGoal(int x, int y);
	void					SetTraps();
	void					SetItems(PLAYER* player);
	BOOL					IsAtExit(PLAYER* player);

	void					Update(PLAYER* player);
};

#endif // !__MAZE_H__
