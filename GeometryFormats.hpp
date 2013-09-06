#ifndef ___FIRSTBLOOD_GEOMETRY_FORMATS_HPP___
#define ___FIRSTBLOOD_GEOMETRY_FORMATS_HPP___

#include "general.hpp"

class GeometryFormats : public Object
{
public:
	//*** Формат моделей для отладочной отрисовки.
	struct Debug
	{
		ptr<VertexLayout> vl;
		ptr<AttributeLayout> al;
		ptr<AttributeLayoutSlot> als;
		ptr<AttributeLayoutElement> alePosition;
		ptr<AttributeLayoutElement> aleNormal;
		ptr<AttributeLayoutElement> aleTexcoord;

		Debug();
	} debug;
};

#endif
