#include "Game.hpp"
#include "Painter.hpp"
#include "Geometry.hpp"
#include "geometry/intersections.hpp"
#include <random>
#include "profiler/scope_profiler.h"

static const float pi = acos(-1.0f);

static bool rvoExampleInitialized = false;

float badRandom(float a, float b)
{
	return a + (b - a) * (float)rand() / (float)RAND_MAX;
}

Game::Game()
{
	quadtree = new Spatial::Quadtree<QuadtreeDebugObject>(4, 32, 1024 * 1024);
	kdTree = new Spatial::KdTree<QuadtreeDebugObject>(4, 1024 * 1024);
	rvoSimulation = new RVO::Simulator(256);
}


Game::~Game()
{
	delete quadtree;
	delete rvoSimulation;
}


/*void Game::drawQuadtreeNode(Quadtree::Node* node)
{
	painter->DebugDrawRectangle(node->min.x, node->min.y, node->max.x, node->max.y, 0, vec3(0, 1, 0));
	for (size_t i = 0; i < 4; ++i)
	{
		if (node->children[i] != nullptr)
			drawQuadtreeNode(node->children[i]);
	}

	Spatial::EntityList<QuadtreeDebugObject>* s = node->inhabitants;
	while (s != nullptr)
	{
		vec2 center = s->getPosition();
		float radius = s->getRadius();
		painter->DebugDrawRectangle(center.x - radius, center.y - radius, center.x + radius, center.y + radius, 0.5, vec3(1.0f, 0.8f, 0.8f));
		s = s->next;
	}
}


void Game::drawKdTreeNode(KdTree::Node* node)
{
	painter->DebugDrawRectangle(node->min.x, node->min.y, node->max.x, node->max.y, 0, vec3(0, 1, 0));
	if (node->children[0] != nullptr)
		drawKdTreeNode(node->children[0]);
	if (node->children[1] != nullptr)
		drawKdTreeNode(node->children[1]);

	Spatial::EntityList<QuadtreeDebugObject>* s = node->inhabitants;
	while (s != nullptr)
	{
		vec2 center = s->getPosition();
		float radius = s->getRadius();
		painter->DebugDrawRectangle(center.x - radius, center.y - radius, center.x + radius, center.y + radius, 0.5, vec3(1.0f, 0.8f, 0.8f));
		s = s->next;
	}
}*/


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
		int numAgents = 16;
		bool addObstacle = true;
		rvoSimulation->setAgentDefaults(15.0f, 8, 15.0f, 1.5f, 2.0f);
		/*for (int i = 0; i < numAgents; ++i) 
		{
			RVO::Agent* agent = rvoSimulation->addAgent(vec2(std::cos(i * 2.0f * pi / numAgents + 0.01f), std::sin(i * 2.0f * pi / numAgents + 0.01f)) * 100.0f);
			agent->maxSpeed = 2.0f;
			agents.push_back(std::make_pair(agent, -agent->position));
		}
		for (int i = 0; i < numAgents; ++i) 
		{
			RVO::Agent* agent = rvoSimulation->addAgent(vec2(-150.0f, 6.0f * (i - 0.5f * numAgents)));
			agent->maxSpeed = 2.0f + 0.001f * i;
			agents.push_back(std::make_pair(agent, -agent->position));

			agent = rvoSimulation->addAgent(vec2(150.0f, 6.0f * (i - 0.5f * numAgents)));
			agent->maxSpeed = 2.0f + 0.001f * i;
			agents.push_back(std::make_pair(agent, -agent->position));
		}
		if (addObstacle)
		{
			RVO::Agent* agent = rvoSimulation->addAgent(vec2(0, 0), 20, 32, 10.0f, 12.0f, 0.0f);
			agent->immobilized = true;
			agents.push_back(std::make_pair(agent, -agent->position));
		}*/
		rvoExampleInitialized = true;
	}

	size_t l = rvoSimulation->getNumAgents();
	for (size_t i = 0; i < l; ++i) 
	{
		std::pair<RVO::Agent*, vec2> agent = agents[i];
		vec2 goalVector = agent.second - agent.first->position;
		if (length2(goalVector) > 1.0f) 
		{
			goalVector = normalize(goalVector);
		}
		agent.first->prefVelocity = goalVector;
	}

	if (rvoSimulation->getMaxAgents() > rvoSimulation->getNumAgents())
	{
		RVO::Agent* agent = rvoSimulation->addAgent(vec2(badRandom(0.0f, 1.0f) < 0.5f ? -25.0f : 25.0f, badRandom(-100.0f, 100.0f)));
		agent->maxSpeed = 2.0f;
		agents.push_back(std::make_pair(agent, -agent->position));
	}

	rvoSimulation->doStep(20 * frameTime);

	l = rvoSimulation->getNumAgents();
	for (size_t i = 0; i < l; i++)
	{
		RVO::Agent* agent = agents[i].first;
		vec2 hisGoal = agents[i].second;
		float distance = length(agent->position - hisGoal);
		if (!agent->immobilized && distance < 1.0f)
		{
			rvoSimulation->removeAgent(agent);
			agents[i] = agents[l - 1];
			agents.pop_back();
			--i;
			--l;
		}
	}

	float visualScale = 0.3f;
	for (size_t i = 0; i < l; ++i) 
	{
		RVO::Agent* agent = agents[i].first;
		vec2 agentPosition = agent->position;
		float agentRadius = agent->radius;
		painter->DebugDrawRectangle(
			visualScale * (agentPosition.x - agentRadius), visualScale * (agentPosition.y - agentRadius),
			visualScale * (agentPosition.x + agentRadius), visualScale * (agentPosition.y + agentRadius),
			0, vec3(1, 1, 1)
		);
	}

	std::vector<QuadtreeDebugObject> spheres;
	spheres.push_back(QuadtreeDebugObject(vec2(10, 10), 3));
	spheres.push_back(QuadtreeDebugObject(vec2(17, 17), 2));
	spheres.push_back(QuadtreeDebugObject(vec2(12, 17), 2));
	spheres.push_back(QuadtreeDebugObject(vec2(7, 17), 2));
	spheres.push_back(QuadtreeDebugObject(vec2(-8, 12), 5));
	spheres.push_back(QuadtreeDebugObject(vec2(0, -6), 4));
	spheres.push_back(QuadtreeDebugObject(vec2(-14, -13), 1));
	std::random_device rd0;
    std::minstd_rand gen0(657);
	std::random_device rd1;
    std::minstd_rand gen1(23);
    std::uniform_real_distribution<float> dis(-16.0f, 16.0f);
	for (size_t i = 0; i < 2000; ++i)
	{
		float x = dis(gen0);
		float y = dis(gen1);
		//spheres.push_back(QuadtreeDebugObject(vec2(x, y), 0.25));
	}

	quadtree->purge();
	quadtree->build(&(spheres[0]), spheres.size());
	quadtree->optimize();

	kdTree->purge();
	kdTree->build(&(spheres[0]), spheres.size());

	//drawKdTreeNode(kdTree->_root);

	//drawQuadtreeNode(quadtree->_root);

	// RAYCAST STRESS TEST
	/*std::vector<std::pair<vec2, vec2>> raycasts;
	for (size_t i = 0; i < 1000; ++i)
	{
		float x = dis(gen0);
		float y = dis(gen1);
		vec2 origin(x, y);
		x = dis(gen0);
		y = dis(gen1);
		vec2 end(x, y);
		raycasts.push_back(std::make_pair(origin, end));
	}

	{
		QuadtreeDebugObject* dudes[8];
		SCOPE_PROFILER( QUADTREE_RAYCAST );
		for (size_t i = 0; i < raycasts.size(); ++i)
		{

			float t;
			vec2& origin = raycasts[i].first;
			vec2& end = raycasts[i].second;

			//vec2 testPoint(2, 2);
			//float testDist = 4.0f;
			//size_t count = quadtree->getNeighbours(testPoint, testDist, 1, &(dudes[0]), 8); 

			quadtree->raycast(origin, end, 1, t);
		}
	}*/
	 
	// RAYCAST TEST
	/*vec2 origin(-20, 18);
	vec2 end(20, 18);
	float t;
	QuadtreeDebugObject* result = kdTree->raycast(origin, end, 1, t);
	if (result != nullptr)
	{
		vec2 center = result->center;
		float radius = result->radius;
		std::cout << t << std::endl;
		painter->DebugDrawRectangle(center.x - radius, center.y - radius, center.x + radius, center.y + radius, 1, vec3(1, 0, 0), 0.2f);
	}

	painter->DebugDrawLine(vec3(origin.x, origin.y, 0), vec3(end.x, end.y, 0), vec3(1, 0, 0));*/

	// NEAREST NEIGHBOURS TEST
	/*QuadtreeDebugObject* dudes[16];
	vec2 testPoint(6, 6);
	float testDist = 4.0f;
	size_t count = kdTree->getNeighbours(testPoint, testDist, 1, &(dudes[0]), 16); 
	painter->DebugDrawRectangle(testPoint.x - testDist, testPoint.y - testDist, testPoint.x + testDist, testPoint.y + testDist, 1, vec3(0, 0, 1), 0.2f);

	for (size_t i = 0; i < count; ++i)
	{
		vec2 center = dudes[i]->center;
		float radius = dudes[i]->radius;
		painter->DebugDrawRectangle(center.x - radius, center.y - radius, center.x + radius, center.y + radius, 2, vec3(1, 1, 0), 0.2f);		
	}*/
	Sleep(10);

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
