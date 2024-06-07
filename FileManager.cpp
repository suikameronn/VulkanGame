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

    FbxModel* fbxModel = new FbxModel();

    const aiScene* scene = importer.ReadFile(getModelPath(obj), 
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType);

    if (scene == nullptr)
    {
        throw std::runtime_error("scene error");
    }

    processNode(scene->mRootNode,scene,fbxModel);

    storage->addModel(obj,fbxModel);
    return storage->getFbxModel(obj);
}

void FileManager::processNode(const aiNode* node, const aiScene* scene,FbxModel* model)
{

    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes = processAiMesh(mesh, scene);
        model->addMeshes(meshes);
    }

    // then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene,model);
    }
}

Meshes* FileManager::processAiMesh(const aiMesh* mesh,const aiScene* scene)
{
    meshes = new Meshes();

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
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

    indexSize += mesh->GetPolygonCount() * 3;
}

Meshes* FileManager::loadPointsFbx(OBJECT obj)
{
    if (Storage::GetInstance()->containMeshes(obj))
    {
        return Storage::GetInstance()->accessObj(obj);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
            meshes->pushBackIndex(face.mIndices[j]);
    }

    //頂点バッファの取得
    FbxVector4* verticesFbx = mesh->GetControlPoints();
    //インデックスバッファの取得
    int* indicesFbx = mesh->GetPolygonVertices();
    //頂点座標の数の取得
    int polygonVertexCount = mesh->GetPolygonVertexCount();
    
ImageData* FileManager::loadModelImage(std::string filePath)
{
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
}    }

    Meshes* meshes = new Meshes();

    /*fbxファイルの読み込み*/
    importer->Initialize(getModelPath(obj).c_str(), -1, manager->GetIOSettings());
    importer->Import(scene);

    //三角ポリゴン化
    converter->Triangulate(scene, true);

    FbxNode* node = scene->GetRootNode();

    if (node)
    {
        for (int i = 0; i < node->GetChildCount(); i++)
        {
            loadPointsFbx(meshes,node->GetChild(i));
        }
    }

    Storage::GetInstance()->addObj(obj, meshes);

    return meshes;
}
*/

std::string FileManager::getImagePath(IMAGE image)
{
    switch (image)
    {
    case IMAGE::IMAGETEST:
        imagePath = "textures\\viking_room.png";
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

    unsigned char* picture = stbi_load(filePath.c_str(), &width, &height, &texChannels, STBI_rgb_alpha);
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