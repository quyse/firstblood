require('math/vec3');

global.Gameplay = global.Gameplay || {};

Gameplay.Map = function(size)
{
	this.min = vec3.v(-size * 0.5, -size * 0.5, 0);
	this.max = vec3.v(size * 0.5, size * 0.5, 0);
};