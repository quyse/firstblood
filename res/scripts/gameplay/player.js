require('math/vec2');

var Player = function(GameplayRegistry, DebugDrawer) {};
Player.prototype = {

	init: function()
	{
		this.rvoAgent = Engine.Rvo.create(vec2.v(0, 0), this.uid);
		this.maxSpeed = 3;
		this.rvoAgent.setMaxSpeed(this.maxSpeed);
		this.rvoAgent.setTimeHorizon(0.1);
	},

	update: function(dt)
	{
		var input = Engine.Input;
		var vx = input.isKeyDown(65) ? -1 : (input.isKeyDown(68) ? 1 : 0);
		var vy = input.isKeyDown(83) ? -1 : (input.isKeyDown(87) ? 1 : 0);
		this.rvoAgent.setPrefVelocity(vec2.scale(vec2.v(vx, vy), this.maxSpeed));
	},

	getPosition: function()
	{
		return this.rvoAgent.getPosition();
	},

	debugDraw: function()
	{
		var position = this.rvoAgent.getPosition();
		Engine.Painter.drawCircle(vec3.v(position[0], position[1], 0), this.rvoAgent.getRadius(), 0xffffff, 16);
	}

};