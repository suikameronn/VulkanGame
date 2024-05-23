#include"FileManager.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include<tiny_obj_loader.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

FileManager* FileManager::fileManager = nullptr;

std::string FileManager::getModelPath(OBJECT obj)
{
    switch (obj)
    {
    case OBJECT::MODELTEST:
        modelPath = "./models/viking_room.obj";
        return modelPath;
        break;
    }
}

Meshes* FileManager::loadModelPoints(OBJECT obj)
{
    if (Storage::GetInstance()->containMeshes(obj))
    {
        return Storage::GetInstance()->accessObj(obj);
    }

    Meshes* meshes = new Meshes();

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
                uniqueVertices[vertex] = meshes->getVerticesSize();
                meshes->pushBackVertex(&vertex);
            }

            meshes->pushBackIndex(uniqueVertices[vertex]);
        }
    }

    Storage::GetInstance()->addObj(obj, meshes);

    return meshes;
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

ImageData* FileManager::loadModelImage(IMAGE image)
{
    if (Storage::GetInstance()->containImageData(image))
    {
        return Storage::GetInstance()->accessImage(image);
    }

    int width, height, texChannels;
    std::vector<unsigned char> pixels;

    unsigned char* picture = stbi_load(getImagePath(image).c_str(), &width, &height, &texChannels, STBI_rgb_alpha);
    if (!picture)
    {
        throw std::runtime_error("faile image load");
    }

    int imageSize = width * height * 4;
    imageData = new ImageData(width,height,texChannels,picture);

    stbi_image_free(picture);

    Storage::GetInstance()->addImage(image, imageData);

    return Storage::GetInstance()->accessImage(image);
}