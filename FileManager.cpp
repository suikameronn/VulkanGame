#include"FileManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

FileManager* FileManager::fileManager = nullptr;

FileManager::FileManager()
{
    indexSize = 0;
}

glm::mat4 FileManager::aiMatrix4x4ToGlm(const aiMatrix4x4* from)
{
    glm::mat4 to;

    to[0][0] = from->a1; to[0][1] = from->b1;  to[0][2] = from->c1; to[0][3] = from->d1;
    to[1][0] = from->a2; to[1][1] = from->b2;  to[1][2] = from->c2; to[1][3] = from->d2;
    to[2][0] = from->a3; to[2][1] = from->b3;  to[2][2] = from->c3; to[2][3] = from->d3;
    to[3][0] = from->a4; to[3][1] = from->b4;  to[3][2] = from->c4; to[3][3] = from->d4;

    return to;
}

int FileManager::getModelResource(OBJECT obj)
{
    switch (obj)
    {
    case FBXTEST:
        return IDR_MODEL1;
    case UNITYCHAN_NO_ANIM:
        return IDR_MODEL2;
    case GROUND1:
        return IDR_MODEL3;
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

    void* ptr = nullptr;
    int size = 0;
    loadFbxModel(getModelResource(obj), &ptr,size);

    const aiScene* scene = importer.ReadFileFromMemory(ptr,size,
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType);

    processNode(scene->mRootNode,scene,fbxModel);

    fbxModel->calcAveragePos();

    storage->addModel(obj,fbxModel);

    return storage->getFbxModel(obj);
}

void FileManager::processNode(const aiNode* node, const aiScene* scene, FbxModel* model)
{
    int meshNumVertices = 0;

    int allVertices = 0;
    for (uint32_t i = 0; i < scene->mNumMeshes; i++)
    {
        allVertices += scene->mMeshes[i]->mNumVertices;
    }
    model->ReserveBones(allVertices);

    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        Meshes* meshes = processAiMesh(mesh, scene,meshNumVertices,model);
        meshNumVertices += mesh->mNumVertices;

        std::shared_ptr<Material> material = processAiMaterial(mesh->mMaterialIndex, scene);
        meshes->setMaterial(material);
        model->addMeshes(meshes);
    }

    // then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene, model);
    }
}

Meshes* FileManager::processAiMesh(const aiMesh* mesh,const aiScene* scene,uint32_t meshNumVertices, FbxModel* model)
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

    processMeshBones(mesh, meshNumVertices,model);

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

void FileManager::processMeshBones(const aiMesh* mesh, uint32_t meshNumVertices, FbxModel* model)
{
    for (uint32_t i = 0; i < mesh->mNumBones; i++)
    {
        loadSingleBone(mesh->mBones[i], meshNumVertices,model);
    }
}

void FileManager::loadSingleBone(const aiBone* bone, uint32_t meshNumVertices, FbxModel* model)
{
    int boneID = getBoneID(bone,model);

    glm::mat4 offset = aiMatrix4x4ToGlm(&bone->mOffsetMatrix);
    model->setBoneInfo(boneID, offset);

    for (uint32_t i = 0; i < bone->mNumWeights; i++)
    {
        const aiVertexWeight& vw = bone->mWeights[i];
        uint32_t globalVertexID = meshNumVertices + bone->mWeights[i].mVertexId;
        model->addBoneData(globalVertexID, boneID, vw.mWeight);
    }
}

int FileManager::getBoneID(const aiBone* bone, FbxModel* model)
{
    std::string boneName(bone->mName.C_Str());

    return model->setBoneToMap(boneName);
}

std::shared_ptr<Material> FileManager::processAiMaterial(int index, const aiScene* scene)
{
    std::shared_ptr<Material> material = std::shared_ptr<Material>(new Material());

    aiMaterial* aiMat = scene->mMaterials[index];

    glm::vec3 v;
    
    aiColor3D diffuse{};
    aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
    v = glm::vec3(diffuse.r, diffuse.g, diffuse.b);
    material->setDiffuse(&v);

    aiColor3D ambient{};
    aiMat->Get(AI_MATKEY_COLOR_AMBIENT, ambient);
    v = glm::vec3(ambient.r, ambient.g, ambient.b);
    material->setAmbient(&v);

    aiColor3D specular{};
    aiMat->Get(AI_MATKEY_COLOR_SPECULAR, specular);
    v = glm::vec3(specular.r, specular.g, specular.b);
    material->setSpecular(&v);

    aiColor3D emissive{};
    aiMat->Get(AI_MATKEY_COLOR_EMISSIVE, emissive);
    v = glm::vec3(emissive.r, emissive.g, emissive.b);
    material->setEmissive(&v);

    float shininess;
    aiMat->Get(AI_MATKEY_SHININESS, shininess);
    material->setShininess(&shininess);

    aiColor3D transparent{};
    aiMat->Get(AI_MATKEY_COLOR_TRANSPARENT, transparent);
    v = glm::vec3(transparent.r,transparent.g,transparent.b);
    material->setDiffuse(&v);

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

void FileManager::loadFbxModel(int id,void** ptr,int& size)
{
    HRESULT hr = S_OK;
    HRSRC hrsrc = FindResource(NULL, MAKEINTRESOURCE(id), L"MODEL");
    hr = (hrsrc ? S_OK : E_FAIL);

    HGLOBAL handle = NULL;
    if (SUCCEEDED(hr)) {
        handle = LoadResource(NULL, hrsrc);
        hr = (handle ? S_OK : E_FAIL);
    }

    if (SUCCEEDED(hr)) {
        *ptr = (void*)LockResource(handle);
        hr = (*ptr ? S_OK : E_FAIL);
    }

    if (SUCCEEDED(hr)) {
        size = SizeofResource(NULL, hrsrc);
        hr = (size ? S_OK : E_FAIL);
    }
}

std::string FileManager::extractFileName(std::string path)
{
    std::string symbol = "\\";
    int symbolLength = symbol.length();
    int pathLength = path.length();
    if (pathLength < symbolLength)
    {
        throw std::runtime_error("extractFilePath:: symbol is longer than path");
    }

    int pos = 0,tmp = 0;
    while (true)
    {
        tmp = path.find(symbol);
        if (tmp < pathLength && tmp >= 0)
        {
            pos = tmp;
            path = path.substr(pos + symbolLength);
        }
        else
        {
            break;
        }
    }

    return path;
}

int FileManager::getImageID(std::string path)
{
    if (path == "Buche de Noel.png")
    {
        return IDB_PNG1;
    }
    else if(path == "body_01.tga")
    {
        return IDB_PNG2;
    }
    else if (path == "eye_iris_L_00.tga")
    {
        return IDB_PNG3;
    }
    else if (path == "eye_iris_R_00.tga")
    {
        return IDB_PNG4;
    }
    else if (path == "eyeline_00.tga")
    {
        return IDB_PNG5;
    }
    else if (path == "face_00.tga")
    {
        return IDB_PNG6;
    }
    else if (path == "hair_01.tga")
    {
        return IDB_PNG7;
    }
    else if (path == "skin_01.tga")
    {
        return IDB_PNG8;
    }
    else if (path == "cheek_00.tga")
    {
        return IDB_PNG9;
    }
    else if (path == "Nature_Texture_01.png")
    {
        return IDB_PNG10;
    }

    return -1;
}

std::shared_ptr<ImageData> FileManager::loadModelImage(std::string filePath)
{
    Storage* storage = Storage::GetInstance();

    if (storage->containImageData(filePath))
    {
        return storage->getImageData(filePath);
    }

    int id = getImageID(extractFileName(filePath));

    HRESULT hr = S_OK;
    HRSRC hrsrc = FindResource(NULL, MAKEINTRESOURCE(id), L"PNG");
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