/** Standard events **/
var FrameEvent = function(dt)
{
	this.dt = dt;
	this.type = Event.FRAME;
}

this.Event = {
	// ids
	FRAME: "event_frame",
	
	// objects
	FrameEvent: FrameEvent
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

this.Engine = {
	Painter: engine.getPainter(),
	Rvo: engine.getRvoSimulation()
};