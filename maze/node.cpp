#include "node.h"

using namespace std;

NODE::NODE()
{
	f_score = 0;
	g_score = 0;
	parent = NULL;
	type = 0;
	weight = 0;
}

bool NODE::_compare(const NODE * a, const NODE * b)
{
	return a->f_score < b->f_score;
}

int NODE::_manhattan_distance(NODE * a, NODE * b)
{
	return (abs(a->x - b->x) + abs(a->y - b->y)) * NODE_LENGTH;
}

vector<NODE*> NODE::_reconstruct_path(NODE * current)
{
	vector<NODE*> path;
	while (current) {
		path.push_back(current);
		current = current->parent;
	}
	return path;
}

int NODE::_heap_find(vector<NODE*>& heap, NODE * node)
{
	for (int i = heap.size(); i;) {
		if (heap[--i] == node) {
			return i;
		}
	}
	return -1;
}

void NODE::_heap_push(vector<NODE*> &heap, NODE * node)
{
	heap.push_back(node);
	_heapify(heap);
}

NODE* NODE::_heap_remove(vector<NODE*>& heap, const int index)
{
	if (index < 0 || index >= (int)heap.size())
		return NULL;

	// Only sort when index > 0;
	if(index > 0){
		_heap_sort(heap);
	}

	NODE* node = heap[index];
	heap.erase(heap.begin() + index);

	_heapify(heap);

	return node;
}

void NODE::_heap_sift_down(vector<NODE*>& heap, int start, int end)
{
	int root = start;
	int left, right, swap;

	while (2 * root + 1 <= end) {
		left = 2 * root + 1;
		right = 2 * root + 2;
		swap = root;

		if (_compare(heap[swap], heap[left]))
			swap = left;

		if (right <= end && _compare(heap[swap], heap[right]))
			swap = right;

		if (root == swap)
			return;

		else {
			_swap(&heap[root], &heap[swap]);
			root = swap;
		}
	}
}

void NODE::_heap_sort(vector<NODE*>& heap)
{
	int end;
	_heapify(heap);

	end = heap.size() - 1;
	while (end > 0) {
		_swap(&heap[end], &heap[0]);

		_heap_sift_down(heap, 0, --end);
	}
}

void NODE::_swap(NODE ** a, NODE ** b)
{
	NODE* temp = *a;
	*a = *b;
	*b = temp;
}

void NODE::_heapify(vector<NODE*>& heap)
{
	int start = ((int)heap.size() - 2) / 2;

	while (start >= 0) {
		_heap_sift_down(heap, start, heap.size() - 1);
		start--;
	}
}
