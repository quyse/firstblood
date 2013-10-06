#include <algorithm>
#include "rvo/agent.hpp"
#include "rvo/simulator.hpp"
#include "geometry/distance.hpp"
#include <algorithm>

namespace RVO 
{
	
	Agent::Agent() : maxNeighbors(0), maxSpeed(0.0f), neighborDist(0.0f), radius(0.0f), timeHorizon(0.0f), immobilized(false), mask(1) {}

	Agent::~Agent() {};

	// todo: get rid of std::vector
	void Agent::computeNewVelocity(float dt, NearestNeighborsFinder* nearestNeighborsFinder)
	{
		size_t maxResultLength = std::min(RVO_GET_NEAREST_AGENTS_MAX_BUFFER_SIZE, maxNeighbors);
		NeighborEntity agentNeighbours[RVO_GET_NEAREST_AGENTS_MAX_BUFFER_SIZE];
		
		size_t neighboursCount = nearestNeighborsFinder->find(this, agentNeighbours, maxResultLength);

		Line orcaLines[MAX_ORCA_LINES];
		size_t orcaLinesCount = 0;

		const float invTimeHorizon = 1.0f / timeHorizon;

		/* Create agent ORCA lines. */
		for (size_t i = 0; i < neighboursCount; ++i) {
			NeighborEntity& other = agentNeighbours[i];

			const vec2 relativePosition = other.position - position;
			const vec2 relativeVelocity = velocity_ - other.velocity;
			const float distSq = length2(relativePosition);
			const float combinedRadius = radius + other.radius;
			const float combinedRadiusSq = sqr(combinedRadius);

			Line line;
			vec2 u;

			if (distSq > combinedRadiusSq) {
				/* No collision. */
				const vec2 w = relativeVelocity - relativePosition * invTimeHorizon;
				/* Vector from cutoff center to relative velocity. */
				const float wLengthSq = length2(w);

				const float dotProduct1 = dot(w, relativePosition);

				if (dotProduct1 < 0.0f && sqr(dotProduct1) > combinedRadiusSq * wLengthSq) {
					/* Project on cut-off circle. */
					const float wLength = std::sqrt(wLengthSq);
					const vec2 unitW = w / wLength;

					line.direction = vec2(unitW.y, -unitW.x);
					u = unitW * (combinedRadius * invTimeHorizon - wLength);
				}
				else {
					/* Project on legs. */
					const float leg = std::sqrt(distSq - combinedRadiusSq);

					if (det(relativePosition, w) > 0.0f) {
						/* Project on left leg. */
						line.direction = vec2(relativePosition.x * leg - relativePosition.y * combinedRadius, relativePosition.x * combinedRadius + relativePosition.y * leg) / distSq;
					}
					else {
						/* Project on right leg. */
						line.direction = -vec2(relativePosition.x * leg + relativePosition.y * combinedRadius, -relativePosition.x * combinedRadius + relativePosition.y * leg) / distSq;
					}

					const float dotProduct2 = dot(relativeVelocity, line.direction);

					u =  line.direction * dotProduct2 - relativeVelocity;
				}
			}
			else {
				/* Collision. Project on cut-off circle of time timeStep. */
				const float invTimeStep = 1.0f / dt;

				/* Vector from cutoff center to relative velocity. */
				const vec2 w = relativeVelocity -  relativePosition * invTimeStep;

				const float wLength = length(w);
				const vec2 unitW = w / wLength;

				line.direction = vec2(unitW.y, -unitW.x);
				u = unitW * (combinedRadius * invTimeStep - wLength);
			}

			line.point = velocity_ + u * 0.5f;
			if (orcaLinesCount < MAX_ORCA_LINES)
				orcaLines[orcaLinesCount++] = line;
		}

		size_t lineFail = linearProgram2(orcaLines, orcaLinesCount, maxSpeed, prefVelocity, false, newVelocity_);

		if (lineFail < orcaLinesCount) 
		{
			linearProgram3(orcaLines, orcaLinesCount, lineFail, maxSpeed, newVelocity_);
		}
	}


	void Agent::update(float dt)
	{
		velocity_ = newVelocity_;
		position += velocity_ * dt;
	}

}
