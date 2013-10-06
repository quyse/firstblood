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

var MouseEvent = function(x, y)
{
	this.x = x;
	this.y = y;
	this.type = Event.MOUSE;
};

var KeyEvent = function(keyCode, isDown, isCtrlDown, isShiftDown)
{
	this.keyCode = keyCode;
	this.isDown = isDown;
	this.isUp = !isDown;
	this.isCtrlDown = isCtrlDown;
	this.isShiftDown = isShiftDown;
	this.type = Event.KEY;
};

this.Event = {
	// ids
	FRAME: "event_frame",
	MOUSE: "event_mouse",
	KEY: "event_key",
	APP_LOSE_FOCUS: "lose_focus",
	APP_GAIN_FOCUS: "gain_focus",
	
	// objects
	FrameEvent: FrameEvent,
	MouseEvent: MouseEvent,
	KeyEvent: KeyEvent
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
		var eventType = event.type;
		if (eventType in this.mapEventToSubscribersList)
		{
			var subscribers = this.mapEventToSubscribersList[eventType];
			for (var i = 0, l = subscribers.length; i < l; ++i)
				subscribers[i](event);
		}
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
	
	getTime: function() { return time.getTime(); }
};

this.setTimeout = function(closure, timeSpan) { return time.createTimer(closure, timeSpan, true); };
this.setInterval = function(closure, timeSpan) { return time.createTimer(closure, timeSpan, false); };
this.clearTimer = function(timerId) { time.destroyTimer(timerId); };
this.clearInterval = function(timerId) { time.destroyTimer(timerId); };