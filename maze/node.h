#ifndef __NODE_H__
#define __NODE_H__

#include "main.h"
#include "core.h"
#include "d3d.h"

using namespace std;

#define NODE_LENGTH				10
#define NODE_LENGTH_DIAGONAL	14

struct CELL {
	int x;
	int y;
};

struct NODE : public CELL {
	unsigned				f_score;
	unsigned				g_score;
	NODE*					parent;
	BYTE					type;
	int						weight;

	NODE();

	static bool				_compare(const NODE* a, const NODE* b);
	static void				_swap(NODE** a, NODE** b);
	static int				_heap_find(vector<NODE*> &heap, NODE* node);
	static void				_heap_push(vector<NODE*> &heap, NODE* node);
	static NODE*			_heap_remove(vector<NODE*> &heap, const int index);
	static void				_heap_sift_down(vector<NODE*> &heap, int start, int end);
	static void				_heap_sort(vector<NODE*> &heap);
	static void				_heapify(vector<NODE*> &heap);
	static int				_manhattan_distance(NODE* a, NODE* b);
	static vector<NODE*>	_reconstruct_path(NODE* current);
};



#endif // !__NODE_H__
