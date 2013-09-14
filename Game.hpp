#ifndef ___FIRSTBLOOD_GAME_HPP___
#define ___FIRSTBLOOD_GAME_HPP___

#define GL_DEBUG

#include <vector>

#include "Engine.hpp"
#include "spatial/quadtree.hpp"
#include "rvo/RVO.h"

class Game : public Engine
{
public:
	Game();
	~Game();

protected:
	void Step(float frameTime);

protected:
	RVO::RVOSimulator rvoSimulation;
	std::vector<RVO::Vector2> goals;
	Quadtree<int>* quadtree;

};

#endif
