#include"Meshes.h"

void Meshes::pushBackVertex(Vertex* v)
{
	vertices.push_back(*v);
}

void Meshes::pushBackIndex(uint32_t i)
{
	indices.push_back(i);
}

std::vector<Vertex>::iterator Meshes::getVertItr()
{
	return vertices.begin();
}

std::vector<uint32_t>::iterator Meshes::getIndiItr()
{
	return indices.begin();
}

Vertex* Meshes::getVertexPoint()
{
	return vertices.data();
}

uint32_t* Meshes::getIndexPoint()
{
	return indices.data();
}

uint32_t Meshes::getVerticesSize()
{
	return vertices.size();
}

uint32_t Meshes::getIndicesSize()
{
	return indices.size();
}

void Meshes::vertResize(uint32_t size)
{
	vertices.resize(size);
}

void Meshes::indexResize(uint32_t size)
{
	indices.resize(size);
}