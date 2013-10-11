#ifndef ___FIRSTBLOOD_GAME_HPP___
#define ___FIRSTBLOOD_GAME_HPP___

#define GL_DEBUG

#include <vector>
#include "Engine.hpp"
#include "spatial/quadtree.hpp"
#include "spatial/kd_tree.hpp"

struct QuadtreeDebugObject
{
	QuadtreeDebugObject() : center(0, 0), radius(0) {};
	QuadtreeDebugObject(const vec2& inCenter, float inRadius) : center(inCenter), radius(inRadius) {};
	vec2 center;
	float radius;
	bool raycast(const vec3& origin, const vec3& end, float& dist) { return true; }
	inline float getRadius() { return radius; }
	inline vec3 getPosition() { return vec3(center.x, center.y, 0); }
	inline uint32_t getMask() { return 1; }
};


class Game : public Engine
{
public:
	Game();
	~Game();

protected:
	void Step(float frameTime);
	//void drawQuadtreeNode(Quadtree::Node* node);
	//void drawKdTreeNode(KdTree::Node* node);


protected:
	// debug crap
	std::vector<std::pair<Firstblood::RvoAgent*, vec2>> agents;
	Spatial::Quadtree<QuadtreeDebugObject>* quadtree;
	Spatial::KdTree<QuadtreeDebugObject>* kdTree;
};

#endif
