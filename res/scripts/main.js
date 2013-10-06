if (!this.initialized)
{
	// set init flag and make a loop reference to global scope
	this.initialized = true;
	this.global = this;
	
	// receive core subsystems and utilities from engine
	var engine = Firstblood.Engine.getInstance()
	this.painter = engine.getPainter()
	this.logger = engine.getLogger()
	this.rvo = engine.getRvoSimulation()
	
	// import wrapper
	this.import = engine.import
	
	// fancy logging wrappers
	this.log = function(v) { global.logger.write("INFO: " + v) };
	this.logWarning = function(v) { global.logger.write("WARNING: " + v) };
	
	// script's int main() :)
	
	var Main = function()
	{
		this.agentsWithGoals = [];
		rvo.setAgentDefaults(15.0, 8, 15.0, 1.5, 2.0);
	};
	Main.prototype =
	{
		update: function()
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
				
				var agentX = agent.getX();
				var agentY = agent.getY();
				var dx = goal[0] - agentX;
				var dy = goal[1] - agentY;
				var length = Math.sqrt(dx * dx + dy * dy);
				if (length > 1.0) 
				{
					dx /= length;
					dy /= length;
				}
				agent.setPrefVelocity([dx, dy]);
			}

			if (rvo.getMaxAgents() > rvo.getNumAgents())
			{
				var x = badRandom(0.0, 1.0) < 0.5 ? -25.0 : 25.0
				var y = badRandom(-100.0, 100.0)
				var agent = rvo.create([x, y]);
				agent.setMaxSpeed(2.0);
				this.agentsWithGoals.push([agent, [-x, -y]])
			}
			
			for (var i = 0, l = this.agentsWithGoals.length; i < l; ++i) 
			{
				var agent = this.agentsWithGoals[i][0];
				var hisGoal = this.agentsWithGoals[i][1];

				var dx = hisGoal[0] - agent.getX();
				var dy = hisGoal[1] - agent.getY();
				var length = Math.sqrt(dx * dx + dy * dy);
				if (length < 1.0)
				{
					rvo.destroy(agent);
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
				var x = agent.getX();
				var y = agent.getY();
				var radius = agent.getRadius();
				painter.drawRect(
					[visualScale * (x - radius), visualScale * (y - radius)],
					[visualScale * (x + radius), visualScale * (y + radius)],
					0, 0xffffff, 0.1
				);
			}
		}
	}
	this.main = new Main();
}

(function()
{
	main.update();
})();