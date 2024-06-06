#include"FileManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

FileManager* FileManager::fileManager = nullptr;

FileManager::FileManager()
{

    indexSize = 0;
}

std::string FileManager::getModelPath(OBJECT obj)
{
    switch (obj)
    {
    case OBJECT::MODELTEST:
        modelPath = "models/viking_room.obj";
        return modelPath;
    case OBJECT::FBXTEST:
        modelPath = "models/Buche_de_Noel_Cut.fbx";
        return modelPath;
    }
}

std::shared_ptr<FbxModel> FileManager::loadModel(OBJECT obj)
{
    Storage* storage = Storage::GetInstance();
    if (storage->containModel(obj))
    {
        return storage->getFbxModel(obj);
    }

    const aiScene* scene = importer.ReadFile(getModelPath(obj), 
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType);

    if (scene == nullptr)
    {
        throw std::runtime_error("scene error");
    }

    processNode(scene->mRootNode,scene);
}

void FileManager::processNode(const aiNode* node, const aiScene* scene)
{


    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        Meshes* meshes = processAiMesh(mesh, scene);
    }
    // then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

Meshes* processAiMesh(const aiMesh* mesh,const aiScene* scene)
{
    Meshes* meshes = new Meshes();

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        vertex.pos = glm::vec3(mesh->mVertices[i].x, -mesh->mVertices[i].y, mesh->mVertices[i].z);
        vertex.normal = glm::vec3(mesh->mNormals[i].x, -mesh->mNormals[i].y, mesh->mNormals[i].z);

        if (mesh->mTextureCoords[0])
        {
            vertex.texCoord = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        }
        else
        {
            vertex.texCoord = glm::vec2(0.0f, 0.0f);
        }

        meshes->pushBackVertex(&vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
            meshes->pushBackIndex(face.mIndices[j]);
        }
    }

    return Mesh(vertices, indices, textures);
}

ImageData* FileManager::loadModelImage(std::string filePath)
{
    int width, height, texChannels;
    std::vector<unsigned char> pixels;

    unsigned char* picture = stbi_load(filePath.c_str(), &width, &height, &texChannels, STBI_rgb_alpha);
    if (!picture)
    {
        throw std::runtime_error("faile image load");
    }

    int imageSize = width * height * 4;
    imageData = new ImageData(width,height,texChannels,picture);

    stbi_image_free(picture);
}