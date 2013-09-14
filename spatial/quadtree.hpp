#ifndef __FBE_QUADTREE_HPP__
#define __FBE_QUADTREE_HPP__

#include "memory/arena_allocator.hpp"
#include "geometry/intersections.hpp"
#include "spatial_index_interfaces.hpp"

class VoidQuadtreeDebugDrawer;

template<class T, class DebugDrawer = VoidQuadtreeDebugDrawer>
class Quadtree : ISpatialIndex2D<T>
{

public:
	Quadtree(float zeroLevelSize, size_t depth, size_t maxMemory):
		_zeroLevelSize(zeroLevelSize),
		_depth(depth)
	{
		_arena = new ArenaAllocator(maxMemory);
	}


	virtual ~Quadtree()
	{
		delete _arena;
	}
	

	virtual void addBoundingCircle(vec2& center, float radius, uint32_t mask, T* entity)
	{

	}


	virtual void purge()
	{

	}


	virtual float raycast(vec2& origin, vec2& end, uint32_t mask, T** entity)
	{
		return 0;
	}


	virtual bool getNeighbours(vec2& point, float distance, uint32_t mask, T** result, int maxResultLength)
	{
		return false;
	}

private:
	struct BoundingCircle
	{
		BoundingCircle() : next(nullptr), entity(nullptr) {};
		vec2 center;
		float radius;
		uint32_t mask;
		T* entity;
		BoundingCircle* next;
	};

	struct Node
	{
		Node() : topLeft(nullptr), topRight(nullptr), botLeft(nullptr), botRight(nullptr), inhabitants(nullptr) {};
		Node* topLeft;
		Node* topRight;
		Node* botLeft;
		Node* botRight;
		BoundingCircle* inhabitants;
	};


private:
	float _zeroLevelSize;
	int _depth;
	Node _root;
	ArenaAllocator* _arena;
};


class VoidQuadtreeDebugDrawer
{
	template<class T>
	void draw(Quadtree<T>& quadtree);
};

#endif