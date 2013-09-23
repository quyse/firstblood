#ifndef __FBE_SPATIAL_TREE_BASE_HPP__
#define __FBE_SPATIAL_TREE_BASE_HPP__

#include "memory/arena_allocator.hpp"
#include "geometry/intersections.hpp"
#include "spatial/interfaces.hpp"

#define GET_NEIGHBOURS_QUERY_MAX_BUFFER_SIZE (size_t)128

namespace Spatial
{
	// the linked list of tree node's inhabitants
	// supposed to be allocated by tree's arena allocator
	template<class T>
	struct EntityList
	{
		EntityList() : entity(nullptr), next(nullptr) {};

		T* entity;
		EntityList<T>* next;

		inline uint32_t getMask() { return entity->getMask(); }
		inline float getRadius() { return entity->getRadius(); }
		inline vec2 getPosition() { return entity->getPosition(); }
	};

	// tree node with N children
	template<class T, class Descendant, int N>
	struct TreeNode
	{
		TreeNode() : inhabitants(nullptr) 
		{
			for (size_t i = 0; i < N; ++i)
				children[i] = nullptr;
		};

		vec2 min;
		vec2 max;
		EntityList<T>* inhabitants;
		Descendant* children[N];
	};

	// getNeighbours query uses this struct for heap-sorting candidates
	template<class T>
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


	// base tree class, containing utility Node typedef and implementations of getNearestNeighbours and rayCast queries
	template<class T, template<class T> class Descendant, template<class T> class Node>
	class TreeBase : public ISpatialIndex2D<T>
	{
	public:
		virtual T* raycast(const vec2& origin, const vec2& end, uint32_t mask, float& t, T* skipEntity = nullptr)
		{
			return raycastRecursively(origin, end, mask, t, _root, skipEntity);
		}

		virtual size_t getNeighbours(const vec2& point, float distance, uint32_t mask, T** result, size_t maxResultLength, T* skipEntity = nullptr) const
		{
			if (maxResultLength == 0)
				return 0;
			maxResultLength = std::min(maxResultLength, GET_NEIGHBOURS_QUERY_MAX_BUFFER_SIZE);

			PrioritizedEntity<T> heap[GET_NEIGHBOURS_QUERY_MAX_BUFFER_SIZE + 1];
			size_t currentResultLength = 0;
			float currentMinDistance = distance;

			getNeighboursRecursively(point, currentMinDistance, mask, currentResultLength, maxResultLength, _root, heap, skipEntity);

			for (size_t i = 0; i < currentResultLength; ++i)
				*(result + i) = heap[i].entity;
			return currentResultLength;
		}

	protected:
		T* raycastRecursively(const vec2& origin, const vec2& end, uint32_t mask, float& t, Node<T>* node, T* skipEntity)
		{
			vec2 clippedOrigin, clippedEnd;
			float dist;
			float tmin, tmax;
			bool intersects = intersectSegmentAABB(origin, end, node->min, node->max, clippedOrigin, clippedEnd, tmin, tmax);
			if (!intersects)
				return nullptr;
		
			EntityList<T>* currentInhabitant = node->inhabitants;
			vec2 i0, i1;
			t = FLT_MAX;
			T* chosenEntity = nullptr;
			while (currentInhabitant != nullptr)
			{
				if ((mask & currentInhabitant->getMask()) && intersectSegmentSphere(origin, end, currentInhabitant->getPosition(), currentInhabitant->getRadius(), i0, i1, tmin, tmax))
				{
					T* currentEntity = currentInhabitant->entity;
					if (currentEntity != skipEntity && currentEntity->raycast(origin, end, dist))
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

			for (size_t i = 0; i < Descendant<T>::NODES_COUNT; ++i)
			{
				Node<T>* child = node->children[i];
				if (child != nullptr)
				{
					float dist;
					T* childEntity = raycastRecursively(origin, end, mask, dist, child, skipEntity);
					if (childEntity != nullptr && dist < t)
					{
						t = dist;
						chosenEntity = childEntity;
					}
				}
			}

			return chosenEntity;
		}

		void getNeighboursRecursively(const vec2& point, float& distance, uint32_t mask, size_t& currentResultLength, size_t maxResultLength, Node<T>* currentNode, PrioritizedEntity<T>* heap, T* skipEntity) const
		{
			if (!testSphereAABB(point, distance, currentNode->min, currentNode->max))
				return;
		
			EntityList<T>* inhabitant = currentNode->inhabitants;
			while (inhabitant != nullptr)
			{
				if ((inhabitant->getMask() & mask) && (inhabitant->entity != skipEntity))
				{
					vec2 center = inhabitant->getPosition();
					float dx = point.x - center.x;
					float dy = point.y - center.y;
					float radius = inhabitant->getRadius();
					float sumR = distance + radius;
					float sqrDist = dx * dx + dy * dy;
					if (sqrDist < sumR * sumR)
					{
						float priority = std::max(0.0f, sqrtf(sqrDist) - radius);
						if (currentResultLength < maxResultLength)
						{
							(heap + currentResultLength)->priority = priority;
							(heap + currentResultLength)->entity = inhabitant->entity;
							std::push_heap(heap, heap + currentResultLength + 1);
							++currentResultLength;
						}
						else
						{
							if (priority < heap->priority)
							{
								std::pop_heap(heap, heap + currentResultLength);
								(heap + currentResultLength - 1)->priority = priority;
								(heap + currentResultLength - 1)->entity = inhabitant->entity;
								std::push_heap(heap, heap + currentResultLength);
								distance = heap->priority;
							}
						}
					}
				}
				inhabitant = inhabitant->next;
			}

			for (size_t i = 0; i < Descendant<T>::NODES_COUNT; ++i)
			{
				Node<T>* child = currentNode->children[i];
				if (child != nullptr)
					getNeighboursRecursively(point, distance, mask, currentResultLength, maxResultLength, child, heap, skipEntity);
			}
		}

	protected:
		Node<T>* _root;
	};

}

#endif