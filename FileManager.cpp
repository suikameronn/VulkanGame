#include"FileManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

FileManager* FileManager::fileManager = nullptr;

FileManager::FileManager()
{
    indexSize = 0;
}

glm::vec3 FileManager::aiVec3DToGLM(const aiVector3D* vec)
{
    return glm::vec3(vec->x, vec->y, vec->z);
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
    case FRAG:
        return IDR_MODEL4;
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
    loadFbxModel(getModelResource(obj), &ptr, size);

    const aiScene* scene = importer.ReadFileFromMemory(ptr, size,
        aiProcess_Triangulate);

    allVertNum = 0;
    processNode(scene->mRootNode, scene, fbxModel);
    imageDataCount = 0;

    if (scene->mNumAnimations > 0)
    {
        loadAnimation(scene, fbxModel);
    }

    importer.FreeScene();

    fbxModel->calcAveragePos();

    loadAnimations(fbxModel);

    storage->addModel(obj, fbxModel);

    return storage->getFbxModel(obj);
}

void FileManager::processNode(const aiNode* node, const aiScene* scene, FbxModel* model)
{

    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        Meshes* meshes = processAiMesh(mesh, scene, allVertNum, model);

        glm::mat4 localMat = aiMatrix4x4ToGlm(&node->mTransformation);
        localMat[3][1] *= -1;
        meshes->setLocalTransform(localMat);
        allVertNum += mesh->mNumVertices;

        std::shared_ptr<Material> material = processAiMaterial(mesh->mMaterialIndex, scene);
        if (material->hasImageData())
        {
            imageDataCount++;
        }

        meshes->setMaterial(material);
        model->addMeshes(meshes);
    }

    // then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene, model);
    }

    model->setImageDataCount(imageDataCount);
}

Meshes* FileManager::processAiMesh(const aiMesh* mesh, const aiScene* scene, uint32_t meshNumVertices, FbxModel* model)
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
        meshes->pushBackVertex(vertex);
    }

    if (mesh->mNumBones > 0)
    {
        model->setBoneInfo(0, glm::mat4(1.0f));
        processMeshBones(mesh, meshNumVertices, model, meshes);
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

void FileManager::processMeshBones(const aiMesh* mesh, uint32_t meshNumVertices, FbxModel* model,Meshes* meshes)
{
    for (uint32_t i = 0; i < mesh->mNumBones; i++)
    {
        loadSingleBone(mesh->mBones[i], meshNumVertices,model,meshes);
    }
}

void FileManager::loadSingleBone(const aiBone* bone, uint32_t meshNumVertices, FbxModel* model, Meshes* meshes)
{
    int boneID = getBoneID(bone, model) + 1;
    glm::mat4 offset = aiMatrix4x4ToGlm(&bone->mOffsetMatrix);
    model->setBoneInfo(boneID, offset);

    for (uint32_t i = 0; i < bone->mNumWeights; i++)
    {
        meshes->addBoneData(bone->mWeights[i].mVertexId, boneID, bone->mWeights[i].mWeight);
    }
}

int FileManager::getBoneID(const aiBone* bone, FbxModel* model)
{
    std::string boneName(bone->mName.C_Str());

    return model->getBoneToMap(boneName);
}

const aiNodeAnim* FileManager::findNodeAnim(const aiAnimation* pAnimation, std::string nodeName)
{
    for (uint32_t i = 0; i < pAnimation->mNumChannels; i++)
    {
        const aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];

        if (std::string(pNodeAnim->mNodeName.data) == nodeName)
        {
            return pNodeAnim;
        }
    }

    return nullptr;
}

void FileManager::ReadNodeHeirarchy(const aiScene* scene, aiNode* node
    , AnimNode* parentNode, unsigned int i, FbxModel* model)
{
    AnimNode* currentNode;

    std::string nodeName(node->mName.data);
    const aiAnimation* pAnimation = scene->mAnimations[0];

    aiMatrix4x4 NodeTransformation(node->mTransformation);
    const aiNodeAnim* pNodeAnim = findNodeAnim(pAnimation, nodeName);

    if (pNodeAnim)
    {
        std::unordered_map<float, glm::vec3> keyTimeScale;
        for (uint32_t i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++)
        {
            keyTimeScale[pNodeAnim->mScalingKeys[i + 1].mTime] = aiVec3DToGLM(&pNodeAnim->mScalingKeys[i + 1].mValue);
        }

        std::unordered_map<float, aiQuaternion> keyTimeQuat;
        for (uint32_t i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++)
        {
            keyTimeQuat[pNodeAnim->mRotationKeys[i + 1].mTime] = pNodeAnim->mRotationKeys->mValue;
        }

        std::unordered_map<float, glm::vec3> keyTimePos;
        for (uint32_t i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++)
        {
            keyTimePos[pNodeAnim->mPositionKeys[i + 1].mTime] = aiVec3DToGLM(&pNodeAnim->mPositionKeys[i + 1].mValue);
        }

        AnimationKeyData animKeyData = { keyTimeScale, keyTimeQuat, keyTimePos };
        currentNode = new AnimNode(nodeName, animKeyData,node->mNumChildren);
    }
    else
    {
        currentNode = new AnimNode(nodeName, aiMatrix4x4ToGlm(&NodeTransformation), node->mNumChildren);
    }

    currentNode->resizeChildren(node->mNumChildren);
    parentNode->setChild(i, currentNode);

    for (uint32_t i = 0; i < node->mNumChildren; i++)
    {
        ReadNodeHeirarchy(scene, node->mChildren[i], currentNode, i, model);
    }
}

void FileManager::ReadNodeHeirarchy(const aiScene* scene, aiNode* node
    , AnimNode* parentNode, FbxModel* model,std::shared_ptr<Animation> animation)
{
    AnimNode* currentNode;

    std::string nodeName(node->mName.data);
    const aiAnimation* pAnimation = scene->mAnimations[0];

    aiMatrix4x4 NodeTransformation(node->mTransformation);
    const aiNodeAnim* pNodeAnim = findNodeAnim(pAnimation, nodeName);

    if (pNodeAnim)
    {
        std::unordered_map<float, glm::vec3> keyTimeScale;
        for (uint32_t i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++)
        {
            keyTimeScale[pNodeAnim->mScalingKeys[i + 1].mTime] = aiVec3DToGLM(&pNodeAnim->mScalingKeys[i + 1].mValue);
        }

        std::unordered_map<float, aiQuaternion> keyTimeQuat;
        for (uint32_t i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++)
        {
            keyTimeQuat[pNodeAnim->mRotationKeys[i + 1].mTime] = pNodeAnim->mRotationKeys->mValue;
        }

        std::unordered_map<float, glm::vec3> keyTimePos;
        for (uint32_t i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++)
        {
            keyTimePos[pNodeAnim->mPositionKeys[i + 1].mTime] = aiVec3DToGLM(&pNodeAnim->mPositionKeys[i + 1].mValue);
        }

        AnimationKeyData animKeyData = { keyTimeScale, keyTimeQuat, keyTimePos };
        currentNode = new AnimNode(nodeName, animKeyData, node->mNumChildren);
    }
    else
    {
        currentNode = new AnimNode(nodeName, aiMatrix4x4ToGlm(&NodeTransformation), node->mNumChildren);
    }

    animation->setRootNode(currentNode);

    parentNode = currentNode;

    currentNode->resizeChildren(node->mNumChildren);
    for (uint32_t i = 0; i < node->mNumChildren; i++)
    {
        ReadNodeHeirarchy(scene, node->mChildren[i], currentNode, i, model);
    }
}

//���炩�̕����ŕ����̃p�X�Ƃ���ɕt������A�j���[�V��������n��(������)
void FileManager::loadAnimations(FbxModel* fbxModel)
{
    void* ptr = nullptr;
    int size = 0;
    loadFbxModel(163, &ptr, size);

    const aiScene* scene = importer.ReadFileFromMemory(ptr, size, aiProcess_SortByPType);

    loadAnimation(scene,fbxModel);
}

void FileManager::loadAnimation(const aiScene* scene,FbxModel* model)
{
    std::shared_ptr<Animation> animation =
        std::shared_ptr<Animation>(new Animation(
            scene->mAnimations[0]->mTicksPerSecond
        ,scene->mAnimations[0]->mDuration));

    AnimNode* rootNode = nullptr;

    ReadNodeHeirarchy(scene,scene->mRootNode,rootNode,model,animation);

    model->setAnimation("walk",animation);
}

std::shared_ptr<Material> FileManager::processAiMaterial(int index, const aiScene* scene)
{
    std::shared_ptr<Material> material = std::shared_ptr<Material>(new Material());

    aiMaterial* aiMat = scene->mMaterials[index];

    glm::vec3 v;

    aiColor4D diffuse{};
    aiGetMaterialColor(aiMat,AI_MATKEY_COLOR_DIFFUSE, &diffuse);
    v = glm::vec3(diffuse.r, diffuse.g, diffuse.b);
    material->setDiffuse(&v);

    aiColor4D ambient{};
    aiGetMaterialColor(aiMat,AI_MATKEY_COLOR_AMBIENT, &ambient);
    v = glm::vec3(ambient.r, ambient.g, ambient.b);
    material->setAmbient(&v);

    aiColor4D specular{};
    aiGetMaterialColor(aiMat,AI_MATKEY_COLOR_SPECULAR, &specular);
    v = glm::vec3(specular.r, specular.g, specular.b);
    material->setSpecular(&v);

    aiColor4D emissive{};
    aiGetMaterialColor(aiMat,AI_MATKEY_COLOR_EMISSIVE, &emissive);
    v = glm::vec3(emissive.r, emissive.g, emissive.b);
    material->setEmissive(&v);

    aiColor4D shininess;
    aiGetMaterialColor(aiMat,AI_MATKEY_SHININESS, &shininess);
    float shine = shininess[3];
    material->setShininess(&shine);

    aiColor4D transparent{};
    aiGetMaterialColor(aiMat,AI_MATKEY_COLOR_TRANSPARENT, &transparent);
    float trans = transparent[3];
    material->setTransmissive(&trans);

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

void FileManager::loadFbxModel(int id, void** ptr, int& size)
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

    int pos = 0, tmp = 0;
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
    else if (path == "body_01.tga")
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
    else if (path == "Add_your_Flag.png")
    {
        return IDB_PNG11;
    }
    else
    {
        return IDB_PNG4;
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
    picture = stbi_load_from_memory((unsigned char*)pFile, size, &width, &height, &texChannels, 4);
    if (!picture)
    {
        throw std::runtime_error("faile image load");
    }

    int imageSize = width * height * 4;
    ImageData* imageData = new ImageData(width, height, texChannels, picture);

    stbi_image_free(picture);

    storage->addImageData(filePath, imageData);

    return storage->getImageData(filePath);
}