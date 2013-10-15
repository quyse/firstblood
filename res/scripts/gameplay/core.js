var GameplayRegistry = function()
{
	this.uidCounter = 0;
	this.table = {};
	this.uidToListIndexMap = {};
	this.list = [];

	this.toBeRemoved = [];
	this.updating = false;
}
GameplayRegistry.prototype = {

	get: function(uid)
	{
		return this.table[uid];
	},

	register: function(object)
	{
		object.alive = true;
		object.uid = ++this.uidCounter;
		this.table[object.uid] = object;
		if (object.update)
			this.list.push(object);
		this.uidToListIndexMap[object.uid] = this.list.length - 1;
	},

	unregister: function(object)
	{
		object.alive = false;
		if (this.updating)
		{
			this.toBeRemoved.push(object);
		}
		else
		{
			if (object.update)
			{
				var listIndex = this.uidToListIndexMap[object.uid];
				delete this.uidToListIndexMap[object.uid];

				var last = this.list[this.list.length - 1];
				this.list[listIndex] = last;
				this.uidToListIndexMap[last.uid] = listIndex;
				this.list.pop();
			}
			delete this.table[object.uid];
		}
	},

	update: function(dt)
	{
		this.updating = true;
		for (var i = 0, l = this.list.length; i < l; ++i)
			this.list[i].update(dt);

		this.updating = false;
		for (i = 0, l = this.toBeRemoved.length; i < l; ++i)
			this.unregister(this.toBeRemoved[i]);
		this.toBeRemoved = [];
	}

};


var DebugDrawer = function()
{
	this.list = [];
};
DebugDrawer.prototype = {

	register: function(object)
	{
		this.list.push(object);
	},

	unregister: function(object)
	{
		var index = this.list.indexOf(object);
		if (index >= 0)
			this.list.splice(index, 1);
	},

	update: function()
	{
		for (var i = 0, l = this.list.length; i < l; ++i)
			this.list[i].debugDraw();
	}

};