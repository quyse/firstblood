#ifndef ___FIRSTBLOOD_GEOMETRY_FORMATS_HPP___
#define ___FIRSTBLOOD_GEOMETRY_FORMATS_HPP___

#include "general.hpp"

class GeometryFormats : public Object
{
public:
	//*** Формат моделей для отладочной отрисовки.
	struct Debug
	{
		struct Vertex
		{
			vec3 position;
			vec3 color;

			Vertex(const vec3& position = vec3(0, 0, 0), const vec3& color = vec3(0, 0, 0));
		};

		ptr<VertexLayout> vl;
		ptr<AttributeLayout> al;
		ptr<AttributeLayoutSlot> als;
		ptr<AttributeLayoutElement> alePosition;
		ptr<AttributeLayoutElement> aleColor;

		Debug();
	} debug;
};

#endif
