#include"Meshes.h"

Meshes::~Meshes()
{

}

void Meshes::pushBackVertex(Vertex& v)
{
	vertices.push_back(v);
}

void Meshes::pushBackIndex(uint32_t i)
{
	indices.push_back(i);
}

void Meshes::pushBackPrimitive(Primitive p)
{
	primitives.push_back(p);
}

/*
void Meshes::setMaterial(std::shared_ptr<Material> material)
{
	this->material = material;
}
*/

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

Primitive* Meshes::getPrimitivePoint()
{
	return primitives.data();
}

uint32_t Meshes::getVerticesSize()
{
	return vertices.size();
}

uint32_t Meshes::getIndicesSize()
{
	return indices.size();
}

uint32_t Meshes::getPrimitivesSize()
{
	return primitives.size();
}

void Meshes::vertResize(uint32_t size)
{
	vertices.resize(size);
}

void Meshes::indexResize(uint32_t size)
{
	indices.resize(size);
}

void Meshes::primitiveResize(uint32_t size)
{
	primitives.resize(size);
}

/*
void Meshes::setLocalTransform(glm::mat4 transform)
{
	localTransform = transform;
}

glm::mat4 Meshes::getLocalTransform()
{
	return localTransform;
}
*/

void Meshes::addBoneData(uint32_t index, uint32_t infoID, float weight)
{
	vertices[index].addBoneData(infoID, weight);
}