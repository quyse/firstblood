require("math/vec2");
require("math/vec3");

var Main = function()
{
	Engine.Camera.setLookAtLH(vec3.fromValues(0, 0, 100.0), vec3.fromValues(0.000001, 0, -1), vec3.fromValues(0, 1, 1))

	this.agentsWithGoals = [];
	global.addListener(Event.FRAME, bind(this.update, this));
	Engine.Rvo.setAgentDefaults(15.0, 8, 15.0, 1.5, 1.0);
};

Main.prototype = {
	update: function(event)
	{
		function badRandom(a, b)
		{
			return a + (b - a) * Math.random()
		}

		for (var i = 0, l = this.agentsWithGoals.length; i < l; ++i) 
		{
			var agentWithGoal = this.agentsWithGoals[i];
			var agent = agentWithGoal[0];
			var goal = agentWithGoal[1];
			var toGoal = vec2.sub(goal, agent.getPosition());
			agent.setPrefVelocity(vec2.scale(vec2.normalize(toGoal), agent.getMaxSpeed()));
		}

		if (Engine.Rvo.getMaxAgents() > Engine.Rvo.getNumAgents())
		{
			var x = badRandom(0.0, 1.0) < 0.5 ? -25.0 : 25.0
			var y = badRandom(-100.0, 100.0)
			var agent = Engine.Rvo.create(vec2.fromValues(x, y));
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

		var visualScale = 0.3;
		for (var i = 0, l = this.agentsWithGoals.length; i < l; ++i) 
		{
			var agent = this.agentsWithGoals[i][0];
			var agentPosition = agent.getPosition();
			var x = agentPosition[0];
			var y = agentPosition[1];
			var radius = agent.getRadius();
			Engine.Painter.drawRect(
				[visualScale * (x - radius), visualScale * (y - radius)],
				[visualScale * (x + radius), visualScale * (y + radius)],
				0, 0xffffff, 0.1
			);
		}
	}
}