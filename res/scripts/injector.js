var Injector = function()
{
	this.classUidCounter = 0;
	this.classUidToDependenciesMap = {};
	this.constructedDependencies = {};

	this.functionArgsExtractorRegex = /^function\s*[^\(]*\(\s*([^\)]*)\)/m;

	this.addDependency("Injector", this);
};
Injector.prototype = {

	// first argument is class, other will be passed to instance's ::init()
	create: function()
	{
		var constructor = arguments[0];
		if (!constructor.__classUid__)
			this._registerClass(constructor);

		var dependencies = this.classUidToDependenciesMap[constructor.__classUid__];
		var object = new constructor();
		for (var i = 0, l = dependencies.length; i < l; ++i)
		{
			var dependencyName = dependencies[i];
			var dependency = this.constructedDependencies[dependencyName];
			if (dependency.register)
				dependency.register(object);
			object[dependencyName] = dependency;
		}
		object.__classUid__ = constructor.__classUid__;
		object.init.apply(object, Array.prototype.slice(arguments, 1));
		return object;
	},

	destroy: function(object)
	{
		object.fini.apply(object);
		var dependencies = this.classUidToDependenciesMap[object.__classUid__];
		for (var i = 0, l = dependencies.length; i < l; ++i)
		{
			var dependencyName = dependencies[i];
			var dependency = this.constructedDependencies[dependencyName];
			if (dependency.unregister)
				dependency.unregister(object);
		}
	},

	addDependency: function(className, instance)
	{
		this.constructedDependencies[className] = instance;
	},

	_registerClass: function(cls)
	{
		var classString = cls.toString();
		var args = classString.match(this.functionArgsExtractorRegex)[1].split(',');
		for (var i = 0, l = args.length; i < l; ++i)
			args[i] = args[i].trim();
		var classUid = this.classUidCounter++;
		this.classUidToDependenciesMap[classUid] = args;
		cls.__classUid__ = classUid;
	}

};