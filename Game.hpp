#ifndef ___FIRSTBLOOD_GAME_HPP___
#define ___FIRSTBLOOD_GAME_HPP___

#include "Engine.hpp"

class Game : public Engine
{
protected:
	void Step(float frameTime);
};

#endif
