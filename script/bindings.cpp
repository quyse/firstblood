#include "inanity/script/v8/impl.ipp"
#include "inanity/inanity-base-meta.ipp"
#include "inanity/inanity-math-script.ipp"

#include "script/system.hpp"
#include "script/utils.hpp"
#include "script/time.hpp"
#include "script/camera.hpp"
#include "script/input.hpp"
#include "script/spatial.hpp"
#include "gamelogic/rvo.hpp"

/* SPATIAL */
META_CLASS(Firstblood::ScriptSpatialIndex, Firstblood.Space);
	META_METHOD(raycast);
	META_METHOD(getNeighbors);
	META_METHOD(draw);
META_CLASS_END();

/* INPUT */
META_CLASS(Firstblood::ScriptInput, Firstblood.Input)
	META_METHOD(addListener);
	META_METHOD(removeListener);
	META_METHOD(isKeyDown);
	META_METHOD(getCursorPosition);
META_CLASS_END();

/* CAMERA */
META_CLASS(Firstblood::ScriptCamera, Firstblood.Camera);
	META_METHOD(setLookAtLH);
META_CLASS_END();

/* TIME */
META_CLASS(Firstblood::ScriptTime, Firstblood.Time);
	META_METHOD(getTime);
	META_METHOD(createTimer);
	META_METHOD(destroyTimer);
META_CLASS_END();

/* RVO */
META_CLASS(Firstblood::RvoSimulation, Firstblood.RvoSimulation);
	META_METHOD(setAgentDefaults);
	META_METHOD(getNumAgents);
	META_METHOD(getMaxAgents);
	META_METHOD(create);
	META_METHOD(destroy);
META_CLASS_END();

META_CLASS(Firstblood::RvoAgent, Firstblood.RvoAgent);
	META_METHOD(getMask);
	META_METHOD(setMask);
	META_METHOD(setMaxNeighbors);
	META_METHOD(setImmobilized);
	META_METHOD(setTimeHorizon);
	META_METHOD(getMaxSpeed);
	META_METHOD(setMaxSpeed);
	META_METHOD(setPrefVelocity);
	META_METHOD(getPosition);
	META_METHOD(getRadius);
META_CLASS_END();

/* UTILITIES */
META_CLASS(Firstblood::ScriptSystem, Firstblood.Engine);
	META_STATIC_METHOD(getInstance);
	META_METHOD(getLogger);
	META_METHOD(getPainter);
	META_METHOD(getRvoSimulation);
	META_METHOD(getTime);
	META_METHOD(getCamera);
	META_METHOD(getInput);
	META_METHOD(getSpatialIndex);
	META_METHOD(require);
META_CLASS_END();

META_CLASS(Firstblood::ScriptLogger, Firstblood.Logger);
	META_METHOD(write);
META_CLASS_END();

META_CLASS(Firstblood::ScriptPainter, Firstblood.Painter)
	META_METHOD(drawLine);
	META_METHOD(drawAABB);
	META_METHOD(drawRect);
	META_METHOD(drawCircle);
	META_METHOD(setGlobalScale);
META_CLASS_END();
