require('math/vec2');
require('math/vec3');

require('gameplay/core');
require('gameplay/player');
require('gameplay/spawner')
require('injector');

require("citygen");

var Main = function()
{
	Engine.Painter.setGlobalScale(0.3);
	Engine.Rvo.setAgentDefaults(15.0, 8, 15.0, 1.5, 1.0);

	this.debugDrawer = new DebugDrawer();
	this.gameplayRegistry = new GameplayRegistry();


	this.injector = new Injector();
	this.injector.addDependency("GameplayRegistry", this.gameplayRegistry);
	this.injector.addDependency("DebugDrawer", this.debugDrawer);

	this.player = this.injector.create(Player);
	this.injector.addDependency("Player", this.player);

	this.spawner = this.injector.create(Spawner);

	global.addListener(Event.FRAME, bind(this.update, this));
	global.addListener(Event.KEYBOARD, bind(this.handleKeyEvent, this));
	global.addListener(Event.MOUSE_MOVE, bind(this.handleMouseMoveEvent, this));
	global.addListener(Event.MOUSE_BUTTON, bind(this.handleMouseButtonEvent, this));
	
	this.uidCounter = 1;
	this.agentsWithGoals = [];
	

	/*var cityGen = new CityGenerator(64, 64, 4, 4, 2.25, 7.5, 1.2);
	this.city = cityGen.generate();*/

	/*var agent = Engine.Rvo.create(vec2.fromValues(50, 40), ++this.uidCounter);
	this.agentsWithGoals.push([agent])
	agent = Engine.Rvo.create(vec2.fromValues(60, 80), ++this.uidCounter);
	this.agentsWithGoals.push([agent])
	agent = Engine.Rvo.create(vec2.fromValues(30, 40), ++this.uidCounter);
	this.agentsWithGoals.push([agent])*/
};

Main.prototype = {

	handleKeyEvent: function(event)
	{
		if (event.isDown && event.key == 32)
			this.cameraPosition[0] = this.cameraPosition[1] = 0;
	},
	
	handleMouseMoveEvent: function(event)
	{
		if (Engine.Input.isKeyDown(13) == true)
			log("Mr Jerry's coords are: ", Engine.Input.getCursorPosition(), ", he's running from Tom with speed: ", event.dx, event.dy);
	},
	
	handleMouseButtonEvent: function(event)
	{
		if (event.isLeftButton && event.isDown)
			log("Hey, easy, man, easy!");
	},

	update: function(event)
	{
		this.gameplayRegistry.update(event.dt);
		this.debugDrawer.update();

		Engine.Painter.drawLine(vec3.v(0, 0, 0), vec3.v(5, 0, 0), 0xff0000, 0.1);
		Engine.Painter.drawLine(vec3.v(0, 0, 0), vec3.v(0, 5, 0), 0x00ff00, 0.1);
		Engine.Painter.drawLine(vec3.v(0, 0, 0), vec3.v(0, 0, 5), 0x0000ff, 0.1);
		
		var position = this.player.getPosition();
		Engine.Camera.setLookAtLH(vec3.v(0.3 * position[0], 0.3 * position[1], 100), vec3.fromValues(0.000001, 0, -1), vec3.fromValues(0, 1, 1));
		//return;
		/*

		var start = vec3.fromValues(0, 0, 0);
		var end = vec3.fromValues(62, 82, 0);
		var entityId = Engine.Space.raycast(start, end, 1);
		Engine.Painter.drawLine(vec3.scale(start, visualScale), vec3.scale(end, visualScale), 0x00ff00, 0.1);*/
		//Engine.SpatialIndex.draw(visualScale);

		//log(this.city)
		/*for (var i = 0, l = this.city.length; i < l; ++i)
		{
			var block = this.city[i];
			var min = block[0];
			var max = block[1];
			Engine.Painter.drawAABB(min.concat(0), max.concat(5), 0xffffff)
		}

		return;*/

		/*function badRandom(a, b)
		{
			return a + (b - a) * Math.random()
		}

		for (var i = 0, l = this.agentsWithGoals.length; i < l; ++i) 
		{
			var agentWithGoal = this.agentsWithGoals[i];
			var agent = agentWithGoal[0];
			//Engine.Space.raycast(agent.getPosition(), vec3.create(0, 0, 0), 1, agent);
			var goal = agentWithGoal[1];
			var toGoal = vec2.sub(goal, agent.getPosition());
			agent.setPrefVelocity(vec2.scale(vec2.normalize(toGoal), agent.getMaxSpeed()));
		}

		if (128 > Engine.Rvo.getNumAgents())
		{
			var x = badRandom(0.0, 1.0) < 0.5 ? -100.0 : 100.0
			var y = badRandom(-100.0, 100.0)
			var agent = Engine.Rvo.create(vec2.fromValues(x, y), ++this.uidCounter);
			this.agentsWithGoals.push([agent, vec2.fromValues(-x, -y)])
		}
		
		for (var i = 0, l = this.agentsWithGoals.length; i < l; ++i) 
		{
			var agent = this.agentsWithGoals[i][0];
			var hisGoal = this.agentsWithGoals[i][1];
			var agentPosition = agent.getPosition();
			var toGoal = vec2.sub(hisGoal, agentPosition);
			if (vec2.len(toGoal) < 1.0)
			{
				Engine.Rvo.destroy(agent);
				this.agentsWithGoals[i] = this.agentsWithGoals[l - 1];
				this.agentsWithGoals.pop();
				--i;
				--l;
			}
		}

		
		for (var i = 0, l = this.agentsWithGoals.length; i < l; ++i) 
		{
			var agent = this.agentsWithGoals[i][0];
			var agentPosition = agent.getPosition();
			var x = agentPosition[0];
			var y = agentPosition[1];
			var radius = agent.getRadius();
			Engine.Painter.drawCircle([x, y, 0], radius, 0xffffff, 16);
		}*/
	}
}