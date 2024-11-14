#include"FileManager.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT

FileManager* FileManager::fileManager = nullptr;

FileManager::FileManager()
{
}

glm::vec3 FileManager::aiVec3DToGLM(const aiVector3D& vec)
{
    return glm::vec3(vec.x, vec.y, vec.z);
}

glm::mat4 FileManager::FbxMatrix4x4ToGlm(FbxAMatrix& from)
{
    from.Transpose();
    glm::mat4 matrix;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            matrix[i][j] = from.Get(i, j);
        }
    }

    return matrix;
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

    //"C:/Users/sukai/Documents/VulkanGame/models/test_out/test.gltf"
    tinygltf::Model gltfModel;
    tinygltf::TinyGLTF gltfContext;
    std::string error;
    std::string warning;
    bool binary = false;
    binary = gltfContext.LoadASCIIFromFile(&gltfModel, &error, &warning, "C:/Users/sukai/Documents/VulkanGame/models/test_out/test.gltf");
    const tinygltf::Scene& scene = gltfModel.scenes[gltfModel.defaultScene > -1 ? gltfModel.defaultScene : 0];//デフォルトシーンがあればそれを、なければ最初のシーン

    allVertNum = 0;
    pivot = glm::vec3(0.0);
    minPos = glm::vec3(1000.0f, 1000.0f, 1000.0f);
    maxPos = glm::vec3(-1000.0f, -1000.0f, -1000.0f);

    for (size_t i = 0; i < scene.nodes.size(); i++)
    {
        const tinygltf::Node node = gltfModel.nodes[scene.nodes[i]];
        processNode(node, gltfModel,fbxModel);
    }

    fbxModel->setTotalVertexNum(allVertNum);
    fbxModel->setImageDataCount(imageDataCount);

    fbxModel->setMinMaxVertexPos(minPos, maxPos);

    imageDataCount = 0;

    //loadPoses(fbxModel);

    storage->addModel(obj, fbxModel);

    return storage->getFbxModel(obj);
}

void FileManager::processNode(const tinygltf::Node& currentNode, const tinygltf::Model model, FbxModel* fbxModel)
{
    if (currentNode.mesh > -1)
    {
        const tinygltf::Mesh mesh = model.meshes[currentNode.mesh];

        Meshes * meshes = processFbxMesh(scene->GetSrcObject<FbxMesh>(i), scene, allVertNum, model);

        for (unsigned int i = 0; i < meshNodeCount; i++)
        {

            allVertNum += scene->GetSrcObject<FbxMesh>(i)->GetControlPointsCount();

            std::shared_ptr<Material> material = processMaterial(scene->GetSrcObject<FbxMesh>(i), scene);
            if (material->hasImageData())
            {
                //imageDataCount++;
            }

            meshes->setMaterial(material);
        }

        fbxModel->addMeshes(meshes);
    }

    for (int i = 0; i < currentNode.children.size(); i++)
    {
        processNode(currentNode, model.nodes[currentNode.children[i]], model, fbxModel);
    }
}


void FileManager::processNode(const tinygltf::Node& parentNode, const tinygltf::Node& currentNode,const tinygltf::Model model, FbxModel* fbxModel)
{
    if (currentNode.mesh > -1)
    {
        const tinygltf::Mesh mesh = model.meshes[node.mesh]

        Meshes* meshes = processFbxMesh(scene->GetSrcObject<FbxMesh>(i), scene, allVertNum, model);

        for (unsigned int i = 0; i < meshNodeCount; i++)
        {

            allVertNum += scene->GetSrcObject<FbxMesh>(i)->GetControlPointsCount();

            std::shared_ptr<Material> material = processMaterial(scene->GetSrcObject<FbxMesh>(i), scene);
            if (material->hasImageData())
            {
                //imageDataCount++;
            }

            meshes->setMaterial(material);
        }

        fbxModel->addMeshes(meshes);
    }

    for (int i = 0; i < currentNode.children.size(); i++)
    {
        processNode(currentNode, model.nodes[currentNode.children[i]], model,fbxModel);
    }
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

Meshes* FileManager::processFbxMesh(const FbxMesh* mesh, const FbxScene* scene, uint32_t meshNumVertices, FbxModel* model)
{
    Meshes* meshes = new Meshes();

    FbxNode* node = mesh->GetNode();

    glm::vec3 translate, rotation, scale;
    FbxDouble3 value;
    float deg;
    value = node->LclTranslation.Get();
    translate[0] = value[0]; translate[1] = value[1]; translate[2] = value[2];
    value = node->LclRotation.Get();
    rotation[0] = value[0]; rotation[1] = value[1]; rotation[2] = value[2];
    deg = node->LclRotation.Get()[3];
    value = node->LclScaling.Get();
    scale[0] = value[0]; scale[1] = value[1]; scale[2] = value[2];

    glm::mat4 transform(glm::translate(glm::mat4(1.0f),translate) * glm::rotate(glm::mat4(1.0f), glm::radians(deg), rotation) * glm::scale(glm::mat4(1.0f), scale));
    meshes->setLocalTransform(transform);

    int polygonVertexCount = mesh->GetPolygonVertexCount();
    int* indices = mesh->GetPolygonVertices();
    FbxArray<FbxVector4> normals;
    mesh->GetPolygonVertexNormals(normals);

    FbxVector4 vertices;
    for (unsigned int i = 0; i < polygonVertexCount; i++)
    {
        Vertex vertex;

        vertices = mesh->GetControlPointAt(indices[i]);
        vertex.pos = glm::vec3(vertices[0], vertices[1], vertices[2]);
        //vertex.normal = glm::vec3(normals[i][0], normals[i][1], normals[i][2]);

        pivot += vertex.pos;

        /*

        if (mesh->HasTextureCoords(0))
        {
            vertex.texCoord = glm::vec2(mesh->mTextureCoords[0][i].x,1 - mesh->mTextureCoords[0][i].y);
        }
        else
        {
            vertex.texCoord = glm::vec2(0.0f, 0.0f);
        }

        */

        calcMinMaxVertexPos(vertex.pos);

        meshes->pushBackVertex(vertex);
    }

    if (mesh->GetDeformerCount() != 0)
    {
        processMeshBones(mesh, meshNumVertices, model, meshes);
    }
    else
    {
        std::string newBoneName = mesh->GetName();
        int boneID = getBoneID(newBoneName, model);
        model->setBoneInfo(boneID, glm::mat4(1.0f));

        for (int i = 0; i < polygonVertexCount; i++)
        {
            meshes->addBoneData(i, boneID, 1.0f);
        }
    }

    for (unsigned int i = 0; i < mesh->GetPolygonVertexCount(); i++)
    {
        meshes->pushBackIndex(i);
    }

    return meshes;
}

void FileManager::processMeshBones(const FbxMesh* mesh, uint32_t meshNumVertices, FbxModel* model,Meshes* meshes)
{
    FbxSkin* skin = static_cast<FbxSkin*>(mesh->GetDeformer(0, FbxDeformer::eSkin));
    int boneCount = skin->GetClusterCount();
    for (uint32_t i = 0; i < boneCount; i++)
    {
        loadSingleBone(skin->GetCluster(i), meshNumVertices, model, meshes);
    }
}

void FileManager::loadSingleBone(const FbxCluster* bone, uint32_t meshNumVertices, FbxModel* model, Meshes* meshes)
{
    int boneID = getBoneID(bone, model);

    glm::mat4 offset;
    FbxAMatrix transform,matrix,linkMatrix;
    bone->GetTransformLinkMatrix(linkMatrix);
    bone->GetTransformMatrix(matrix);
    transform = linkMatrix.Inverse() * matrix;
    offset = FbxMatrix4x4ToGlm(transform);
    model->setBoneInfo(boneID, offset);

    int* vertexArray = bone->GetControlPointIndices();
    double* weightArray = bone->GetControlPointWeights();
    for (uint32_t i = 0; i < bone->GetControlPointIndicesCount(); i++)
    {
        meshes->addBoneData(vertexArray[i], boneID, weightArray[i]);
    }
}

int FileManager::getBoneID(const FbxCluster* bone, FbxModel* model)
{
    std::string boneName(bone->GetName());

    return model->getBoneToMap(boneName);
}

int FileManager::getBoneID(const std::string boneName, FbxModel* model)
{
    return model->getBoneToMap(boneName);
}

const FbxNodeAnim* FileManager::findNodeAnim(const aiAnimation* pAnimation, std::string nodeName)
{
    for (uint32_t i = 0; i < pAnimation->mNumChannels; i++)
    {
        const FbxNodeAnim* pNodeAnim = pAnimation->mChannels[i];

        if (std::string(pNodeAnim->mNodeName.data) == nodeName)
        {
            return pNodeAnim;
        }
    }

    return nullptr;
}

void FileManager::ReadNodeHeirarchy(FbxAnimLayer* layer, FbxNode* node
    , AnimNode* parentNode, unsigned int childIdx, FbxModel* model)
{
    AnimNode* currentNode;

    std::string nodeName(node->mName.data);

    const aiAnimation* pAnimation = scene->mAnimations[0];

    {
        aiMatrix4x4 NodeTransformation(node->mTransformation);

        const FbxNodeAnim* pNodeAnim = findNodeAnim(pAnimation, nodeName);

        if (pNodeAnim)
        {
            std::map<float, glm::vec3> keyTimeScale;
            for (uint32_t i = 0; i < pNodeAnim->mNumScalingKeys; i++)
            {
                keyTimeScale[pNodeAnim->mScalingKeys[i].mTime] = aiVec3DToGLM(pNodeAnim->mScalingKeys[i].mValue);
            }

            std::map<float, aiQuaternion> keyTimeQuat;
            for (uint32_t i = 0; i < pNodeAnim->mNumRotationKeys; i++)
            {
                keyTimeQuat[pNodeAnim->mRotationKeys[i].mTime] = pNodeAnim->mRotationKeys[i].mValue;
            }

            std::map<float, glm::vec3> keyTimePos;
            for (uint32_t i = 0; i < pNodeAnim->mNumPositionKeys; i++)
            {
                keyTimePos[pNodeAnim->mPositionKeys[i].mTime] = aiVec3DToGLM(pNodeAnim->mPositionKeys[i].mValue);
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

void FileManager::ReadNodeHeirarchy(FbxAnimLayer* layer, FbxNode* node
    , AnimNode* parentNode, FbxModel* model, Animation* animation)
{
    bool hasAnimCurve = false;

    AnimNode* currentNode;
    FbxAnimCurve* curve;

    std::string nodeName(node->GetName());

    curve = node->LclScaling.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_X);
    if (curve)
    {
        std::map<float, glm::vec3> keyTimeScale;
        for (uint32_t i = 0; i < pNodeAnim->mNumScalingKeys; i++)
        {
            keyTimeScale[pNodeAnim->mScalingKeys[i].mTime] = aiVec3DToGLM(pNodeAnim->mScalingKeys[i].mValue);
        }
    }

    curve = node->LclScaling.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_X);
    if (curve)
    {
        std::map<float, aiQuaternion> keyTimeQuat;
        for (uint32_t i = 0; i < pNodeAnim->mNumRotationKeys; i++)
        {
            keyTimeQuat[pNodeAnim->mRotationKeys[i].mTime] = pNodeAnim->mRotationKeys[i].mValue;
        }
    }

    curve = node->LclScaling.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_X);
    if (curve)
    {
        std::map<float, glm::vec3> keyTimePos;
        for (uint32_t i = 0; i < pNodeAnim->mNumPositionKeys; i++)
        {
            keyTimePos[pNodeAnim->mPositionKeys[i].mTime] = aiVec3DToGLM(pNodeAnim->mPositionKeys[i].mValue);
        }
    }

    if (hasAnimCurve)
    {
        AnimationKeyData animKeyData = { keyTimeScale, keyTimeQuat, keyTimePos };
        currentNode = new AnimNode(nodeName, animKeyData, node->mNumChildren);
    }
    else
    {
        currentNode = new AnimNode(nodeName, aiMatrix4x4ToGlm(&NodeTransformation), node->mNumChildren);
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

    FbxScene* scene = FbxScene::Create(manager, "");
    FbxImporter* importer = FbxImporter::Create(manager, "");

    //importer->Initialize((char*)ptr,size, manager->GetIOSettings());
    importer->Initialize("C:/Users/sukai/Documents/VulkanGame/models/test.fbx", -1, manager->GetIOSettings());
    importer->Import(scene);
    importer->Destroy();

    //const FbxScene* scene = importer.ReadFileFromMemory(ptr, size, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices);

    FbxArray<FbxString*> names;
    scene->FillAnimStackNameArray(names);
    FbxTakeInfo* info = scene->GetTakeInfo(names[0]->Buffer());
    float offset = static_cast<float>(info->mImportOffset.Get());
    float startTime = static_cast<float>(info->mLocalTimeSpan.GetStart().Get());
    float endTime = static_cast<float>(info->mLocalTimeSpan.GetStop().Get());
    float duration = static_cast<float>(info->mLocalTimeSpan.GetDuration().Get());

    Animation* animation = new Animation(startTime,endTime,duration);
    animation->setGlobalInverseTransform(fbxModel->getGlobalInverseTransform());

    scene->SetCurrentAnimationStack(scene->GetSrcObject<FbxAnimStack>(0));

    loadAnimation(scene, fbxModel, animation);

    Storage::GetInstance()->addAnimation(obj, animation);
    return Storage::GetInstance()->getAnimation(obj);
}

void FileManager::loadAnimation(const FbxScene* scene, FbxModel* model, Animation* animation)
{
    AnimNode* rootNode = nullptr;

    FbxAnimStack* stack = scene->GetSrcObject<FbxAnimStack>(0);

    ReadNodeHeirarchy(stack->GetSrcObject<FbxAnimLayer>(0), scene->GetRootNode(), rootNode, model, animation);
}

/*
void FileManager::loadPoses(FbxModel* fbxModel)
{
    void* ptr = nullptr;
    int size = 0;
    loadFbxModel(164, &ptr, size);

    const FbxScene* scene = importer.ReadFileFromMemory(ptr, size, aiProcess_SortByPType | aiProcess_PopulateArmatureData);

    loadPose(scene, fbxModel);
}
*/

void FileManager::ReadNodeHeirarchy(const FbxScene* scene, const FbxNode* node,aiMatrix4x4 matrix, std::array<glm::mat4, 250>& matrixArray, FbxModel* fbxModel)
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

void FileManager::ReadNodeHeirarchy(const FbxScene* scene, const FbxNode* node, std::array<glm::mat4, 250>& matrixArray, FbxModel* fbxModel)
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

/*
void FileManager::loadPose(const FbxScene* scene, FbxModel* fbxModel)
{
    std::shared_ptr<Pose> pose = std::shared_ptr<Pose>(new Pose());

    std::array<glm::mat4, 250> boneMatrixArray;
    std::fill(boneMatrixArray.begin(), boneMatrixArray.end(), glm::mat4(1.0f));

    ReadNodeHeirarchy(scene, scene->mRootNode, boneMatrixArray,fbxModel);

    pose->setPoseMatrix(boneMatrixArray);

    fbxModel->setPose("idle", pose);
}
*/

std::string FileManager::splitFileName(std::string filePath)
{
    int pos = filePath.rfind('/');
    filePath = filePath.substr(pos + 1, filePath.length() - pos);

    return filePath;
}

std::shared_ptr<Material> FileManager::processMaterial(FbxMesh* mesh, const FbxScene* scene)
{
    std::shared_ptr<Material> material = std::shared_ptr<Material>(new Material());

    FbxLayerElementMaterial* fbxMaterial = mesh->GetElementMaterial(0);
    int index = fbxMaterial->GetIndexArray().GetAt(0);
    FbxSurfaceMaterial* surface_material = mesh->GetNode()->GetSrcObject<FbxSurfaceMaterial>(index);

    glm::vec3 v;

    FbxProperty prop = surface_material->FindProperty(FbxSurfaceMaterial::sDiffuse);
    glm::vec3 diffuse = glm::vec3(prop.Get<FbxDouble3>()[0], prop.Get<FbxDouble3>()[1], prop.Get<FbxDouble3>()[2]);
    material->setDiffuse(&v);

    if (prop.GetSrcObjectCount<FbxFileTexture>() > 0)
    {
        std::string path = prop.GetSrcObject<FbxFileTexture>(0)->GetRelativeFileName();;
        path = splitFileName(path);
        std::shared_ptr<ImageData> imageData = loadModelImage(path);

        material->setImageData(imageData);
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