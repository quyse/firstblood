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

		// todo: these ones are ugly and probably useless
		inline uint32_t getMask() { return entity->getMask(); }
		inline float getRadius() { return entity->getRadius(); }
		inline vec3 getPosition() { return entity->getPosition(); }
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


	// base tree class, containing utility Node typedef and implementations of getNearestNeighbours and rayCast queries
	template<class T, template<class T> class Descendant, template<class T> class Node>
	class TreeBase : public IIndex2D<T>
	{
	public:
		virtual T* raycast(const vec3& origin, const vec3& end, uint32_t mask, float& t, T* skipEntity = nullptr)
		{
			return raycastRecursively(origin, end, mask, t, _root, skipEntity);
		}

		virtual size_t getNeighbours(const vec3& point, float distance, uint32_t mask, NearestNeighbor<T>* result, size_t maxResultLength, T* skipEntity = nullptr) const
		{
			if (maxResultLength == 0)
				return 0;
			size_t currentResultLength = 0;
			float currentMinDistance = distance;
			getNeighboursRecursively(point, currentMinDistance, mask, currentResultLength, maxResultLength, _root, result, skipEntity);
			return currentResultLength;
		}

		virtual void draw(IDrawer& drawer)
		{
			drawRecursively(_root, drawer);
		}

	protected:
		// todo: should be iteratively
		// todo: cull the segment by the current minimum distance
		T* raycastRecursively(const vec3& origin, const vec3& end, uint32_t mask, float& t, Node<T>* node, T* skipEntity)
		{
			vec2 clippedOrigin, clippedEnd;
			float tmin, tmax;
			bool intersects = intersectSegmentAABB(vec2(origin.x, origin.y), vec2(end.x, end.y), node->min, node->max, clippedOrigin, clippedEnd, tmin, tmax);
			if (!intersects)
				return nullptr;
		
			EntityList<T>* currentInhabitant = node->inhabitants;
			vec3 i0, i1;
			t = FLT_MAX;
			T* chosenEntity = nullptr;
			while (currentInhabitant != nullptr)
			{
				if ((mask & currentInhabitant->getMask()) && intersectSegmentSphere(origin, end, currentInhabitant->getPosition(), currentInhabitant->getRadius(), i0, i1, tmin, tmax))
				{
					T* currentEntity = currentInhabitant->entity;
					float dist = length(i0 - origin);
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

		// todo: should be iteratively
		// todo: maybe heap sorting ain't worth it, try to use simple insertion sort instead
		void getNeighboursRecursively(const vec3& point, float& distance, uint32_t mask, size_t& currentResultLength, size_t maxResultLength, Node<T>* currentNode, NearestNeighbor<T>* heap, T* skipEntity) const
		{
			if (!testSphereAABB(vec2(point.x, point.y), distance, currentNode->min, currentNode->max))
				return;
		
			EntityList<T>* inhabitant = currentNode->inhabitants;
			while (inhabitant != nullptr)
			{
				if ((inhabitant->getMask() & mask) && (inhabitant->entity != skipEntity))
				{
					vec3 center = inhabitant->getPosition();
					float dx = point.x - center.x;
					float dy = point.y - center.y;
					float radius = inhabitant->getRadius();
					float sumR = distance + radius;
					float sqrDist = dx * dx + dy * dy;
					if (sqrDist < sumR * sumR)
					{
						float actualDistance = std::max(0.0f, sqrtf(sqrDist) - radius);
						if (currentResultLength < maxResultLength)
						{
							(heap + currentResultLength)->distance = actualDistance;
							(heap + currentResultLength)->entity = inhabitant->entity;
							std::push_heap(heap, heap + currentResultLength + 1);
							++currentResultLength;
						}
						else
						{
							if (actualDistance < heap->distance)
							{
								std::pop_heap(heap, heap + currentResultLength);
								(heap + currentResultLength - 1)->distance = actualDistance;
								(heap + currentResultLength - 1)->entity = inhabitant->entity;
								std::push_heap(heap, heap + currentResultLength);
								distance = heap->distance;
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

		void drawRecursively(Node<T>* node, IDrawer& drawer)
		{
			drawer.drawNode(node->min, node->max);
			EntityList<T>* inhabitant = node->inhabitants;
			while (inhabitant != nullptr)
			{
				vec3 position = inhabitant->getPosition();
				drawer.drawInhabitant(vec2(position.x, position.y), inhabitant->getRadius());
				inhabitant = inhabitant->next;
			}
			for (size_t i = 0; i < Descendant<T>::NODES_COUNT; ++i)
			{
				Node<T>* child = node->children[i];
				if (child != nullptr)
					drawRecursively(child, drawer);
			}
		}

	protected:
		Node<T>* _root;
	};

}

#endif