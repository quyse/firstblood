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
	
	// we wanna be as cool, as browsers are
	this.global = this;

	// and away we go
	this.initialized = true;
	require("main");
	var main = new Main();
}

// todo: retrieve frame time and mouse/keyboard events from the engine
this.dispatch(new Event.FrameEvent(0));