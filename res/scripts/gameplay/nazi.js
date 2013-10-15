require('math/vec2');

var Nazi = function(GameplayRegistry, Player, DebugDrawer) {};
Nazi.prototype = {

	init: function(position, spawner)
	{
		this.spawner = spawner;
		this.rvoAgent = Engine.Rvo.create(position, this.uid);
		this.rvoAgent.setMaxSpeed(0.5 + Math.random());
	},

	fini: function()
	{
		Engine.Rvo.destroy(this.rvoAgent);
		this.spawner.naziKilled();
	},

	update: function(dt)
	{
		if (this.alive)
		{
			var playerPosition = this.Player.getPosition();
			var prefVelocity = vec2.sub(playerPosition, this.rvoAgent.getPosition());
			this.rvoAgent.setPrefVelocity(prefVelocity);
		}
	},

	debugDraw: function()
	{
		var position = this.rvoAgent.getPosition();
		Engine.Painter.drawCircle(vec3.v(position[0], position[1], 0), this.rvoAgent.getRadius(), 0xff0000, 16);
	}

};