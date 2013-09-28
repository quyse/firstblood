#ifndef __FBE_SPATIAL_QUADTREE_HPP__
#define __FBE_SPATIAL_QUADTREE_HPP__

#include "spatial/tree_base.hpp"

namespace Spatial
{

	struct QuadtreeNodeDescription
	{
		float x;
		float y;
		float minX;
		float minY;
		float maxX;
		float maxY;
	};

	static const QuadtreeNodeDescription childNodesDescription[4] =
	{
		{-1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f}, 
		{1.0f, -1.0f, 0.0f, -1.0f, 1.0f, 0.0f}, 
		{-1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f}, 
		{1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f}
	};

	template<class T>
	struct QuadtreeNode : public TreeNode<T, QuadtreeNode<T>, 4>
	{
		float size;
	};

	// this quadtree is a loose one (e.g. all nodes are doubled in size)
	// google "loose octree" to find out why it can be nice
	// call optimize when builing is done to shrink nodes, if you want
	template<class T>
	class Quadtree : public TreeBase<T, Quadtree, QuadtreeNode>
	{
	public:
		enum { NODES_COUNT = 4 };

	public:
		Quadtree(size_t depth, float zeroLevelSize, size_t maxMemory) : _depth(depth), _zeroLevelSize(zeroLevelSize)
		{
			_arena = new ArenaAllocator(maxMemory);
			_root = _arena->alloc<QuadtreeNode<T>>();
			initNode(_root, _zeroLevelSize, 0, 0);
		}

		virtual ~Quadtree()
		{
			delete _arena;
		}

		virtual void purge()
		{
			_arena->purge();
			_root = _arena->alloc<QuadtreeNode<T>>();
			initNode(_root, _zeroLevelSize, 0, 0);
		}

		virtual void build(T* objects, size_t objectsCount)
		{
			for (size_t i = 0; i < objectsCount; ++i)
			{
				T* object = objects + i;
				float radius = object->getRadius();
				vec2 position = object->getPosition();
				addObjectRecursively(object, radius, position, _root, 0);
			}
		}

		virtual void build(T** objects, size_t objectsCount)
		{
			for (size_t i = 0; i < objectsCount; ++i)
			{
				T* object = *(objects + i);
				float radius = object->getRadius();
				vec2 position = object->getPosition();
				addObjectRecursively(object, radius, position, _root, 0);
			}
		}

		// each node's bounding box is shrinked to exactly fit it's content
		virtual void optimize()
		{
			minifyRecursively(_root);
		}

	private:
		void addObjectRecursively(T* object, float radius, const vec2& position, QuadtreeNode<T>* currentNode, size_t currentLevel)
		{
			float size = currentNode->size;
			float nextLevelSize = 0.5f * size;
			float nextLevelHalfSize = 0.5f * nextLevelSize;
			if (radius >= nextLevelHalfSize || currentLevel >= _depth)
			{
				EntityList<T>* wrapper = _arena->alloc<EntityList<T>>();
				wrapper->entity = object;
				wrapper->next = currentNode->inhabitants;
				currentNode->inhabitants = wrapper;
			}
			else
			{ 
				QuadtreeNode<T>* deeperNode = nullptr;
				float x = 0.5f * (currentNode->min.x + currentNode->max.x);
				float y = 0.5f * (currentNode->min.y + currentNode->max.y);
				for (size_t i = 0; i < 4; ++i)
				{
					QuadtreeNodeDescription nodeDesc = childNodesDescription[i];
					vec2 min(x + nodeDesc.minX * size, y + nodeDesc.minY * size);
					vec2 max(x + nodeDesc.maxX * size, y + nodeDesc.maxY * size);
					if (testPointAABB(position, min, max))
					{
						if (currentNode->children[i] == nullptr)
						{
							currentNode->children[i] = _arena->alloc<QuadtreeNode<T>>();
							initNode(currentNode->children[i], nextLevelSize, x + nodeDesc.x * nextLevelHalfSize, y + nodeDesc.y * nextLevelHalfSize);
						}
						deeperNode = currentNode->children[i];
						break;
					}
				}

				if (deeperNode != nullptr)
					addObjectRecursively(object, radius, position, deeperNode, currentLevel + 1);
				else
					std::cout << "Cannot add object to quadtree: " << position << std::endl;
			}
		}

		void minifyRecursively(QuadtreeNode<T>* currentNode)
		{
			float minX = FLT_MAX;
			float minY = FLT_MAX;
			float maxX = -FLT_MAX;
			float maxY = -FLT_MAX;

			for (size_t i = 0; i < 4; ++i)
			{
				QuadtreeNode<T>* child = currentNode->children[i];
				if (child != nullptr)
				{
					minifyRecursively(child);
					minX = std::min(minX, child->min.x);
					minY = std::min(minY, child->min.y);
					maxX = std::max(maxX, child->max.x);
					maxY = std::max(maxY, child->max.y);
				}
			}

			EntityList<T>* inhabitant = currentNode->inhabitants;
			while (inhabitant != nullptr)
			{
				vec2& center = inhabitant->getPosition();
				float radius = inhabitant->getRadius();
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

		inline void initNode(QuadtreeNode<T>* node, float size, float x, float y)
		{
			node->min.x = x - size;
			node->min.y = y - size;
			node->max.x = x + size;
			node->max.y = y + size;
			node->size = size;
		}

	private:
		float _zeroLevelSize;
		size_t _depth;
		ArenaAllocator* _arena;
	};

}

#endif