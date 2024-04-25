#include"Trimesh.h"

Trimesh::Trimesh()
{
	vertexItr = vertices.begin();
	indexItr = indices.begin();
}

void Trimesh::addVertex(Vertex v)
{
	pushBackVertex(v);
}

void Trimesh::addVertex(glm::vec3 p)
{
	v.pos = p;
	v.color = glm::vec3(1.0, 1.0, 1.0);
	v.texCoord = glm::vec2(0.0, 0.0);

	pushBackVertex(v);
}

void Trimesh::setItrBeginV()
{
	vertexItr = vertices.begin();
}

void Trimesh::setItrBeginI()
{
	indexItr = indices.begin();
}

void Trimesh::goForwardItrV()
{
	if (vertexItr + 1 != vertices.end())
	{
		vertexItr++;
	}
}

void Trimesh::goForwardItrI()
{
	if (indexItr + 1 != indices.end())
	{
		indexItr++;
	}
}

std::vector<Vertex>::iterator Trimesh::getItrV()
{
	if (vertexItr == vertices.end())
	{
		std::cout << "itr == end" << std::endl;
	}

	return vertexItr;
}

std::vector<uint32_t>::iterator Trimesh::getItrI()
{
	if (indexItr == indices.end() && indexItr == indices.begin())
	{
		std::cout << "itr == end" << std::endl;
	}

	return indexItr;
}

std::vector<Vertex>::iterator Trimesh::getFirstItrV()
{
	vertexItr = vertices.begin();
	return vertexItr;
}

std::vector<uint32_t>::iterator Trimesh::getFirstItrI()
{
	indexItr = indices.begin();
	return indexItr;
}

std::vector<Vertex>::iterator Trimesh::getEndItrV()
{
	return vertices.end();
}

std::vector<uint32_t>::iterator Trimesh::getEndItrI()
{
	return indices.end();
}

void Trimesh::pushBackVertex(Vertex v)
{
	vertices.push_back(v);
}

void Trimesh::pushBackIndex(uint32_t u)
{
	indices.push_back(u);
}

uint32_t Trimesh::getSizeVertex()
{
	return vertices.size();
}

uint32_t Trimesh::getSizeIndex()
{
	return indices.size();
}

Vertex* Trimesh::getFirstPointerVertex()
{
	return &vertices[0];
}

uint32_t* Trimesh::getFirstPointerIndex()
{
	return &indices[0];
}

void Trimesh::setMaterial(glm::vec3* diffuse, glm::vec3* ambient, glm::vec3* specular, glm::vec3* emissive
	, float* shininess, glm::vec3* transmissive)
{
	material = std::unique_ptr<Material>(new Material(diffuse, ambient, specular, emissive, shininess, transmissive));
}

std::shared_ptr<Material> Trimesh::getMaterial()
{
	return material;
}

void Trimesh::calcNormal()
{
	uint32_t vertSize = getSizeVertex();
	uint32_t indeSize = getSizeIndex();
	normals.resize(vertSize);
	std::fill(normals.begin(), normals.end(), glm::vec3(0.0f,0.0f,0.0f));

	std::vector<uint32_t> numFaces(vertSize);
	std::fill(numFaces.begin(), numFaces.end(), 0);

	uint16_t j;

	for (auto i = indices.begin();i != indices.end(); i += 3)
	{
		glm::vec3* a = &vertices[*i].pos;
		glm::vec3* b = &vertices[*(i + 1)].pos;
		glm::vec3* c = &vertices[*(i + 2)].pos;

		glm::vec3 faceNormal = glm::cross((* b - *a), (* c - *a));
		faceNormal = glm::normalize(faceNormal);

		for (j = 0; j < 3; j++)
		{
			normals[*(i + j)] += faceNormal;
			++numFaces[*(i + j)];
		}
	}

	for (uint32_t i = 0; i < vertSize; ++i)
	{
		if (numFaces[i])
		{
			normals[i] /= numFaces[i];
			vertices[i].normal = normals[i];
		}
	}
}