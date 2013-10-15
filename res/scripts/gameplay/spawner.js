require('math/vec2');
require('gameplay/nazi');

var Spawner = function(GameplayRegistry, Injector) {};
Spawner.prototype = {

	init: function() 
	{
		this.nazisCount = 0;
	},

	naziKilled: function()
	{
		--this.nazisCount;
	},

	update: function(dt)
	{
		if (this.nazisCount < 256)
		{
			var spawnPoint = vec2.v(-100, 100 * (1 - 2 * Math.random()));
			++this.nazisCount;
			this.Injector.create(Nazi, spawnPoint, this);
		}
	}

};