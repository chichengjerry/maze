#ifndef __MAZE_H__
#define __MAZE_H__

#include "main.h"
#include "d3d.h"
#include "gamemain.h"
#include "node.h"

using namespace std;

class MAZE2D {
public:
	MAZE2D(int width, int height);
	~MAZE2D();

protected:
	int						width;
	int						height;

private:
	bool					generated;
	NODE*					pCell;
	bool					findingPath;
	void					_extend_wall(int x, int y, vector<CELL>& wall);
	bool					_is_current_wall(int x, int y, vector<CELL>& wall);
	void					_set_wall(int x, int y, vector<CELL>& wall);
	void					_find_neighbours(NODE* neighbours[], NODE* current);

public:
	void					GenerateMethodExtend();
	void					FindPath(NODE* start, NODE* goal, vector<NODE*> &path);
	BYTE					GetCellType(int x, int y);
	NODE*					GetNode(int x, int y);
	void					GetSize(int & w, int& h);
	bool					SetCell(int x, int y, BYTE value);
};

class MAZE : public MAZE2D {
public:
	MAZE(int width, int height);
	~MAZE();

private:
	LPDIRECT3DVERTEXBUFFER9	pVtxBuffer;
	LPDIRECT3DINDEXBUFFER9	pVtxIdxBuffer;
	LPDIRECT3DTEXTURE9		pTexHedge;
	LPDIRECT3DTEXTURE9		pTexDirt;
	int						nPolygon;
	int						nPath;
	HRESULT					_create_vertices();

public:
	CELL					goal;
	HRESULT					Draw(CAMERA* camera);
	BYTE					GetPositionCell(D3DXVECTOR3 &pos);
	BYTE					GetPositionCell(D3DXVECTOR3 &pos, int &x, int& y);
	D3DXVECTOR3				GetCellPosition(int x, int y);
	void					SetGoal(int x, int y);
};

#endif // !__MAZE_H__
