#include "rvo/agent.hpp"
#include "rvo/kd_tree.hpp"
#include "rvo/obstacle.hpp"
#include "rvo/simulator.hpp"
#include "geometry/distance.hpp"

namespace RVO {
	Agent::Agent() : maxNeighbors(0), maxSpeed(0.0f), neighborDist(0.0f), radius(0.0f), timeHorizon(0.0f), timeHorizonObst(0.0f), isStatic(false) { }

	void Agent::computeNeighbors(KdTree* spatialIndex)
	{
		float rangeSq = sqr(timeHorizonObst * maxSpeed + radius);
		spatialIndex->computeObstacleNeighbors(this, rangeSq);

		if (maxNeighbors > 0) {
			rangeSq = sqr(neighborDist);
			spatialIndex->computeAgentNeighbors(this, rangeSq);
		}
	}

	/* Search for the best new velocity. */
	void Agent::computeNewVelocity(float dt)
	{
		std::vector<Line> orcaLines;
		const float invTimeHorizonObst = 1.0f / timeHorizonObst;

		/* Create obstacle ORCA lines. */
		for (size_t i = 0; i < obstacleNeighbors_->size(); ++i) {

			const Obstacle *obstacle1 = (*obstacleNeighbors_)[i].second;
			const Obstacle *obstacle2 = obstacle1->nextObstacle_;

			const vec2 relativePosition1 = obstacle1->point_ - position;
			const vec2 relativePosition2 = obstacle2->point_ - position;

			/*
			 * Check if velocity obstacle of obstacle is already taken care of by
			 * previously constructed obstacle ORCA lines.
			 */
			bool alreadyCovered = false;

			for (size_t j = 0; j < orcaLines.size(); ++j) {
				if (det(relativePosition1 * invTimeHorizonObst - orcaLines[j].point, orcaLines[j].direction) - invTimeHorizonObst * radius >= -EPSILON && det(relativePosition2 * invTimeHorizonObst  - orcaLines[j].point, orcaLines[j].direction) - invTimeHorizonObst * radius >=  -EPSILON) {
					alreadyCovered = true;
					break;
				}
			}

			if (alreadyCovered) {
				continue;
			}

			/* Not yet covered. Check for collisions. */

			const float distSq1 = length2(relativePosition1);
			const float distSq2 = length2(relativePosition2);

			const float radiusSq = sqr(radius);

			const vec2 obstacleVector = obstacle2->point_ - obstacle1->point_;
			const float s = (-dot(relativePosition1, obstacleVector)) / length2(obstacleVector);
			const float distSqLine = length2(-relativePosition1 - obstacleVector * s);

			Line line;

			if (s < 0.0f && distSq1 <= radiusSq) {
				/* Collision with left vertex. Ignore if non-convex. */
				if (obstacle1->isConvex_) {
					line.point = vec2(0.0f, 0.0f);
					line.direction = normalize(vec2(-relativePosition1.y, relativePosition1.x));
					orcaLines.push_back(line);
				}

				continue;
			}
			else if (s > 1.0f && distSq2 <= radiusSq) {
				/* Collision with right vertex. Ignore if non-convex
				 * or if it will be taken care of by neighoring obstace */
				if (obstacle2->isConvex_ && det(relativePosition2, obstacle2->unitDir_) >= 0.0f) {
					line.point = vec2(0.0f, 0.0f);
					line.direction = normalize(vec2(-relativePosition2.y, relativePosition2.x));
					orcaLines.push_back(line);
				}

				continue;
			}
			else if (s >= 0.0f && s < 1.0f && distSqLine <= radiusSq) {
				/* Collision with obstacle segment. */
				line.point = vec2(0.0f, 0.0f);
				line.direction = -obstacle1->unitDir_;
				orcaLines.push_back(line);
				continue;
			}

			/*
			 * No collision.
			 * Compute legs. When obliquely viewed, both legs can come from a single
			 * vertex. Legs extend cut-off line when nonconvex vertex.
			 */

			vec2 leftLegDirection, rightLegDirection;

			if (s < 0.0f && distSqLine <= radiusSq) {
				/*
				 * Obstacle viewed obliquely so that left vertex
				 * defines velocity obstacle.
				 */
				if (!obstacle1->isConvex_) {
					/* Ignore obstacle. */
					continue;
				}

				obstacle2 = obstacle1;

				const float leg1 = std::sqrt(distSq1 - radiusSq);
				leftLegDirection = vec2(relativePosition1.x * leg1 - relativePosition1.y * radius, relativePosition1.x * radius + relativePosition1.y * leg1) / distSq1;
				rightLegDirection = vec2(relativePosition1.x * leg1 + relativePosition1.y * radius, -relativePosition1.x * radius + relativePosition1.y * leg1) / distSq1;
			}
			else if (s > 1.0f && distSqLine <= radiusSq) {
				/*
				 * Obstacle viewed obliquely so that
				 * right vertex defines velocity obstacle.
				 */
				if (!obstacle2->isConvex_) {
					/* Ignore obstacle. */
					continue;
				}

				obstacle1 = obstacle2;

				const float leg2 = std::sqrt(distSq2 - radiusSq);
				leftLegDirection = vec2(relativePosition2.x * leg2 - relativePosition2.y * radius, relativePosition2.x * radius + relativePosition2.y * leg2) / distSq2;
				rightLegDirection = vec2(relativePosition2.x * leg2 + relativePosition2.y * radius, -relativePosition2.x * radius + relativePosition2.y * leg2) / distSq2;
			}
			else {
				/* Usual situation. */
				if (obstacle1->isConvex_) {
					const float leg1 = std::sqrt(distSq1 - radiusSq);
					leftLegDirection = vec2(relativePosition1.x * leg1 - relativePosition1.y * radius, relativePosition1.x * radius + relativePosition1.y * leg1) / distSq1;
				}
				else {
					/* Left vertex non-convex; left leg extends cut-off line. */
					leftLegDirection = -obstacle1->unitDir_;
				}

				if (obstacle2->isConvex_) {
					const float leg2 = std::sqrt(distSq2 - radiusSq);
					rightLegDirection = vec2(relativePosition2.x * leg2 + relativePosition2.y * radius, -relativePosition2.x * radius + relativePosition2.y * leg2) / distSq2;
				}
				else {
					/* Right vertex non-convex; right leg extends cut-off line. */
					rightLegDirection = obstacle1->unitDir_;
				}
			}

			/*
			 * Legs can never point into neighboring edge when convex vertex,
			 * take cutoff-line of neighboring edge instead. If velocity projected on
			 * "foreign" leg, no constraint is added.
			 */

			const Obstacle *const leftNeighbor = obstacle1->prevObstacle_;

			bool isLeftLegForeign = false;
			bool isRightLegForeign = false;

			if (obstacle1->isConvex_ && det(leftLegDirection, -leftNeighbor->unitDir_) >= 0.0f) {
				/* Left leg points into obstacle. */
				leftLegDirection = -leftNeighbor->unitDir_;
				isLeftLegForeign = true;
			}

			if (obstacle2->isConvex_ && det(rightLegDirection, obstacle2->unitDir_) <= 0.0f) {
				/* Right leg points into obstacle. */
				rightLegDirection = obstacle2->unitDir_;
				isRightLegForeign = true;
			}

			/* Compute cut-off centers. */
			const vec2 leftCutoff =  (obstacle1->point_ - position) * invTimeHorizonObst;
			const vec2 rightCutoff = (obstacle2->point_ - position) * invTimeHorizonObst;
			const vec2 cutoffVec = rightCutoff - leftCutoff;

			/* Project current velocity on velocity obstacle. */

			/* Check if current velocity is projected on cutoff circles. */
			const float t = (obstacle1 == obstacle2 ? 0.5f : (dot((velocity_ - leftCutoff), cutoffVec)) / length2(cutoffVec));
			const float tLeft = dot((velocity_ - leftCutoff), leftLegDirection);
			const float tRight = dot((velocity_ - rightCutoff), rightLegDirection);

			if ((t < 0.0f && tLeft < 0.0f) || (obstacle1 == obstacle2 && tLeft < 0.0f && tRight < 0.0f)) {
				/* Project on left cut-off circle. */
				const vec2 unitW = normalize(velocity_ - leftCutoff);

				line.direction = vec2(unitW.y, -unitW.x);
				line.point = leftCutoff +  unitW * radius * invTimeHorizonObst;
				orcaLines.push_back(line);
				continue;
			}
			else if (t > 1.0f && tRight < 0.0f) {
				/* Project on right cut-off circle. */
				const vec2 unitW = normalize(velocity_ - rightCutoff);

				line.direction = vec2(unitW.y, -unitW.x);
				line.point = rightCutoff + unitW * radius * invTimeHorizonObst;
				orcaLines.push_back(line);
				continue;
			}

			/*
			 * Project on left leg, right leg, or cut-off line, whichever is closest
			 * to velocity.
			 */
			const float distSqCutoff = ((t < 0.0f || t > 1.0f || obstacle1 == obstacle2) ? std::numeric_limits<float>::infinity() : length2(velocity_ - (leftCutoff + cutoffVec * t)));
			const float distSqLeft = ((tLeft < 0.0f) ? std::numeric_limits<float>::infinity() : length2(velocity_ - (leftCutoff +  leftLegDirection * tLeft)));
			const float distSqRight = ((tRight < 0.0f) ? std::numeric_limits<float>::infinity() : length2(velocity_ - (rightCutoff +  rightLegDirection * tRight)));

			if (distSqCutoff <= distSqLeft && distSqCutoff <= distSqRight) {
				/* Project on cut-off line. */
				line.direction = -obstacle1->unitDir_;
				line.point = leftCutoff + vec2(-line.direction.y, line.direction.x) * radius * invTimeHorizonObst;
				orcaLines.push_back(line);
				continue;
			}
			else if (distSqLeft <= distSqRight) {
				/* Project on left leg. */
				if (isLeftLegForeign) {
					continue;
				}

				line.direction = leftLegDirection;
				line.point = leftCutoff + vec2(-line.direction.y, line.direction.x) * radius * invTimeHorizonObst;
				orcaLines.push_back(line);
				continue;
			}
			else {
				/* Project on right leg. */
				if (isRightLegForeign) {
					continue;
				}

				line.direction = -rightLegDirection;
				line.point = rightCutoff + vec2(-line.direction.y, line.direction.x) * radius * invTimeHorizonObst;
				orcaLines.push_back(line);
				continue;
			}
		}

		const size_t numObstLines = orcaLines.size();

		const float invTimeHorizon = 1.0f / timeHorizon;

		/* Create agent ORCA lines. */
		for (size_t i = 0; i < agentNeighbors_->size(); ++i) {
			const Agent *const other = (*agentNeighbors_)[i].second;

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

		if (lineFail < orcaLines.size()) {
			linearProgram3(orcaLines, numObstLines, lineFail, maxSpeed, newVelocity_);
		}
	}

	void Agent::insertAgentNeighbor(const Agent *agent, float &rangeSq)
	{
		if (this != agent) {
			const float dist = std::max(0.0f, length(position - agent->position) - radius - agent->radius);
			const float distSq = dist * dist;

			if (distSq < rangeSq) {
				if (agentNeighbors_->size() < maxNeighbors) {
					agentNeighbors_->push_back(std::make_pair(distSq, agent));
				}

				size_t i = agentNeighbors_->size() - 1;

				while (i != 0 && distSq < (*agentNeighbors_)[i - 1].first) {
					(*agentNeighbors_)[i] = (*agentNeighbors_)[i - 1];
					--i;
				}

				(*agentNeighbors_)[i] = std::make_pair(distSq, agent);

				if (agentNeighbors_->size() == maxNeighbors) {
					rangeSq = agentNeighbors_->back().first;
				}
			}
		}
	}

	void Agent::insertObstacleNeighbor(const Obstacle *obstacle, float rangeSq)
	{
		const Obstacle *const nextObstacle = obstacle->nextObstacle_;

		const float distSq = distanceSquaredPointSegment(obstacle->point_, nextObstacle->point_, position);

		if (distSq < rangeSq) {
			obstacleNeighbors_->push_back(std::make_pair(distSq, obstacle));

			size_t i = obstacleNeighbors_->size() - 1;

			while (i != 0 && distSq < (*obstacleNeighbors_)[i - 1].first) {
				(*obstacleNeighbors_)[i] = (*obstacleNeighbors_)[i - 1];
				--i;
			}

			(*obstacleNeighbors_)[i] = std::make_pair(distSq, obstacle);
		}
	}

	void Agent::update(float dt)
	{
		velocity_ = newVelocity_;
		position += velocity_ * dt;
	}

}
