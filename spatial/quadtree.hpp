#ifndef __FBE_QUADTREE_HPP__
#define __FBE_QUADTREE_HPP__

#include <assert.h>
#include "memory/arena_allocator.hpp"
#include "geometry/intersections.hpp"
#include "spatial_index_interfaces.hpp"

#define QUADTREE_GET_NEIGHBOURS_QUERY_MAX_BUFFER_SIZE 128
#define QUADTREE_CHILDREN_COUNT 4

struct QuadtreeNodeDescription
{
	float x;
	float y;
	float minX;
	float minY;
	float maxX;
	float maxY;
};

static const QuadtreeNodeDescription childNodesDescription[QUADTREE_CHILDREN_COUNT] =
{
	{-1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f}, 
	{1.0f, -1.0f, 0.0f, -1.0f, 1.0f, 0.0f}, 
	{-1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f}, 
	{1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f}
};

class VoidQuadtreeDebugDrawer;


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
		if (maxResultLength > QUADTREE_GET_NEIGHBOURS_QUERY_MAX_BUFFER_SIZE)
			maxResultLength = QUADTREE_GET_NEIGHBOURS_QUERY_MAX_BUFFER_SIZE;
		PrioritizedEntity heap[QUADTREE_GET_NEIGHBOURS_QUERY_MAX_BUFFER_SIZE + 1];
		size_t currentResultLength = 0;
		getNeighboursRecursively(point, distance, mask, currentResultLength, maxResultLength, _root, heap);
		for (size_t i = 0; i < currentResultLength; ++i)
			*(result + i) = heap[i].entity;
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

	struct PrioritizedEntity
	{
		PrioritizedEntity() : entity(nullptr), priority(FLT_MAX) {};
		T* entity;
		float priority;
		
		bool operator<(const PrioritizedEntity& other)
		{
			return priority < other.priority;
		}
	};

	struct Node
	{
		Node(float argSize, float x, float y) :  inhabitants(nullptr), size(argSize), min(x - argSize, y - argSize), max(x + argSize, y + argSize) 
		{
			for (size_t i = 0; i < QUADTREE_CHILDREN_COUNT; ++i)
				children[i] = nullptr;
		};

		vec2 min;
		vec2 max;
		float size;
		BoundingCircle* inhabitants;
		Node* children[QUADTREE_CHILDREN_COUNT];
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
			float x = 0.5f * (currentNode->min.x + currentNode->max.x);
			float y = 0.5f * (currentNode->min.y + currentNode->max.y);
			for (size_t i = 0; i < QUADTREE_CHILDREN_COUNT; ++i)
			{
				QuadtreeNodeDescription nodeDesc = childNodesDescription[i];
				vec2 min(x + nodeDesc.minX * size, y + nodeDesc.minY * size);
				vec2 max(x + nodeDesc.maxX * size, y + nodeDesc.maxY * size);
				if (testPointAABB(circle->center, min, max))
				{
					if (currentNode->children[i] == nullptr)
						currentNode->children[i] = allocNode(nextLevelSize, x + nodeDesc.x * nextLevelHalfSize, y + nodeDesc.y * nextLevelHalfSize);
					deeperNode = currentNode->children[i];
					break;
				}
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
		t = FLT_MAX;
		T* chosenEntity = nullptr;
		while (currentInhabitant != nullptr)
		{
			if ((mask & currentInhabitant->mask) && intersectSegmentSphere(origin, end, currentInhabitant->center, currentInhabitant->radius, i0, i1, tmin, tmax))
			{
				T* currentEntity = currentInhabitant->entity;
				if (currentEntity->raycast(origin, end, dist))
				{
					if (dist < t)
					{
						t = dist;
						chosenEntity = currentEntity;
					}
				}
			}
			currentInhabitant = currentInhabitant->next;
		}

		for (size_t i = 0; i < QUADTREE_CHILDREN_COUNT; ++i)
		{
			Node* child = node->children[i];
			if (child != nullptr)
			{
				float dist;
				T* childEntity = raycastRecursively(origin, end, mask, dist, child);
				if (childEntity != nullptr && dist < t)
				{
					t = dist;
					chosenEntity = childEntity;
				}
			}
		}

		return chosenEntity;
	}


	void getNeighboursRecursively(const vec2& point, float distance, uint32_t mask, size_t& currentResultLength, size_t maxResultLength, Node* currentNode, PrioritizedEntity* heap) const
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
						(*(heap + currentResultLength)).entity = inhabitant->entity;
						std::push_heap(heap, heap + currentResultLength + 1);
						++currentResultLength;
					}
					else
					{
						if (priority < heap->priority)
						{
							std::pop_heap(heap, heap + currentResultLength);
							(*(heap + currentResultLength - 1)).priority = priority;
							(*(heap + currentResultLength - 1)).entity = inhabitant->entity;
							std::push_heap(heap, heap + currentResultLength);
						}
					}
				}
			}
			inhabitant = inhabitant->next;
		}

		for (size_t i = 0; i < QUADTREE_CHILDREN_COUNT; ++i)
		{
			Node* child = currentNode->children[i];
			if (child != nullptr)
				getNeighboursRecursively(point, distance, mask, currentResultLength, maxResultLength, child, heap);
		}
	}


	void minifyRecursively(Node* currentNode)
	{
		float minX = FLT_MAX;
		float minY = FLT_MAX;
		float maxX = -FLT_MAX;
		float maxY = -FLT_MAX;

		for (size_t i = 0; i < QUADTREE_CHILDREN_COUNT; ++i)
		{
			Node* child = currentNode->children[i];
			if (child != nullptr)
			{
				minifyRecursively(child);
				minX = std::min(minX, child->min.x);
				minY = std::min(minY, child->min.y);
				maxX = std::max(maxX, child->max.x);
				maxY = std::max(maxY, child->max.y);
			}
		}

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