#include"FileManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

FileManager* FileManager::fileManager = nullptr;

FileManager::FileManager()
{
}

glm::vec3 FileManager::aiVec3DToGLM(const aiVector3D* vec)
{
    return glm::vec3(vec->x, vec->y, vec->z);
}

glm::mat4 FileManager::aiMatrix4x4ToGlm(const aiMatrix4x4* from)
{
    return glm::transpose(glm::make_mat4(&from->a1));
}

int FileManager::getModelResource(OBJECT obj)
{
    switch (obj)
    {
    case OBJECT::FBXTEST:
        return IDR_MODEL1;
    case OBJECT::UNITYCHAN_NO_ANIM:
        return IDR_MODEL2;
    case OBJECT::GROUND1:
        return IDR_MODEL3;
    case OBJECT::FRAG:
        return IDR_MODEL4;
    case OBJECT::IDLE:
        return IDR_MODEL7;
    case OBJECT::WALK:
        return IDR_MODEL8;
    case OBJECT::NORMALBOX:
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
    loadFbxModel(getModelResource(obj), &ptr, size);

    const aiScene* scene = importer.ReadFileFromMemory(ptr, size,
        aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices);

    fbxModel->setGlobalInverseTransform(aiMatrix4x4ToGlm(&scene->mRootNode->mTransformation.Inverse()));

    allVertNum = 0;
    minPos = glm::vec3(1000.0f, 1000.0f, 1000.0f);
    maxPos = glm::vec3(-1000.0f, -1000.0f, -1000.0f);

    processNode(scene->mRootNode,scene, fbxModel);
    
    fbxModel->setMinMaxVertexPos(minPos, maxPos);

    imageDataCount = 0;

    importer.FreeScene();

    //loadPoses(fbxModel);

    storage->addModel(obj, fbxModel);

    return storage->getFbxModel(obj);
}

/*
void FileManager::processNode(const aiScene* scene, FbxModel* model)
{
    for (uint32_t i = 0; i < scene->mNumMeshes; ++i)
    {
        const aiMesh* mesh = scene->mMeshes[i];

        Meshes* meshes = processAiMesh(mesh, scene, allVertNum, model);

        allVertNum += mesh->mNumVertices;

        std::shared_ptr<Material> material = processAiMaterial(mesh->mMaterialIndex, scene);
        if (material->hasImageData())
        {
            imageDataCount++;
        }

        meshes->setMaterial(material);
        model->addMeshes(meshes);
    }

    model->setTotalVertexNum(allVertNum);

    model->setImageDataCount(imageDataCount);
}
*/


void FileManager::processNode(const aiNode* node, const aiScene* scene, FbxModel* model)
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        Meshes* meshes = processAiMesh(mesh, scene, allVertNum, model);

        glm::mat4 localMat = aiMatrix4x4ToGlm(&node->mTransformation);
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

    model->setTotalVertexNum(allVertNum);

    model->setImageDataCount(imageDataCount);
}

void FileManager::calcMinMaxVertexPos(glm::vec3 pos)
{
    for (int i = 0; i < 3; i++)
    {
        if (minPos[i] > pos[i])
        {
            minPos[i] = pos[i];
        }
    }

    for (int i = 0; i < 3; i++)
    {
        if (maxPos[i] < pos[i])
        {
            maxPos[i] = pos[i];
        }
    }
}

Meshes* FileManager::processAiMesh(const aiMesh* mesh, const aiScene* scene, uint32_t meshNumVertices, FbxModel* model)
{
    Meshes* meshes = new Meshes();

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        vertex.pos = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y,mesh->mVertices[i].z);
        vertex.normal = glm::vec3(mesh->mNormals[i].x, -mesh->mNormals[i].y, mesh->mNormals[i].z);

        if (mesh->HasTextureCoords(0))
        {
            vertex.texCoord = glm::vec2(mesh->mTextureCoords[0][i].x,1 - mesh->mTextureCoords[0][i].y);
        }
        else
        {
            vertex.texCoord = glm::vec2(0.0f, 0.0f);
        }

        calcMinMaxVertexPos(vertex.pos);

        meshes->pushBackVertex(vertex);
    }

    if (mesh->mNumBones != 0)
    {
        processMeshBones(mesh, meshNumVertices, model, meshes);
    }
    else
    {
        std::string newBoneName = mesh->mName.data;
        int boneID = getBoneID(newBoneName, model);
        model->setBoneInfo(boneID, glm::mat4(1.0f));

        for (int i = 0; i < mesh->mNumVertices; i++)
        {
            meshes->addBoneData(i, boneID, 1.0f);
        }
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
    int boneID = getBoneID(bone, model);

    aiMatrix4x4 boneOffsetMatrix = bone->mOffsetMatrix;

    glm::mat4 offset = aiMatrix4x4ToGlm(&boneOffsetMatrix);
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

int FileManager::getBoneID(const std::string boneName, FbxModel* model)
{
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
    , AnimNode* parentNode, unsigned int childIdx, FbxModel* model)
{
    AnimNode* currentNode;

    std::string nodeName(node->mName.data);

    const aiAnimation* pAnimation = scene->mAnimations[0];

    {
        aiMatrix4x4 NodeTransformation(node->mTransformation);

        const aiNodeAnim* pNodeAnim = findNodeAnim(pAnimation, nodeName);

        if (pNodeAnim)
        {
            std::map<float, glm::vec3> keyTimeScale;
            for (uint32_t i = 0; i < pNodeAnim->mNumScalingKeys; i++)
            {
                keyTimeScale[pNodeAnim->mScalingKeys[i].mTime] = aiVec3DToGLM(&pNodeAnim->mScalingKeys[i].mValue);
            }

            std::map<float, aiQuaternion> keyTimeQuat;
            for (uint32_t i = 0; i < pNodeAnim->mNumRotationKeys; i++)
            {
                keyTimeQuat[pNodeAnim->mRotationKeys[i].mTime] = pNodeAnim->mRotationKeys[i].mValue;
            }

            std::map<float, glm::vec3> keyTimePos;
            for (uint32_t i = 0; i < pNodeAnim->mNumPositionKeys; i++)
            {
                keyTimePos[pNodeAnim->mPositionKeys[i].mTime] = aiVec3DToGLM(&pNodeAnim->mPositionKeys[i].mValue);
            }

            AnimationKeyData animKeyData = { keyTimeScale, keyTimeQuat, keyTimePos };
            currentNode = new AnimNode(nodeName, animKeyData, node->mNumChildren);
        }
        else
        {
            currentNode = new AnimNode(nodeName, aiMatrix4x4ToGlm(&NodeTransformation), node->mNumChildren);
        }
    }

    currentNode->resizeChildren(node->mNumChildren);
    parentNode->setChild(childIdx, currentNode);

    for (uint32_t i = 0; i < node->mNumChildren; i++)
    {
        ReadNodeHeirarchy(scene, node->mChildren[i], currentNode, i, model);
    }
}

void FileManager::ReadNodeHeirarchy(const aiScene* scene, aiNode* node
    , AnimNode* parentNode, FbxModel* model,Animation* animation)
{
    AnimNode* currentNode;

    std::string nodeName(node->mName.data);

    const aiAnimation* pAnimation = scene->mAnimations[0];

    {
        aiMatrix4x4 NodeTransformation(node->mTransformation);
        const aiNodeAnim* pNodeAnim = findNodeAnim(pAnimation, nodeName);

        if (pNodeAnim)
        {
            std::map<float, glm::vec3> keyTimeScale;
            for (uint32_t i = 0; i < pNodeAnim->mNumScalingKeys; i++)
            {
                keyTimeScale[pNodeAnim->mScalingKeys[i].mTime] = aiVec3DToGLM(&pNodeAnim->mScalingKeys[i].mValue);
            }

            std::map<float, aiQuaternion> keyTimeQuat;
            for (uint32_t i = 0; i < pNodeAnim->mNumRotationKeys; i++)
            {
                keyTimeQuat[pNodeAnim->mRotationKeys[i].mTime] = pNodeAnim->mRotationKeys[i].mValue;
            }

            std::map<float, glm::vec3> keyTimePos;
            for (uint32_t i = 0; i < pNodeAnim->mNumPositionKeys; i++)
            {
                keyTimePos[pNodeAnim->mPositionKeys[i].mTime] = aiVec3DToGLM(&pNodeAnim->mPositionKeys[i].mValue);
            }

            AnimationKeyData animKeyData = { keyTimeScale, keyTimeQuat, keyTimePos };
            currentNode = new AnimNode(nodeName, animKeyData, node->mNumChildren);
        }
        else
        {
            currentNode = new AnimNode(nodeName, aiMatrix4x4ToGlm(&NodeTransformation), node->mNumChildren);
        }
    }

    currentNode->resizeChildren(node->mNumChildren);
    for (uint32_t i = 0; i < node->mNumChildren; i++)
    {
        ReadNodeHeirarchy(scene, node->mChildren[i], currentNode, i, model);
    }

    animation->setRootNode(currentNode);
}

std::shared_ptr<Animation> FileManager::loadAnimations(FbxModel* fbxModel, OBJECT obj)
{
    if (Storage::GetInstance()->containAnimation(obj))
    {
        return Storage::GetInstance()->getAnimation(obj);
    }

    void* ptr = nullptr;
    int size = 0;
    loadFbxModel(getModelResource(obj), &ptr, size);

    const aiScene* scene = importer.ReadFileFromMemory(ptr, size, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices);

    Animation* animation = new Animation(
            scene->mAnimations[0]->mTicksPerSecond
            , scene->mAnimations[0]->mDuration);
    animation->setGlobalInverseTransform(fbxModel->getGlobalInverseTransform());

    loadAnimation(scene, fbxModel,animation);

    Storage::GetInstance()->addAnimation(obj, animation);
    return Storage::GetInstance()->getAnimation(obj);
}

void FileManager::loadAnimation(const aiScene* scene,FbxModel* model,Animation* animation)
{
    AnimNode* rootNode = nullptr;

    ReadNodeHeirarchy(scene,scene->mRootNode,rootNode,model,animation);
}

void FileManager::loadPoses(FbxModel* fbxModel)
{
    void* ptr = nullptr;
    int size = 0;
    loadFbxModel(164, &ptr, size);

    const aiScene* scene = importer.ReadFileFromMemory(ptr, size, aiProcess_SortByPType | aiProcess_PopulateArmatureData);

    loadPose(scene, fbxModel);
}

void FileManager::ReadNodeHeirarchy(const aiScene* scene, const aiNode* node,aiMatrix4x4 matrix, std::array<glm::mat4, 250>& matrixArray, FbxModel* fbxModel)
{
    std::string nodeName(node->mName.data);

    aiMatrix4x4 NodeTransformation(node->mTransformation);

    matrix = matrix * NodeTransformation;

    if (fbxModel->containBone(nodeName))
    {
        int boneID = fbxModel->getBoneToMap(nodeName);
        matrixArray[boneID] = aiMatrix4x4ToGlm(&matrix) * fbxModel->getBoneOffset(boneID);
    }

    for (uint32_t i = 0; i < node->mNumChildren; i++)
    {
        ReadNodeHeirarchy(scene, node->mChildren[i], matrix,matrixArray,fbxModel);
    }
}

void FileManager::ReadNodeHeirarchy(const aiScene* scene, const aiNode* node, std::array<glm::mat4, 250>& matrixArray, FbxModel* fbxModel)
{
    std::string nodeName(node->mName.data);

    aiMatrix4x4 NodeTransformation(node->mTransformation);

    if (fbxModel->containBone(nodeName))
    {
        int boneID = fbxModel->getBoneToMap(nodeName);
        matrixArray[boneID] = aiMatrix4x4ToGlm(&NodeTransformation) * fbxModel->getBoneOffset(boneID);
    }

    for (uint32_t i = 0; i < node->mNumChildren; i++)
    {
        ReadNodeHeirarchy(scene, node->mChildren[i],NodeTransformation,matrixArray,fbxModel);
    }
}

void FileManager::loadPose(const aiScene* scene, FbxModel* fbxModel)
{
    std::shared_ptr<Pose> pose = std::shared_ptr<Pose>(new Pose());

    std::array<glm::mat4, 250> boneMatrixArray;
    std::fill(boneMatrixArray.begin(), boneMatrixArray.end(), glm::mat4(1.0f));

    ReadNodeHeirarchy(scene, scene->mRootNode, boneMatrixArray,fbxModel);

    pose->setPoseMatrix(boneMatrixArray);

    fbxModel->setPose("idle", pose);
}

std::string FileManager::splitFileName(std::string filePath)
{
    int pos = filePath.rfind('/');
    filePath = filePath.substr(pos + 1, filePath.length() - pos);

    return filePath;
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
            path = splitFileName(path);
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
    if (path == "Remy_Top_Diffuse.png")
    {
        return IDB_PNG2;
    }
    else if (path == "Remy_Body_Diffuse.png")
    {
        return IDB_PNG3;
    }
    else if (path == "Remy_Bottom_Diffuse.png")
    {
        return IDB_PNG4;
    }
    else if (path == "Remy_Hair_Diffuse.png")
    {
        return IDB_PNG5;
    }
    else if (path == "Remy_Shoes_Diffuse.png")
    {
        return IDB_PNG6;
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