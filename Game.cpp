#include "Game.hpp"
#include "Painter.hpp"
#include "Geometry.hpp"

static const float pi = acos(-1.0f);

static bool rvoExampleInitialized = false;

void Game::Step(float frameTime)
{
#ifdef GL_DEBUG
	if (!rvoExampleInitialized)
	{
		rvoSimulation.setAgentDefaults(15.0f, 10, 10.0f, 10.0f, 1.5f, 2.0f);
		for (size_t i = 0; i < 125; ++i) 
		{
			rvoSimulation.addAgent(100.0f * RVO::Vector2(std::cos(i * 2.0f * pi / 125.0f), std::sin(i * 2.0f * pi / 125.0f)));
			goals.push_back(-rvoSimulation.getAgentPosition(i));
		}
		rvoExampleInitialized = true;
	}
	
	rvoSimulation.setTimeStep(0.1f);

	size_t l = rvoSimulation.getNumAgents();
	for (size_t i = 0; i < l; ++i) 
	{
		RVO::Vector2 goalVector = goals[i] - rvoSimulation.getAgentPosition(i);
		if (RVO::absSq(goalVector) > 1.0f) 
		{
			goalVector = RVO::normalize(goalVector);
		}
		rvoSimulation.setAgentPrefVelocity(i, goalVector);
	}

	rvoSimulation.doStep();

	float visualScale = 0.5f;
	for (size_t i = 0; i < l; ++i) 
	{
		RVO::Vector2 agentPosition = rvoSimulation.getAgentPosition(i);
		float agentRadius = rvoSimulation.getAgentRadius(i);
		painter->DebugDrawRectangle(
			visualScale * (agentPosition.x() - agentRadius), visualScale * (agentPosition.y() - agentRadius),
			visualScale * (agentPosition.x() + agentRadius), visualScale * (agentPosition.y() + agentRadius),
			0, vec3(1, 1, 1)
		);
	}

#else
	painter->DebugDrawRectangle(0, 0, 20, 20, 0, vec3(1, 1, 1));

	vec3 p[10];
	for(int i = 0; i < 10; ++i)
		p[i] = vec3(cos(pi * i * 0.8f), sin(pi * i * 0.8f), 0.0f);
	for(int h = 0; h < 10; ++h)
	{
		vec3 scale(10.0f - (float)h, 10.0f - (float)h, 1.0f);
		vec3 offset(10.0f, 10.0f, (float)h);
		vec3 color(1.0f, 1.0f - h * 0.05f, 1.0f - h * 0.05f);
		for(int i = 0; i < 10; ++i)
			painter->DebugDrawLine(p[i ? i - 1 : 9] * scale + offset, p[i] * scale + offset, color);
	}
#endif
}
