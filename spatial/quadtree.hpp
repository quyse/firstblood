#ifndef __FBE_QUADTREE_HPP__
#define __FBE_QUADTREE_HPP__

#include "memory/arena_allocator.hpp"
#include "geometry/intersections.hpp"
#include "spatial_index_interfaces.hpp"

#define GET_NEIGHBOURS_QUERY_MAX_BUFFER_SIZE 128

class VoidQuadtreeDebugDrawer;

template<class T, class DebugDrawer = VoidQuadtreeDebugDrawer>
class Quadtree : ISpatialIndex2D<T>
{

friend DebugDrawer;

public:
	Quadtree(float zeroLevelSize, size_t depth, size_t maxMemory):
		_zeroLevelSize(zeroLevelSize),
		_depth(depth),
		_sphereUidCounter(0)
	{
		_arena = new ArenaAllocator(maxMemory);
		_root = allocNode(_zeroLevelSize, 0, 0);
	}


	virtual ~Quadtree()
	{
		delete _arena;
	}
	

	virtual void addBoundingCircle(vec2& center, float radius, uint32_t mask, T* entity)
	{
		BoundingCircle* circle = allocCircle();
		circle->center = center;
		circle->radius = radius;
		circle->mask = mask;
		circle->entity = entity;
		addCircleRecursively(circle, _root, 0);
	}


	virtual void purge()
	{
		_arena->purge();
		_sphereUidCounter = 0;
		_root = allocNode(_zeroLevelSize, 0, 0);
	}


	virtual T* raycast(vec2& origin, vec2& end, uint32_t mask, float& t)
	{
		return raycastRecursively(origin, end, mask, t, _root);
	}


	virtual size_t getNeighbours(vec2& point, float distance, uint32_t mask, T** result, size_t maxResultLength)
	{
		if (maxResultLength > GET_NEIGHBOURS_QUERY_MAX_BUFFER_SIZE)
			maxResultLength = GET_NEIGHBOURS_QUERY_MAX_BUFFER_SIZE;
		PrioritizedBoundingCircle heap[GET_NEIGHBOURS_QUERY_MAX_BUFFER_SIZE];
		size_t currentResultLength = 0;
		getNeighboursRecursively(point, distance, mask, currentResultLength, maxResultLength, _root, heap);
		for (size_t i = 0; i < currentResultLength; ++i)
			*(result + i) = heap[i].circle->entity;
		return currentResultLength;
	}

private:
	struct BoundingCircle
	{
		BoundingCircle(size_t inUid) : next(nullptr), entity(nullptr), uid(inUid) {};
		vec2 center;
		float radius;
		uint32_t mask;
		T* entity;
		size_t uid;
		BoundingCircle* next;
	};

	struct PrioritizedBoundingCircle
	{
		PrioritizedBoundingCircle() : circle(nullptr), priority(FLT_MAX) {};
		BoundingCircle* circle;
		float priority;

		bool operator<(const PrioritizedBoundingCircle& other)
		{
			if (priority < other.priority)
				return true;
			else if (priority > other.priority)
				return false;
			else
				return circle->uid > other.circle->uid;
		}
	};

	struct Node
	{
		Node(float argSize, float x, float y) : 
			topLeft(nullptr), topRight(nullptr), botLeft(nullptr), botRight(nullptr), inhabitants(nullptr), 
			size(argSize), center(x, y), min(x - argSize, y - argSize), max(x + argSize, y + argSize) {};
		Node* topLeft;
		Node* topRight;
		Node* botLeft;
		Node* botRight;
		vec2 center;
		vec2 min;
		vec2 max;
		float size;
		BoundingCircle* inhabitants;
	};


private:
	void addCircleRecursively(BoundingCircle* circle, Node* currentNode, size_t currentLevel)
	{
		float size = currentNode->size;
		float nextLevelSize = 0.5f * size;
		float nextLevelHalfSize = 0.5f * nextLevelSize;
		if (circle->radius >= nextLevelHalfSize || currentLevel >= _depth)
		{
			circle->next = currentNode->inhabitants;
			currentNode->inhabitants = circle;
		}
		else
		{ 
			Node* deeperNode = nullptr;
			float x = currentNode->center.x;
			float y = currentNode->center.y;
			if (testPointAABB(circle->center, vec2(x - size, y - size), vec2(x, y)))
			{
				if (currentNode->topLeft == nullptr)
					currentNode->topLeft = allocNode(nextLevelSize, x - nextLevelHalfSize, y - nextLevelHalfSize);
				deeperNode = currentNode->topLeft;
			}
			else if (testPointAABB(circle->center, vec2(x, y - size), vec2(x + size, y)))
			{
				if (currentNode->topRight == nullptr)
					currentNode->topRight = allocNode(nextLevelSize, x + nextLevelHalfSize, y - nextLevelHalfSize);
				deeperNode = currentNode->topRight;
			}
			else if (testPointAABB(circle->center, vec2(x - size, y), vec2(x, y + size)))
			{
				if (currentNode->botLeft == nullptr)
					currentNode->botLeft = allocNode(nextLevelSize, x - nextLevelHalfSize, y + nextLevelHalfSize);
				deeperNode = currentNode->botLeft;
			}
			else if (testPointAABB(circle->center, vec2(x, y), vec2(x + size, y + size)))
			{
				if (currentNode->botRight == nullptr)
					currentNode->botRight = allocNode(nextLevelSize, x + nextLevelHalfSize, y + nextLevelHalfSize);
				deeperNode = currentNode->botRight;
			}

			if (deeperNode != nullptr)
				addCircleRecursively(circle, deeperNode, currentLevel + 1);
			else
				std::cout << "Cannot add node to quadtree: " << circle->center << std::endl;
		}
	}


	T* raycastRecursively(vec2& origin, vec2& end, uint32_t mask, float& t, Node* node)
	{
		vec2 clippedOrigin, clippedEnd;
		float dist;
		float tmin, tmax;
		bool intersects = intersectSegmentAABB(origin, end, node->min, node->max, clippedOrigin, clippedEnd, tmin, tmax);
		if (!intersects)
			return nullptr;

		BoundingCircle* currentInhabitant = node->inhabitants;
		vec2 i0, i1;
		float minDist = FLT_MAX;
		T* chosenEntity = nullptr;
		while (currentInhabitant != nullptr)
		{
			if ((mask & currentInhabitant->mask) && intersectSegmentSphere(clippedOrigin, clippedEnd, currentInhabitant->center, currentInhabitant->radius, i0, i1, tmin, tmax))
			{
				T* currentEntity = currentInhabitant->entity;
				if (currentEntity->raycast(clippedOrigin, clippedEnd, dist))
				{
					if (dist < minDist)
					{
						minDist = dist;
						chosenEntity = currentEntity;
					}
				}
			}
			currentInhabitant = currentInhabitant->next;
		}

		raycastChild(node->topLeft, clippedOrigin, clippedEnd, mask, chosenEntity, minDist);
		raycastChild(node->botLeft, clippedOrigin, clippedEnd, mask, chosenEntity, minDist);
		raycastChild(node->topRight, clippedOrigin, clippedEnd, mask, chosenEntity, minDist);
		raycastChild(node->botRight, clippedOrigin, clippedEnd, mask, chosenEntity, minDist);

		t = minDist;
		return chosenEntity;
	}


	inline void raycastChild(Node* child, vec2& clippedOrigin, vec2& clippedEnd, uint32_t mask, T*& chosenEntity, float& minDist)
	{
		if (child != nullptr)
		{
			float dist;
			T* childEntity = raycastRecursively(clippedOrigin, clippedEnd, mask, dist, child);
			if (dist < minDist)
			{
				minDist = dist;
				chosenEntity = childEntity;
			}
		}
	}


	size_t getNeighboursRecursively(vec2& point, float distance, uint32_t mask, size_t currentResultLength, size_t maxResultLength, Node* currentNode, PrioritizedBoundingCircle* heap)
	{
		//BoundingCircle* inhabitant =
		return 0;
	}


	inline Node* allocNode(float size, float x, float y)
	{
		void* memory = _arena->alloc(sizeof(Node));
		assert(memory != nullptr);
		return new (memory) Node(size, x, y);
	}


	inline BoundingCircle* allocCircle()
	{
		void* memory = _arena->alloc(sizeof(BoundingCircle));
		assert(memory != nullptr);
		return new (memory) BoundingCircle(++_sphereUidCounter);
	}


private:
	float _zeroLevelSize;
	size_t _depth;
	Node* _root;
	ArenaAllocator* _arena;
	size_t _sphereUidCounter;
};


class VoidQuadtreeDebugDrawer
{
	template<class T>
	void draw(Quadtree<T>& quadtree);
};

#endif