require('math/vec2');
require('gameplay/projectile');

var Player = function(GameplayRegistry, DebugDrawer, Injector) {};
Player.prototype = {

	init: function()
	{
		this.rvoAgent = Engine.Rvo.create(vec2.v(0, 0), this.uid);
		this.maxSpeed = 2;
		this.rvoAgent.setMaxSpeed(this.maxSpeed);
		this.rvoAgent.setTimeHorizon(0.1);

		this.weaponRecharge = 0;
		this.playerPressesFire = false;
		global.addListener(Event.MOUSE_BUTTON, this.keyboardListener = bind(this.handleKeyEvent, this));
	},

	fini: function()
	{
		global.removeListener(Event.MOUSE_BUTTON, this.keyboardListener);
	},

	handleKeyEvent: function(event)
	{
		if (event.isLeftButton)
			this.playerPressesFire = event.isDown;
	},

	update: function(dt)
	{
		var input = Engine.Input;
		var vx = input.isKeyDown(65) ? -1 : (input.isKeyDown(68) ? 1 : 0);
		var vy = input.isKeyDown(83) ? -1 : (input.isKeyDown(87) ? 1 : 0);
		this.rvoAgent.setPrefVelocity(vec2.scale(vec2.v(vx, vy), this.maxSpeed));

		if (this.weaponRecharge <= 0)
		{
			this.weaponRecharge = 0.05;
			if (this.playerPressesFire)
			{
				cursor = Engine.Input.getCursorPosition();
				var dx = cursor[0] - 505;
				var dy = 282 - cursor[1];
				var angle = Math.atan2(dy, dx);
				angle += 0.12 * (1 - 2 * Math.random());
				direction = vec3.normalize(vec2.to3(vec2.v(Math.cos(angle), Math.sin(angle)), 0));
				this.Injector.create(Projectile, vec2.to3(this.rvoAgent.getPosition(), 0), direction, 5, this.rvoAgent);
			}
		}
		else
		{
			this.weaponRecharge -= dt;
		}
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