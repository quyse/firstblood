#include "Game.hpp"
#include "Painter.hpp"
#include "Geometry.hpp"
#include "geometry/intersections.hpp"

static const float pi = acos(-1.0f);

static bool rvoExampleInitialized = false;

Game::Game()
{
	quadtree = new Quadtree<QuadtreeDebugObject, Game>(32, 7, 1024 * 1024);
}


Game::~Game()
{
	delete quadtree;
}


void Game::drawQuadtreeNode(Quadtree<QuadtreeDebugObject, Game>::Node* node)
{
	painter->DebugDrawRectangle(node->min.x, node->min.y, node->max.x, node->max.y, 0, vec3(0, 1, 0));
	if (node->botLeft != nullptr)
		drawQuadtreeNode(node->botLeft);
	if (node->topLeft != nullptr)
		drawQuadtreeNode(node->topLeft);
	if (node->topRight != nullptr)
		drawQuadtreeNode(node->topRight);
	if (node->botRight != nullptr)
		drawQuadtreeNode(node->botRight);

	Quadtree<QuadtreeDebugObject, Game>::BoundingCircle* s = node->inhabitants;
	while (s != nullptr)
	{
		vec2 center = s->center;
		float radius = s->radius;
		painter->DebugDrawRectangle(center.x - radius, center.y - radius, center.x + radius, center.y + radius, 0, vec3(1.0f, 0.8f, 0.8f));
		s = s->next;
	}
}


void Game::Step(float frameTime)
{
#ifdef GL_DEBUG
	vec3 i0, i1;
	float tmin, tmax;
	bool answer = intersectSegmentAABB(vec3(-3, -3, -3), vec3(-2, -2, 0), vec3(-1, -1, -1), vec3(1, 1, 1), i0, i1, tmin, tmax);
	//std::cout << answer << " " << i0 << " " << i1 << "\n";
	answer = intersectSegmentSphere(vec3(0, 0, 0), vec3(5, 5, 5), vec3(2, 2, 2), 5.0f, i0, i1, tmin, tmax);
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

	/*float visualScale = 0.5f;
	for (size_t i = 0; i < l; ++i) 
	{
		RVO::Vector2 agentPosition = rvoSimulation.getAgentPosition(i);
		float agentRadius = rvoSimulation.getAgentRadius(i);
		painter->DebugDrawRectangle(
			visualScale * (agentPosition.x() - agentRadius), visualScale * (agentPosition.y() - agentRadius),
			visualScale * (agentPosition.x() + agentRadius), visualScale * (agentPosition.y() + agentRadius),
			0, vec3(1, 1, 1)
		);
	}*/
	quadtree->purge();

	std::vector<QuadtreeDebugObject*> spheres;
	spheres.push_back(new QuadtreeDebugObject(vec2(10, 10), 3));
	spheres.push_back(new QuadtreeDebugObject(vec2(17, 17), 2));
	spheres.push_back(new QuadtreeDebugObject(vec2(12, 17), 2));
	spheres.push_back(new QuadtreeDebugObject(vec2(7, 17), 2));
	spheres.push_back(new QuadtreeDebugObject(vec2(-8, 12), 5));
	spheres.push_back(new QuadtreeDebugObject(vec2(0, -6), 4));
	spheres.push_back(new QuadtreeDebugObject(vec2(-14, -13), 1));

	for (size_t i = 0; i < spheres.size(); ++i)
		quadtree->addBoundingCircle(spheres[i]->center, spheres[i]->radius, 1, spheres[i]);

	drawQuadtreeNode(quadtree->_root);

	vec2 origin(10, 10);
	vec2 end(-20, -20);
	float t;
	QuadtreeDebugObject* result = quadtree->raycast(origin, end, 1, t);
	if (result != nullptr)
	{
		vec2 center = result->center;
		float radius = result->radius;
		std::cout << t << std::endl;
		painter->DebugDrawRectangle(center.x - radius, center.y - radius, center.x + radius, center.y + radius, 1, vec3(1, 0, 0), 0.2f);
	}

	painter->DebugDrawLine(vec3(origin.x, origin.y, 0), vec3(end.x, end.y, 0), vec3(1, 0, 0));

	QuadtreeDebugObject* dudes[4];
	vec2 testPoint(2, 2);
	float testDist = 24.0f;
	size_t count = quadtree->getNeighbours(testPoint, testDist, 1, &(dudes[0]), 4); 
	painter->DebugDrawRectangle(testPoint.x - testDist, testPoint.y - testDist, testPoint.x + testDist, testPoint.y + testDist, 1, vec3(0, 0, 1), 0.2f);
	std::cout << "Count is " << count << std::endl;

	for (size_t i = 0; i < count; ++i)
	{
		vec2 center = dudes[i]->center;
		float radius = dudes[i]->radius;
		painter->DebugDrawRectangle(center.x - radius, center.y - radius, center.x + radius, center.y + radius, 2, vec3(1, 1, 0), 0.2f);		
	}

	for (size_t i = 0; i < spheres.size(); ++i)
		delete spheres[i];

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

	painter->DebugDrawAABB(vec3(-10, -10, -10), vec3(10, 10, 10), vec3(0, 1, 0));
	painter->DebugDrawAABB(vec3(9, 9, 9), vec3(11, 11, 11), vec3(0, 0, 1));

#endif
}
