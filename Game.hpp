#ifndef ___FIRSTBLOOD_GAME_HPP___
#define ___FIRSTBLOOD_GAME_HPP___

#include "Engine.hpp"
#include "rvo/RVO.h"

class Game : public Engine
{
protected:
	void Step(float frameTime);

protected:
	RVO::RVOSimulator rvoSimulation;

};

#endif
