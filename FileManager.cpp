#include"FileManager.h"

#include<tiny_obj_loader.h>

std::string FileManager::getPath(OBJECT obj)
{
    switch (obj)
    {
    case TEST:
        path = "models/viking_room.obj";
        return path;
        break;

    }
}

std::shared_ptr<Model> FileManager::loadModel(OBJECT obj)
{
    if (models.contains(obj))
    {
        Model m = *models[obj];
        std::shared_ptr<Model> p(&m);
        return p;
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

    for (const auto& shape : shapes)//���ׂẴV�F�C�v�̐������J��Ԃ�
    {
        for (const auto& index : shape.mesh.indices)//�w��̃V�F�C�v���\�����郁�b�V���̐������J��Ԃ�
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

    Model m = *models[obj];
    std::shared_ptr<Model> p(&m);
    return p;
}

std::shared_ptr<Model> FileManager::getModelData(OBJECT obj)
{
    return models[obj];
}

void FileManager::loadScene()
{
    if (scene == nullptr)
    {
        scene = std::shared_ptr<Scene>(new Scene());
    }
    else
    {
        scene.reset();
    }
}

std::shared_ptr<Scene> FileManager::getScene()
{
    return scene;
}