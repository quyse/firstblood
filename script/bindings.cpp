#include "inanity/script/v8/State.hpp"
#include "inanity/script/Function.hpp"
#include "inanity/script/v8/impl.ipp"
#include "inanity/meta/impl.hpp"
#include "inanity/inanity-math-meta.ipp"

#include "script/system.hpp"
#include "script/utils.hpp"
#include "gamelogic/rvo.hpp"

/* RVO */
META_CLASS(Firstblood::RvoSimulation, Firstblood.RvoSimulation);
	META_METHOD(setAgentDefaults);
	META_METHOD(getNumAgents);
	META_METHOD(getMaxAgents);
	META_METHOD(create);
	META_METHOD(destroy);
META_CLASS_END();

META_CLASS(Firstblood::RvoAgent, Firstblood.RvoAgent);
	META_METHOD(setMaxSpeed);
	META_METHOD(setPrefVelocity);
	META_METHOD(getX);
	META_METHOD(getY);
	META_METHOD(getRadius);
META_CLASS_END();

/* UTILITIES */
META_CLASS(Firstblood::ScriptSystem, Firstblood.Engine);
	META_STATIC_METHOD(getInstance);
	META_METHOD(getLogger);
	META_METHOD(getPainter);
	META_METHOD(getRvoSimulation);
META_CLASS_END();

META_CLASS(Firstblood::ScriptLogger, Firstblood.Logger);
	META_METHOD(write);
META_CLASS_END();

META_CLASS(Firstblood::ScriptPainter, Firstblood.Painter)
	META_METHOD(drawLine);
	META_METHOD(drawAABB);
	META_METHOD(drawRect);
META_CLASS_END();