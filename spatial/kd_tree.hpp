#ifndef __FBE_SPATIAL_KDTREE_HPP__
#define __FBE_SPATIAL_KDTREE_HPP__

#include "spatial/tree_base.hpp"

namespace Spatial
{

	template<class T>
	struct KdTreeNode : public TreeNode<T, KdTreeNode<T>, 2> {};

	template<class T>
	class KdTree : public TreeBase<T, KdTree, KdTreeNode>
	{
	public:
		enum { NODES_COUNT = 2 };

	public:
		KdTree(size_t maxLeafSize, size_t maxMemory) : _maxLeafSize(maxLeafSize)
		{
			_arena = new ArenaAllocator(maxMemory);
			_root = _arena->alloc<KdTreeNode<T>>();
		}

		virtual ~KdTree()
		{
			delete _root;
		}

		virtual void purge()
		{
			_arena->purge();
			_root = _arena->alloc<KdTreeNode<T>>();
		}

		virtual void build(T* objects, size_t objectsCount)
		{
			if (objectsCount > 0)
			{
				EntityList<T>* wrappedObjects = _arena->alloc<EntityList<T>>();
				wrappedObjects->entity = objects;
				for (size_t i = 1; i < objectsCount; ++i)
				{
					EntityList<T>* wrap = _arena->alloc<EntityList<T>>();
					wrap->entity = objects + i;
					wrap->next = wrappedObjects;
					wrappedObjects = wrap;
				}
				buildRecursively(_root, wrappedObjects, objectsCount);
			}
		}

		virtual void optimize() {}

	private:
		// todo: first pass over all objects gives us retarded nlogn bounding boxes computation
		void buildRecursively(KdTreeNode<T>* node, EntityList<T>* objects, size_t objectsCount)
		{
			node->min.x = FLT_MAX;
			node->min.y = FLT_MAX;
			node->max.x = -FLT_MAX;
			node->max.y = -FLT_MAX;
			EntityList<T>* currentObject = objects;
			while (currentObject != nullptr)
			{
				vec2 position = currentObject->getPosition();
				float radius = currentObject->getRadius();
				node->max.x = std::max(node->max.x, position.x + radius);
				node->min.x = std::min(node->min.x, position.x - radius);
				node->max.y = std::max(node->max.y, position.y + radius);
				node->min.y = std::min(node->min.y, position.y - radius);
				currentObject = currentObject->next;
			}

			if (objectsCount > _maxLeafSize)
			{
				// for static tree, consider using more interesting heuristic - splitting by median value
				bool isVertical = (node->max.x - node->min.x > node->max.y - node->min.y);
				float splitValue = (isVertical ? 0.5f * (node->max.x + node->min.x) : 0.5f * (node->max.y + node->min.y));

				EntityList<T>* left = nullptr;
				EntityList<T>* right = nullptr;
				size_t leftCount = 0;
				size_t rightCount = 0;
				currentObject = objects;
				while (currentObject != nullptr)
				{
					EntityList<T>* next = currentObject->next;
					vec2 position = currentObject->getPosition();
					if (isVertical ? position.x < splitValue : position.y < splitValue)
					{
						currentObject->next = left;
						left = currentObject;
						++leftCount;
					}
					else
					{
						currentObject->next = right;
						right = currentObject;
						++rightCount;
					}
					currentObject = next;
				}

				if (leftCount > 0)
				{
					node->children[0] = _arena->alloc<KdTreeNode<T>>();
					buildRecursively(node->children[0], left, leftCount);
				}
				else if (rightCount > 1)
				{
					--rightCount;
					++leftCount;
					left = right;
					right = right->next;
					left->next = nullptr;
					node->children[0] = _arena->alloc<KdTreeNode<T>>();
					buildRecursively(node->children[0], left, leftCount);
				}

				if (rightCount > 0)
				{
					node->children[1] = _arena->alloc<KdTreeNode<T>>();
					buildRecursively(node->children[1], right, rightCount);
				}
			}
			else
			{
				node->inhabitants = objects;
			}
		}

	private:
		size_t _maxLeafSize;
		ArenaAllocator* _arena;
	};

}

#endif