#ifndef __FBE_RVO_OBSTACLE_H__
#define __FBE_RVO_OBSTACLE_H__

#include "rvo/math.hpp"

namespace RVO {

	class Agent;
	class KdTree;
	class Simulator;

	class Obstacle {
	private:
		Obstacle();

		bool isConvex_;
		Obstacle* nextObstacle_;
		vec2 point_;
		Obstacle* prevObstacle_;
		vec2 unitDir_;

		size_t id_;

		friend class Agent;
		friend class KdTree;
		friend class Simulator;
	};
}

#endif
