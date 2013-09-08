#include "GeometryFormats.hpp"

GeometryFormats::Debug::Vertex::Vertex(const vec3& position, const vec3& color) :
	position(position), color(color) {}

GeometryFormats::Debug::Debug() :
	vl(NEW(VertexLayout(24))),
	al(NEW(AttributeLayout())),
	als(al->AddSlot()),
	alePosition(al->AddElement(als, vl->AddElement(&Vertex::position))),
	aleColor(al->AddElement(als, vl->AddElement(&Vertex::color)))
{}
