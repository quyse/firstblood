#include "rvo/obstacle.hpp"

namespace RVO {
	Obstacle::Obstacle() : isConvex_(false), nextObstacle_(NULL), prevObstacle_(NULL), id_(0) { }
}
