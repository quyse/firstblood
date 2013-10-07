/** Standard events **/
var GenericEvent = function(type)
{
	this.type = type;
};

var FrameEvent = function(dt)
{
	this.dt = dt;
	this.type = Event.FRAME;
};

var MouseMoveEvent = function(dx, dy)
{
	this.dx = dx;
	this.dy = dy;
	this.type = Event.MOUSE_MOVE;
};

var MouseWheelEvent = function(dz)
{
	this.dz = dz;
	this.type = Event.MOUSE_WHEEL;
};

var MouseButtonEvent = function(button, isDown)
{
	this.isLeftButton = button == 0;
	this.isRightButtpn = button == 1;
	this.isMiddleButton = button == 2;
	this.isDown = isDown;
	this.isUp = !isDown;
	this.type = Event.MOUSE_BUTTON;
};

var KeyboardEvent = function(keyCode, isDown)
{
	this.key = keyCode;
	this.isDown = isDown;
	this.isUp = !isDown;
	this.type = Event.KEYBOARD;
};

this.Event = {
	// ids
	FRAME: "event_frame",
	MOUSE_MOVE: "event_mouse_move",
	MOUSE_BUTTON: "event_mouse_button",
	MOUSE_WHEEL: "event_mouse_wheel",
	KEYBOARD: "event_key",
	APP_LOSE_FOCUS: "lose_focus",
	APP_GAIN_FOCUS: "gain_focus",
	
	// objects
	FrameEvent: FrameEvent,
	MouseMoveEvent: MouseMoveEvent,
	MouseWheelEvent: MouseWheelEvent,
	MouseButtonEvent: MouseButtonEvent,
	KeyboardEvent: KeyboardEvent
};


/** General utilities **/
this.bind = function(method, scope)
{
	return function()
	{
		return method.apply(scope, arguments);
	}	
};


/** Event dispatcher **/
this.EventDispatcher = function()
{
	this.mapEventToSubscribersList = {};
};

EventDispatcher.prototype = {
	
	dispatch: function(event)
	{
		var result = false;
		var eventType = event.type;
		if (eventType in this.mapEventToSubscribersList)
		{
			var subscribers = this.mapEventToSubscribersList[eventType];
			for (var i = 0, l = subscribers.length; i < l; ++i)
			{
				var handled = subscribers[i](event);
				result = result || handled;
			}
		}
		return result;
	},
	
	addListener: function(eventType, listener)
	{
		if (!(eventType in this.mapEventToSubscribersList))
			this.mapEventToSubscribersList[eventType] = [];
		this.mapEventToSubscribersList[eventType].push(listener);
	},
	
	removeListener: function(eventType, listener)
	{
		if (!(eventType in this.mapEventToSubscribersList))
		{
			warning("EventDispatcher::removeListener -- non-existent event type: ", eventType);
		}
		else
		{
			var subscribers = this.mapEventToSubscribersList[eventType];
			var index = subscribers.indexOf(listener);
			if (index < 0)
			{
				warning("EventDispatcher::listener no such listener registered: ", listener, " for event ", eventType);
			}
			else
			{
				subscribers.splice(index, 1);
			}
		}
	}
	
};


/** Engine-provided subsystems and utilities **/
var engine = Firstblood.Engine.getInstance();
var time = engine.getTime();

this.Engine = {
	Painter: engine.getPainter(),
	Rvo: engine.getRvoSimulation(),
	Camera: engine.getCamera(),
	Input: engine.getInput(),
	
	getTime: function() { return time.getTime(); }
};

this.setTimeout = function(closure, timeSpan) { return time.createTimer(closure, timeSpan, true); };
this.setInterval = function(closure, timeSpan) { return time.createTimer(closure, timeSpan, false); };
this.clearTimer = function(timerId) { time.destroyTimer(timerId); };
this.clearInterval = function(timerId) { time.destroyTimer(timerId); };