if (!this.initialized)
{
	var engine = Firstblood.Engine.getInstance();
	
	// global script logger
	this.logger = engine.getLogger();
	var logWithPrefix = function(prefix) 
	{  
		return function()
		{
			var pieces = [];
			for (var i = 0, l = arguments.length; i < l; ++i)
				pieces[i] = String(arguments[i]);
			var loggedString = "[" + prefix + "]" + ": " + pieces.join(" ");
			global.logger.write(loggedString);
		};
	};
	this.log = logWithPrefix("INFO");
	this.warning = logWithPrefix("WARNING");
	this.error = logWithPrefix("ERROR");
	this.debug = logWithPrefix("DEBUG");
	
	// import routine
	this.require = function(module)
	{
		engine.require(module)
	};
	
	// setup event dispatching for global scope
	require('stdlib');
	var engineDispatcher = new EventDispatcher();
	// either there is some fuck-up with global scope's prototype or I don't know a shit about js
	this.addListener = function(eventType, listener) { engineDispatcher.addListener(eventType, listener) };
	this.removeListener = function(eventType, listener) { engineDispatcher.removeListener(eventType, listener) };
	this.dispatch = function(event) { engineDispatcher.dispatch(event) };
	
	// setup MOUSE and KEY events dispatching
	var inputHandler = function(event)
	{
		var isMouseEvent = event[0];
		if (isMouseEvent)
		{
			return 0;
		}
		else
		{
			var details = event[1];
			var isDown = details[0];
			var keyCode = details[1];
			var result = global.dispatch(new Event.KeyEvent(keyCode, isDown));
			return result ? 1 : 0;
		}
	};
	Engine.Input.addListener(inputHandler);
	
	// we wanna be as cool, as browsers are
	this.global = this;

	// and away we go
	this.initialized = true;
	require("main");
	var main = new Main();
	this.prevTime = Engine.getTime();
}

// dispatch FRAME event
var timeNow = Engine.getTime();
var dt = timeNow - prevTime;
this.prevTime = timeNow;
this.dispatch(new Event.FrameEvent(dt));