#include "rvo/math.hpp"

namespace RVO
{

	bool linearProgram1(Line (&lines)[MAX_ORCA_LINES], size_t linesCount, size_t lineNo, float radius, const vec2 &optVelocity, bool directionOpt, vec2 &result)
	{
		const float dotProduct = dot(lines[lineNo].point, lines[lineNo].direction);
		const float discriminant = sqr(dotProduct) + sqr(radius) - length2(lines[lineNo].point);

		if (discriminant < 0.0f) {
			/* Max speed circle fully invalidates line lineNo. */
			return false;
		}

		const float sqrtDiscriminant = std::sqrt(discriminant);
		float tLeft = -dotProduct - sqrtDiscriminant;
		float tRight = -dotProduct + sqrtDiscriminant;

		for (size_t i = 0; i < lineNo; ++i) {
			const float denominator = det(lines[lineNo].direction, lines[i].direction);
			const float numerator = det(lines[i].direction, lines[lineNo].point - lines[i].point);

			if (std::fabs(denominator) <= EPSILON) {
				/* Lines lineNo and i are (almost) parallel. */
				if (numerator < 0.0f) {
					return false;
				}
				else {
					continue;
				}
			}

			const float t = numerator / denominator;

			if (denominator >= 0.0f) {
				/* Line i bounds line lineNo on the right. */
				tRight = std::min(tRight, t);
			}
			else {
				/* Line i bounds line lineNo on the left. */
				tLeft = std::max(tLeft, t);
			}

			if (tLeft > tRight) {
				return false;
			}
		}

		if (directionOpt) {
			/* Optimize direction. */
			if (dot(optVelocity, lines[lineNo].direction) > 0.0f) {
				/* Take right extreme. */
				result = lines[lineNo].point + lines[lineNo].direction * tRight;
			}
			else {
				/* Take left extreme. */
				result = lines[lineNo].point +  lines[lineNo].direction * tLeft;
			}
		}
		else {
			/* Optimize closest point. */
			const float t = dot(lines[lineNo].direction, (optVelocity - lines[lineNo].point));

			if (t < tLeft) {
				result = lines[lineNo].point +  lines[lineNo].direction * tLeft;
			}
			else if (t > tRight) {
				result = lines[lineNo].point +  lines[lineNo].direction * tRight;
			}
			else {
				result = lines[lineNo].point + lines[lineNo].direction * t;
			}
		}

		return true;
	}

	size_t linearProgram2(Line (&lines)[MAX_ORCA_LINES], size_t linesCount, float radius, const vec2 &optVelocity, bool directionOpt, vec2 &result)
	{
		if (directionOpt) {
			/*
			 * Optimize direction. Note that the optimization velocity is of unit
			 * length in this case.
			 */
			result = optVelocity * radius;
		}
		else if (length2(optVelocity) > sqr(radius)) {
			/* Optimize closest point and outside circle. */
			result = normalize(optVelocity) * radius;
		}
		else {
			/* Optimize closest point and inside circle. */
			result = optVelocity;
		}

		for (size_t i = 0; i < linesCount; ++i) {
			if (det(lines[i].direction, lines[i].point - result) > 0.0f) {
				/* Result does not satisfy constraint i. Compute new optimal result. */
				const vec2 tempResult = result;

				if (!linearProgram1(lines, linesCount, i, radius, optVelocity, directionOpt, result)) {
					result = tempResult;
					return i;
				}
			}
		}

		return linesCount;
	}

	// todo: remove obstacles-related parts
	// todo: get rid of std::vector
	void linearProgram3(Line (&lines)[MAX_ORCA_LINES], size_t linesCount, size_t beginLine, float radius, vec2 &result)
	{
		float distance = 0.0f;

		for (size_t i = beginLine; i < linesCount; ++i) {
			if (det(lines[i].direction, lines[i].point - result) > distance) {
				/* Result does not satisfy constraint of line i. */
				Line projLines[MAX_ORCA_LINES];
				size_t projLinesCount = 0;

				for (size_t j = 0; j < i; ++j) {
					Line line;

					float determinant = det(lines[i].direction, lines[j].direction);

					if (std::fabs(determinant) <= EPSILON) {
						/* Line i and line j are parallel. */
						if (dot(lines[i].direction, lines[j].direction) > 0.0f) {
							/* Line i and line j point in the same direction. */
							continue;
						}
						else {
							/* Line i and line j point in opposite direction. */
							line.point = (lines[i].point + lines[j].point) * 0.5f;
						}
					}
					else {
						line.point = lines[i].point + lines[i].direction * (det(lines[j].direction, lines[i].point - lines[j].point) / determinant);
					}

					line.direction = normalize(lines[j].direction - lines[i].direction);
					projLines[projLinesCount++] = line;
				}

				const vec2 tempResult = result;

				if (linearProgram2(projLines, projLinesCount, radius, vec2(-lines[i].direction.y, lines[i].direction.x), true, result) < projLinesCount) {
					/* This should in principle not happen.  The result is by definition
					 * already in the feasible region of this linear program. If it fails,
					 * it is due to small floating point error, and the current result is
					 * kept.
					 */
					result = tempResult;
				}

				distance = det(lines[i].direction, lines[i].point - result);
			}
		}
	}

}