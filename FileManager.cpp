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
        modelPath = "C:\\Users\\sukai\\Documents\\VulkanGame\\models\\Buche_de_Noel.fbx";
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
        aiProcess_FlipWindingOrder |
        aiProcess_SortByPType);

    if (scene == nullptr)
    {
        std::cout << "aaaaaaaaaaaa" << std::endl;
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
        Meshes* meshes = processAiMesh(mesh, scene);

        std::shared_ptr<Material> material = processAiMaterial(mesh->mMaterialIndex, scene);
        meshes->setMaterial(material);
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
    Meshes* meshes = new Meshes();

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        vertex.pos = glm::vec3(mesh->mVertices[i].x, -mesh->mVertices[i].y, mesh->mVertices[i].z);
        vertex.normal = glm::vec3(mesh->mNormals[i].x, -mesh->mNormals[i].y, mesh->mNormals[i].z);

        if (mesh->mTextureCoords[0])
        {
            vertex.texCoord = glm::vec2(mesh->mTextureCoords[0][i].x, -mesh->mTextureCoords[0][i].y);
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

    return meshes;
}

std::shared_ptr<Material> FileManager::processAiMaterial(int index, const aiScene* scene)
{
    std::shared_ptr<Material> material = std::shared_ptr<Material>(new Material());

    aiMaterial* aiMat = scene->mMaterials[index];

    glm::vec3 v;
    
    aiColor3D diffuse{};
    aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);

    aiColor3D ambient{};
    aiMat->Get(AI_MATKEY_COLOR_AMBIENT, ambient);

    aiColor3D specular{};
    aiMat->Get(AI_MATKEY_COLOR_SPECULAR, specular);

    aiColor3D emissive{};
    aiMat->Get(AI_MATKEY_COLOR_EMISSIVE, emissive);

    float shininess;
    aiMat->Get(AI_MATKEY_SHININESS, shininess);

    aiColor3D transparent{};
    aiMat->Get(AI_MATKEY_COLOR_TRANSPARENT, transparent);

    if (aiMat->GetTextureCount(aiTextureType_DIFFUSE) > 0)
    {
        aiString Path;
        if (aiMat->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
        {
            std::string path = Path.data;
            std::shared_ptr<ImageData> imageData = loadModelImage(path);

            material->setImageData(imageData);
        }

        Path.Clear();
    }

    return material;
}

std::shared_ptr<ImageData> FileManager::loadModelImage(std::string filePath)
{
    Storage* storage = Storage::GetInstance();

    HRESULT hr = S_OK;
    HRSRC hrsrc = FindResource(NULL, MAKEINTRESOURCE(IDB_PNG1), L"PNG");
    hr = (hrsrc ? S_OK : E_FAIL);

    HGLOBAL handle = NULL;
    if (SUCCEEDED(hr)) {
        handle = LoadResource(NULL, hrsrc);
        hr = (handle ? S_OK : E_FAIL);
    }

    void* pFile = nullptr;
    if (SUCCEEDED(hr)) {
        pFile = LockResource(handle);
        hr = (pFile ? S_OK : E_FAIL);
    }

    int size = 0;
    if (SUCCEEDED(hr)) {
        size = SizeofResource(NULL, hrsrc);
        hr = (size ? S_OK : E_FAIL);
    }

    int width, height, texChannels;
    std::vector<unsigned char> pixels;

    stbi_uc* picture = nullptr;
    picture = stbi_load_from_memory((unsigned char*)pFile,size,&width,&height,&texChannels,4);
    if (!picture)
    {
        throw std::runtime_error("faile image load");
    }

    int imageSize = width * height * 4;
    ImageData* imageData = new ImageData(width,height,texChannels,picture);

    stbi_image_free(picture);

    storage->addImageData(filePath,imageData);

    return storage->getImageData(filePath);
}