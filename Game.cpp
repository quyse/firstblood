#include "Game.hpp"
#include "Painter.hpp"
#include "Geometry.hpp"

static const float pi = acos(-1.0f);

void Game::Step(float frameTime)
{
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
}
