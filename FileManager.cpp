#include"FileManager.h"

#define TINYOBJLOADER_IMPLEMENTATION

#include<tiny_obj_loader.h>

std::string FileManager::getPath(OBJECT obj)
{
    switch (obj)
    {
    case TEST:
        path = "C:\\Users\\sugiyama\\Documents\\VulkanGame\\models\\viking_room.obj";
        return path;
        break;

    }
}

Model* FileManager::loadModel(OBJECT obj)
{
    if (models.contains(obj))
    {
        return models[obj].get();
    }

    Model* model = new Model();


    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, getPath(obj).c_str()))
    {
        throw std::runtime_error(warn + err);
    }

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    for (const auto& shape : shapes)//すべてのシェイプの数だけ繰り返す
    {
        for (const auto& index : shape.mesh.indices)//指定のシェイプを構成するメッシュの数だけ繰り返す
        {
            Vertex vertex{};

            vertex.pos =
            {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            vertex.texCoord =
            {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };

            vertex.color = { 0.0f,0.0f,0.0f };

            vertex.normal =
            {
                attrib.normals[3 * index.normal_index + 0],
                attrib.normals[3 * index.normal_index + 1],
                attrib.normals[3 * index.normal_index + 2]
            };

            if (uniqueVertices.count(vertex) == 0)
            {
                uniqueVertices[vertex] = static_cast<uint32_t>(model->getVerticesSize());
                model->pushBackVertex(&vertex);
            }

            model->pushBackIndex(uniqueVertices[vertex]);
        }
    }

    models[obj].reset(model);

    return models[obj].get();
}

Model* FileManager::getModelData(OBJECT obj)
{
    return models[obj].get();
}