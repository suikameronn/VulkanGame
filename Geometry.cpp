#include"Geometry.h"

void Geometry::pushBackVertex(Vertex* v)
{
	vertices.push_back(*v);
}

void Geometry::pushBackIndex(uint32_t i)
{
	indices.push_back(i);
}

std::vector<Vertex>::iterator Geometry::getVertItr()
{
	return vertices.begin();
}

std::vector<uint32_t>::iterator Geometry::getIndiItr()
{
	return indices.begin();
}

Vertex* Geometry::getVertexPoint()
{
	return vertices.data();
}

uint32_t* Geometry::getIndexPoint()
{
	return indices.data();
}

uint32_t Geometry::getVerticesSize()
{
	return vertices.size();
}

uint32_t Geometry::getIndicesSize()
{
	return indices.size();
}