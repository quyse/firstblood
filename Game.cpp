#include "Game.hpp"
#include "Painter.hpp"
#include "Geometry.hpp"
#include "geometry/intersections.hpp"
#include <random>
#include "profiler/scope_profiler.h"

Game::Game()
{
	// debug crap
	quadtree = new Spatial::Quadtree<QuadtreeDebugObject>(4, 32, 1024 * 1024);
	kdTree = new Spatial::KdTree<QuadtreeDebugObject>(4, 1024 * 1024);
}

Game::~Game()
{
	// debug crap
	delete quadtree;
	delete kdTree;
}

void Game::Step(float frameTime)
{
	// purge spatial index
	spatialIndex->purge();
	
	// collect spatial entities from all subsystems
	size_t entitiesCollected = 0;
	Firstblood::ISpatiallyIndexable* spatialEntities[1024];
	entitiesCollected += rvoSimulation->collectSpatialData(spatialEntities, 1024 - entitiesCollected);

	// build spatial index
	spatialIndex->build(spatialEntities, entitiesCollected);
	spatialIndex->optimize();

	// rvo simulation
	rvoSimulation->update(20 * frameTime);
	// run scripts 
	scripts->update(20 * frameTime);

	// do cleanup for each subsystem (for example, execute deferred script requests for objects' addition/removal)
	rvoSimulation->postUpdate();

	Thread::Sleep(10);
}
