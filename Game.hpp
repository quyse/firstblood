#ifndef ___FIRSTBLOOD_GAME_HPP___
#define ___FIRSTBLOOD_GAME_HPP___

#define GL_DEBUG

#include <vector>

#include "Engine.hpp"
#include "spatial/quadtree.hpp"
#include "spatial/kd_tree.hpp"
#include "rvo/simulator.hpp"
#include "gamelogic/common.hpp"
#include "gamelogic/rvo.hpp"

struct QuadtreeDebugObject
{
	QuadtreeDebugObject() : center(0, 0), radius(0) {};
	QuadtreeDebugObject(const vec2& inCenter, float inRadius) : center(inCenter), radius(inRadius) {};
	vec2 center;
	float radius;
	bool raycast(const vec2& origin, const vec2& end, float& dist) { return true; }
	inline float getRadius() { return radius; }
	inline vec2 getPosition() { return center; }
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
	// spatial index
	Spatial::IIndex2D<Firstblood::ISpatiallyIndexable>* spatialIndex;
	// rvo
	Firstblood::RvoSimulation* rvoSimulation;

	// debug crap
	std::vector<std::pair<Firstblood::RvoAgent*, vec2>> agents;
	Spatial::Quadtree<QuadtreeDebugObject>* quadtree;
	Spatial::KdTree<QuadtreeDebugObject>* kdTree;
};

#endif
