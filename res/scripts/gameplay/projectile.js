require('math/vec3');

var Projectile = function(GameplayRegistry, DebugDrawer, Injector) {};
Projectile.prototype = {

	init: function(position, direction, speed, owner)
	{
		this.position = position;
		this.owner = owner;
		this.direction = direction;
		this.speed = speed;
		this.age = 0;
	},

	update: function(dt)
	{
		var futurePosition = vec3.add(this.position, vec3.scale(this.direction, this.speed));
		var collidedWith = Engine.Space.raycast(this.position, futurePosition, 1, this.owner)[0];
		var entity = this.GameplayRegistry.get(collidedWith);
		if (entity)
		{
			this.Injector.destroy(entity);
			this.Injector.destroy(this);
			return;
		}
		else
		{
			this.position = futurePosition;
			this.age += dt;
			if (this.age >= 2)
				this.Injector.destroy(this);
		}

	},

	debugDraw: function()
	{
		Engine.Painter.drawLine(this.position, vec3.add(this.position, vec3.scale(this.direction, 1)), 0xff6600, 0.1);
	}

}