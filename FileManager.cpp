#include"FileManager.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include<tiny_obj_loader.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

std::string FileManager::getModelPath(OBJECT obj)
{
    switch (obj)
    {
    case OBJECT::MODELTEST:
        modelPath = "models/viking_room.obj";
        return modelPath;
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

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, getModelPath(obj).c_str()))
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
                uniqueVertices[vertex] = model->getVerticesSize();
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

std::string FileManager::getImagePath(IMAGE image)
{
    switch (image)
    {
    case IMAGE::IMAGETEST:
        imagePath = "textures/viking_room.png";
        return imagePath;
        break;
    }
}

ImageData* FileManager::loadImage(IMAGE image)
{
    if (images.contains(image))
    {
        return images[image].get();
    }

    ImageData imageData;
    stbi_uc* pixels = stbi_load(getImagePath(image).c_str(), &imageData.width, &imageData.height, &imageData.texChannels,STBI_rgb_alpha);
    
    if (!pixels)
    {
        throw std::runtime_error("faile image load");
    }
    
    int imageSize = imageData.width * imageData.height * 4;
    imageData.pixcels = std::shared_ptr<unsigned char>(new unsigned char(imageSize));
    std::copy(pixels[0], pixels[imageSize - 1], imageData.pixcels);

    stbi_image_free(pixels);

    images[image].reset(&imageData);
    return images[image].get();
}