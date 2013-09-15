#ifndef __FBE_QUADTREE_HPP__
#define __FBE_QUADTREE_HPP__

#include <assert.h>
#include "memory/arena_allocator.hpp"
#include "geometry/intersections.hpp"
#include "spatial_index_interfaces.hpp"

#define GET_NEIGHBOURS_QUERY_MAX_BUFFER_SIZE 128

class VoidQuadtreeDebugDrawer;

// TODO: I really hate these ugly smelly botLeft, botRight, topLeft, topRight fields. How about a nice array of child nodes' coordinates?
// TODO: Node's bounding box should not be 2Nx2N sized by default - stretch it only as much as necessary
template<class T, class DebugDrawer = VoidQuadtreeDebugDrawer>
class Quadtree : ISpatialIndex2D<T>
{

friend DebugDrawer;

public:
	Quadtree(float zeroLevelSize, size_t depth, size_t maxMemory):
		_zeroLevelSize(zeroLevelSize),
		_depth(depth)
	{
		_arena = new ArenaAllocator(maxMemory);
		_root = allocNode(_zeroLevelSize, 0, 0);
	}


	virtual ~Quadtree()
	{
		delete _arena;
	}
	

	virtual void addBoundingCircle(const vec2& center, float radius, uint32_t mask, T* entity)
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
		_root = allocNode(_zeroLevelSize, 0, 0);
	}


	virtual T* raycast(const vec2& origin, const vec2& end, uint32_t mask, float& t)
	{
		return raycastRecursively(origin, end, mask, t, _root);
	}


	virtual size_t getNeighbours(const vec2& point, float distance, uint32_t mask, T** result, size_t maxResultLength) const
	{
		assert(maxResultLength > 0);
		if (maxResultLength > GET_NEIGHBOURS_QUERY_MAX_BUFFER_SIZE)
			maxResultLength = GET_NEIGHBOURS_QUERY_MAX_BUFFER_SIZE;
		PrioritizedBoundingCircle heap[GET_NEIGHBOURS_QUERY_MAX_BUFFER_SIZE + 1];
		size_t currentResultLength = 0;
		getNeighboursRecursively(point, distance, mask, currentResultLength, maxResultLength, _root, heap);
		for (size_t i = 0; i < currentResultLength; ++i)
			*(result + i) = heap[i].circle->entity;
		return currentResultLength;
	}

	// each node's bounding box is shrinked to exactly fit it's content
	void minify()
	{
		minifyRecursively(_root);
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

	struct PrioritizedBoundingCircle
	{
		PrioritizedBoundingCircle() : circle(nullptr), priority(FLT_MAX) {};
		BoundingCircle* circle;
		float priority;
		
		bool operator<(const PrioritizedBoundingCircle& other)
		{
			return priority < other.priority;
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


	T* raycastRecursively(const vec2& origin, const vec2& end, uint32_t mask, float& t, Node* node)
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
			if ((mask & currentInhabitant->mask) && intersectSegmentSphere(origin, end, currentInhabitant->center, currentInhabitant->radius, i0, i1, tmin, tmax))
			{
				T* currentEntity = currentInhabitant->entity;
				if (currentEntity->raycast(origin, end, dist))
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

		raycastChild(node->topLeft, origin, end, mask, chosenEntity, minDist);
		raycastChild(node->botLeft, origin, end, mask, chosenEntity, minDist);
		raycastChild(node->topRight, origin, end, mask, chosenEntity, minDist);
		raycastChild(node->botRight, origin, end, mask, chosenEntity, minDist);

		t = minDist;
		return chosenEntity;
	}


	inline void raycastChild(Node* child, const vec2& origin, const vec2& end, uint32_t mask, T*& chosenEntity, float& minDist)
	{
		if (child != nullptr)
		{
			float dist;
			T* childEntity = raycastRecursively(origin, end, mask, dist, child);
			if (childEntity != nullptr && dist < minDist)
			{
				minDist = dist;
				chosenEntity = childEntity;
			}
		}
	}


	void getNeighboursRecursively(const vec2& point, float distance, uint32_t mask, size_t& currentResultLength, size_t maxResultLength, Node* currentNode, PrioritizedBoundingCircle* heap) const
	{
		if (!testAABBAABB(vec2(point.x - distance, point.y - distance), vec2(point.x + distance, point.y + distance), currentNode->min, currentNode->max))
			return;
		
		BoundingCircle* inhabitant = currentNode->inhabitants;
		while (inhabitant != nullptr)
		{
			if (inhabitant->mask & mask)
			{
				float dx = point.x - inhabitant->center.x;
				float dy = point.y - inhabitant->center.y;
				float radius = inhabitant->radius;
				float sumR = distance + radius;
				float sqrDist = dx * dx + dy * dy;
				if (sqrDist < sumR * sumR)
				{
					float priority = sqrtf(sqrDist) - radius;
					if (currentResultLength < maxResultLength)
					{
						(*(heap + currentResultLength)).priority = priority;
						(*(heap + currentResultLength)).circle = inhabitant;
						std::push_heap(heap, heap + currentResultLength + 1);
						++currentResultLength;
					}
					else
					{
						if (priority < heap->priority)
						{
							std::pop_heap(heap, heap + currentResultLength);
							(*(heap + currentResultLength - 1)).priority = priority;
							(*(heap + currentResultLength - 1)).circle = inhabitant;
							std::push_heap(heap, heap + currentResultLength);
						}
					}
				}
			}
			inhabitant = inhabitant->next;
		}

		if (currentNode->topLeft != nullptr)
			getNeighboursRecursively(point, distance, mask, currentResultLength, maxResultLength, currentNode->topLeft, heap);
		if (currentNode->topRight != nullptr)
			getNeighboursRecursively(point, distance, mask, currentResultLength, maxResultLength, currentNode->topRight, heap);
		if (currentNode->botLeft != nullptr)
			getNeighboursRecursively(point, distance, mask, currentResultLength, maxResultLength, currentNode->botLeft, heap);
		if (currentNode->botRight != nullptr)
			getNeighboursRecursively(point, distance, mask, currentResultLength, maxResultLength, currentNode->botRight, heap);
	}


	void minifyRecursively(Node* currentNode)
	{
		float minX = FLT_MAX;
		float minY = FLT_MAX;
		float maxX = -FLT_MAX;
		float maxY = -FLT_MAX;

		minifyChild(currentNode->botLeft, minX, minY, maxX, maxY);
		minifyChild(currentNode->topLeft, minX, minY, maxX, maxY);
		minifyChild(currentNode->botRight, minX, minY, maxX, maxY);
		minifyChild(currentNode->topRight, minX, minY, maxX, maxY);

		BoundingCircle* inhabitant = currentNode->inhabitants;
		while (inhabitant != nullptr)
		{
			vec2& center = inhabitant->center;
			float radius = inhabitant->radius;
			minX = std::min(minX, center.x - radius);
			minY = std::min(minY, center.y - radius);
			maxX = std::max(maxX, center.x + radius);
			maxY = std::max(maxY, center.y + radius);
			inhabitant = inhabitant->next;
		}

		currentNode->min.x = minX;
		currentNode->min.y = minY;
		currentNode->max.x = maxX;
		currentNode->max.y = maxY;
	}


	void minifyChild(Node* child, float& minX, float& minY, float& maxX, float& maxY)
	{
		if (child != nullptr)
		{
			minifyRecursively(child);
			minX = std::min(minX, child->min.x);
			minY = std::min(minY, child->min.y);
			maxX = std::max(maxX, child->max.x);
			maxY = std::max(maxY, child->max.y);
		}
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
		return new (memory) BoundingCircle();
	}


private:
	float _zeroLevelSize;
	size_t _depth;
	Node* _root;
	ArenaAllocator* _arena;
};


class VoidQuadtreeDebugDrawer
{
	template<class T>
	void draw(Quadtree<T>& quadtree);
};

#endif