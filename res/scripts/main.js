var Main = function()
{
	this.agentsWithGoals = [];
	global.addListener(Event.FRAME, bind(this.update, this));
	Engine.Rvo.setAgentDefaults(15.0, 8, 15.0, 1.5, 2.0);
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
			
			var agentPosition = agent.getPosition();
			var dx = goal[0] - agentPosition[0];
			var dy = goal[1] - agentPosition[1];
			var length = Math.sqrt(dx * dx + dy * dy);
			if (length > 1.0) 
			{
				dx /= length;
				dy /= length;
			}
			agent.setPrefVelocity([dx, dy]);
		}

		if (Engine.Rvo.getMaxAgents() > Engine.Rvo.getNumAgents())
		{
			var x = badRandom(0.0, 1.0) < 0.5 ? -25.0 : 25.0
			var y = badRandom(-100.0, 100.0)
			var agent = Engine.Rvo.create([x, y]);
			agent.setMaxSpeed(2.0);
			this.agentsWithGoals.push([agent, [-x, -y]])
		}
		
		for (var i = 0, l = this.agentsWithGoals.length; i < l; ++i) 
		{
			var agent = this.agentsWithGoals[i][0];
			var hisGoal = this.agentsWithGoals[i][1];
	
			var agentPosition = agent.getPosition();
			var dx = hisGoal[0] - agentPosition[0];
			var dy = hisGoal[1] - agentPosition[1];
			var length = Math.sqrt(dx * dx + dy * dy);
			if (length < 1.0)
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
			var agentPosition = this.agentsWithGoals[i][0].getPosition();
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