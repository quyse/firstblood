#include "GeometryFormats.hpp"

GeometryFormats::Debug::Debug() :
	vl(NEW(VertexLayout(32))),
	al(NEW(AttributeLayout())),
	als(al->AddSlot()),
	alePosition(al->AddElement(als, vl->AddElement(DataTypes::_vec3, 0))),
	aleNormal(al->AddElement(als, vl->AddElement(DataTypes::_vec3, 12))),
	aleTexcoord(al->AddElement(als, vl->AddElement(DataTypes::_vec2, 24)))
{}
