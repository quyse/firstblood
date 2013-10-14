require("math/vec2")
require("math/random")

var CityGenerator = function(width, height, minBlockWidth, minBlockHeight, minStreetWidth, maxStreetWidth, squariness)
{
	this.width = width;
	this.height = height;
	this.minBlockWidth = minBlockWidth;
	this.minBlockHeight = minBlockHeight;
	this.minStreetWidth = minStreetWidth;
	this.maxStreetWidth = maxStreetWidth;
	this.approximateMaxSteps = Math.min(width / minBlockWidth, height / minBlockHeight);
	this.squariness = squariness;
};

CityGenerator.prototype = {

	generate: function(seed)
	{
		this.randomGenerator = new MersenneTwister(seed);
		return this.createBlocks([-0.5 * this.width, -0.5 * this.height], [0.5 * this.width, 0.5 * this.height], 0, 0, 0);
	},

	createBlocks: function(min, max, step, diffDivides)
	{
		//log("****", min, " : ", max)
		var widthAvailable = max[0] - min[0];
		var heightAvailable = max[1] - min[1];
		var canDivideX = widthAvailable >= 2 * this.minBlockWidth + this.minStreetWidth;
		var canDivideY = heightAvailable >= 2 * this.minBlockHeight + this.minStreetWidth;
		var dontDivideFurther = this.randomTest(this.squariness * (step - 1) / this.approximateMaxSteps);
		//log(canDivideX, canDivideY, dontDivideFurther, widthAvailable, heightAvailable)

		if (!(canDivideX || canDivideY))
		{
			//log("nothing to be done")
			return [[min, max]];
		}
		else if (dontDivideFurther)
		{
			return [];
		}

		var divideByX = this.randomTest(0.5 - 0.5 * diffDivides);
		if ((canDivideX && divideByX) || !canDivideY)
		{
			var maxStreet = Math.min(widthAvailable - 2 * this.minBlockWidth, this.maxStreetWidth);
			var streetSize = this.random(this.minStreetWidth, maxStreet);
			var streetPosition = this.random(this.minBlockWidth, widthAvailable - streetSize - this.minBlockWidth);

			//log("divide by X", streetPosition, streetSize, [min[0] + streetPosition + streetSize, min[1]], max);
			var leftBlocks = this.createBlocks(min, [min[0] + streetPosition, max[1]], step + 1, diffDivides + 1);
			var rightBlocks = this.createBlocks([min[0] + streetPosition + streetSize, min[1]], max, step + 1, diffDivides + 1);
			return leftBlocks.concat(rightBlocks);
		}
		else
		{
			maxStreet = Math.min(heightAvailable - 2 * this.minBlockHeight, this.maxStreetWidth);
			streetSize = this.random(this.minStreetWidth, maxStreet);
			streetPosition = this.random(this.minBlockHeight, heightAvailable - streetSize - this.minBlockHeight);

			//log("divide by Y", streetPosition);
			var topBlocks = this.createBlocks(min, [max[0], min[1] + streetPosition], step + 1, diffDivides - 1);
			var bottomBlocks = this.createBlocks([min[0], min[1] + streetPosition + streetSize], max, step + 1, diffDivides - 1);
			return topBlocks.concat(bottomBlocks);
		}

	},

	random: function(a, b)
	{
		return a + (b - a) * this.randomGenerator.random();
	},

	randomTest: function(a)
	{
		return this.randomGenerator.random() < a;
	}


};