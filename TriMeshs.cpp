#include"TriMeshs.h"

TriMeshs::TriMeshs()
{
	vertexItr = vertices.begin();
	indexItr = indices.begin();
}

void TriMeshs::addVertex(Vertex v)
{
	pushBackVertex(v);
}

void TriMeshs::addVertex(glm::vec3 p)
{
	v.pos = p;
	v.color = glm::vec3(1.0, 1.0, 1.0);
	v.texCoord = glm::vec2(0.0, 0.0);

	pushBackVertex(v);
}

void TriMeshs::setItrBeginV()
{
	vertexItr = vertices.begin();
}

void TriMeshs::setItrBeginI()
{
	indexItr = indices.begin();
}

void TriMeshs::goForwardItrV()
{
	if (vertexItr + 1 != vertices.end())
	{
		vertexItr++;
	}
}

void TriMeshs::goForwardItrI()
{
	if (indexItr + 1 != indices.end())
	{
		indexItr++;
	}
}

std::vector<Vertex>::iterator TriMeshs::getItrV()
{
	if (vertexItr == vertices.end())
	{
		std::cout << "itr == end" << std::endl;
	}

	return vertexItr;
}

std::vector<uint32_t>::iterator TriMeshs::getItrI()
{
	if (indexItr == indices.end() && indexItr == indices.begin())
	{
		std::cout << "itr == end" << std::endl;
	}

	return indexItr;
}

std::vector<Vertex>::iterator TriMeshs::getFirstItrV()
{
	vertexItr = vertices.begin();
	return vertexItr;
}

std::vector<uint32_t>::iterator TriMeshs::getFirstItrI()
{
	indexItr = indices.begin();
	return indexItr;
}

std::vector<Vertex>::iterator TriMeshs::getEndItrV()
{
	return vertices.end();
}

std::vector<uint32_t>::iterator TriMeshs::getEndItrI()
{
	return indices.end();
}

void TriMeshs::pushBackVertex(Vertex v)
{
	vertices.push_back(v);
}

void TriMeshs::pushBackIndex(uint32_t u)
{
	indices.push_back(u);
}

uint32_t TriMeshs::getSizeVertex()
{
	return vertices.size();
}

uint32_t TriMeshs::getSizeIndex()
{
	return indices.size();
}

Vertex* TriMeshs::getFirstPointerVertex()
{
	return &vertices[0];
}

uint32_t* TriMeshs::getFirstPointerIndex()
{
	return &indices[0];
}

void TriMeshs::calcNormal()
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