#include "rvo/agent.hpp"
#include "rvo/kd_tree.hpp"
#include "rvo/simulator.hpp"
#include "geometry/distance.hpp"

#define GET_NEAREST_AGENTS_MAX_BUFFER_SIZE (size_t)128

namespace RVO 
{
	
	Agent::Agent() : maxNeighbors(0), maxSpeed(0.0f), neighborDist(0.0f), radius(0.0f), timeHorizon(0.0f), immobilized(false) {}


	void Agent::computeNewVelocity(float dt, KdTree* spatialIndex)
	{
		size_t maxResultLength = std::min(GET_NEAREST_AGENTS_MAX_BUFFER_SIZE, maxNeighbors);
		std::pair<float, Agent*> agentNeighbours[GET_NEAREST_AGENTS_MAX_BUFFER_SIZE];
		
		float rangeSq = sqr(neighborDist);
		size_t neighboursCount = spatialIndex->getNeighbours(this, rangeSq, maxResultLength, &(agentNeighbours[0]));

		std::vector<Line> orcaLines;

		const float invTimeHorizon = 1.0f / timeHorizon;

		/* Create agent ORCA lines. */
		for (size_t i = 0; i < neighboursCount; ++i) {
			const Agent *const other = agentNeighbours[i].second;

			const vec2 relativePosition = other->position - position;
			const vec2 relativeVelocity = velocity_ - other->velocity_;
			const float distSq = length2(relativePosition);
			const float combinedRadius = radius + other->radius;
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
			orcaLines.push_back(line);
		}

		size_t lineFail = linearProgram2(orcaLines, maxSpeed, prefVelocity, false, newVelocity_);

		if (lineFail < orcaLines.size()) 
		{
			linearProgram3(orcaLines, 0, lineFail, maxSpeed, newVelocity_);
		}
	}


	void Agent::update(float dt)
	{
		velocity_ = newVelocity_;
		position += velocity_ * dt;
	}

}
