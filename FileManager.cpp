#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT


#include"FileManager.h"

FileManager* FileManager::fileManager = nullptr;

FileManager::FileManager()
{
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

std::shared_ptr<GltfModel> FileManager::loadModel(OBJECT obj)
{
    Storage* storage = Storage::GetInstance();
    if (storage->containModel(obj))
    {
        return storage->getFbxModel(obj);
    }

    void* ptr = nullptr;
    int size = 0;
    //loadFbxModel(getModelResource(obj), &ptr, size);

    //"C:/Users/sukai/Documents/VulkanGame/models/test_out/test.gltf"
    tinygltf::Model gltfModel;
    tinygltf::TinyGLTF gltfContext;
    std::string error;
    std::string warning;
    bool binary = false;
    binary = gltfContext.LoadASCIIFromFile(&gltfModel, &error, &warning, "C:/Users/sukai/Downloads/SittingLaughing_out/SittingLaughing.gltf");
    const tinygltf::Scene& scene = gltfModel.scenes[gltfModel.defaultScene > -1 ? gltfModel.defaultScene : 0];//デフォルトシーンがあればそれを、なければ最初のシーン

    allVertNum = 0;
    pivot = glm::vec3(0.0);
    minPos = glm::vec3(1000.0f, 1000.0f, 1000.0f);
    maxPos = glm::vec3(-1000.0f, -1000.0f, -1000.0f);

    GltfModel* model = loadGLTFModel(scene, gltfModel);

    model->setTotalVertexNum(allVertNum);
    model->setImageDataCount(imageDataCount);

    model->setMinMaxVertexPos(minPos, maxPos);

    imageDataCount = 0;

    //loadPoses(model);

    storage->addModel(obj, model);

    return storage->getFbxModel(obj);
}

GltfModel* FileManager::loadGLTFModel(const tinygltf::Scene& scene,const tinygltf::Model& gltfModel)
{
    GltfModel* model = new GltfModel();

    for (size_t i = 0; i < scene.nodes.size(); i++)
    {
        const tinygltf::Node node = gltfModel.nodes[scene.nodes[i]];
        processMesh(node, gltfModel, model);
    }

    processMeshBones(scene, gltfModel, model);

    return model;
}

void FileManager::processMesh(const tinygltf::Node& currentNode, const tinygltf::Model gltfModel, GltfModel* model)
{
    if (currentNode.mesh > -1)
    {
        const tinygltf::Mesh mesh = gltfModel.meshes[currentNode.mesh];
        Meshes* meshes = new Meshes();
        int indexStart = 0;

        for (unsigned int i = 0; i < mesh.primitives.size(); i++)
        {
            const tinygltf::Primitive glPrimitive = mesh.primitives[j];

            processPrimitive(meshes,indexStart,glPrimitive,gltfModel);
        }

        model->addMeshes(meshes);
    }

    for (int i = 0; i < currentNode.children.size(); i++)
    {
        processMesh(currentNode, gltfModel.nodes[currentNode.children[i]], gltfModel, model);
    }
}


void FileManager::processMesh(const tinygltf::Node& parentNode, const tinygltf::Node& currentNode,const tinygltf::Model gltfModel, GltfModel* model)
{
    if (currentNode.mesh > -1)
    {
        const tinygltf::Mesh mesh = gltfModel.meshes[currentNode.mesh];
        Meshes* meshes = new Meshes();
        int indexStart = 0;

        for (unsigned int i = 0; i < mesh.primitives.size(); i++)
        {
            const tinygltf::Primitive glPrimitive = mesh.primitives[i];

            processPrimitive(meshes, indexStart, glPrimitive, gltfModel);
        }

        model->addMeshes(meshes);
    }

    for (int i = 0; i < currentNode.children.size(); i++)
    {
        processMesh(currentNode, gltfModel.nodes[currentNode.children[i]], gltfModel, model);
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

void FileManager::processPrimitive(Meshes* meshes,int& indexStart, tinygltf::Primitive glPrimitive, tinygltf::Model glModel)
{
    const float* bufferPos = nullptr;
    int vertexCount;

    const float* bufferNormals = nullptr;
    const float* bufferTexCoordSet0 = nullptr;
    const float* bufferTexCoordSet1 = nullptr;
    const float* bufferColorSet0 = nullptr;
    const void* bufferJoints = nullptr;
    const float* bufferWeights = nullptr;

    int posByteStride;
    int normByteStride;
    int uv0ByteStride;
    int uv1ByteStride;
    int color0ByteStride;
    int jointByteStride;
    int weightByteStride;

    int jointComponentType;

    const tinygltf::Accessor& posAccessor = glModel.accessors[glPrimitive.attributes.find("POSITION")->second];//プリミティブから位置を表すバッファにアクセスするための変数
    const tinygltf::BufferView& posView = glModel.bufferViews[posAccessor.bufferView];//アクセッサーを介してバッファーから値を読み取る
    bufferPos = reinterpret_cast<const float*>(&(glModel.buffers[posView.buffer].data[posAccessor.byteOffset + posView.byteOffset]));//それぞれのオフセットを足して参照する位置を調整する
    vertexCount = static_cast<uint32_t>(posAccessor.count);//このプリミティブの持つ頂点の数
    posByteStride = posAccessor.ByteStride(posView) ? (posAccessor.ByteStride(posView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);//頂点を読み取る際の頂点のデータの幅を取得

    //以下頂点座標と同じように、取得していく

    if (glPrimitive.attributes.find("NORMAL") != glPrimitive.attributes.end()) {
        const tinygltf::Accessor& normAccessor = glModel.accessors[glPrimitive.attributes.find("NORMAL")->second];
        const tinygltf::BufferView& normView = glModel.bufferViews[normAccessor.bufferView];
        bufferNormals = reinterpret_cast<const float*>(&(glModel.buffers[normView.buffer].data[normAccessor.byteOffset + normView.byteOffset]));
        normByteStride = normAccessor.ByteStride(normView) ? (normAccessor.ByteStride(normView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);
    }

    // UVs
    if (glPrimitive.attributes.find("TEXCOORD_0") != glPrimitive.attributes.end()) {
        const tinygltf::Accessor& uvAccessor = glModel.accessors[glPrimitive.attributes.find("TEXCOORD_0")->second];
        const tinygltf::BufferView& uvView = glModel.bufferViews[uvAccessor.bufferView];
        bufferTexCoordSet0 = reinterpret_cast<const float*>(&(glModel.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));
        uv0ByteStride = uvAccessor.ByteStride(uvView) ? (uvAccessor.ByteStride(uvView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC2);
    }
    if (glPrimitive.attributes.find("TEXCOORD_1") != glPrimitive.attributes.end()) {
        const tinygltf::Accessor& uvAccessor = glModel.accessors[glPrimitive.attributes.find("TEXCOORD_1")->second];
        const tinygltf::BufferView& uvView = glModel.bufferViews[uvAccessor.bufferView];
        bufferTexCoordSet1 = reinterpret_cast<const float*>(&(glModel.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));
        uv1ByteStride = uvAccessor.ByteStride(uvView) ? (uvAccessor.ByteStride(uvView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC2);
    }

    // Vertex colors
    if (glPrimitive.attributes.find("COLOR_0") != glPrimitive.attributes.end()) {
        const tinygltf::Accessor& accessor = glModel.accessors[glPrimitive.attributes.find("COLOR_0")->second];
        const tinygltf::BufferView& view = glModel.bufferViews[accessor.bufferView];
        bufferColorSet0 = reinterpret_cast<const float*>(&(glModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
        color0ByteStride = accessor.ByteStride(view) ? (accessor.ByteStride(view) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);
    }

    // Skinning
    // Joints
    if (glPrimitive.attributes.find("JOINTS_0") != glPrimitive.attributes.end()) {
        const tinygltf::Accessor& jointAccessor = glModel.accessors[glPrimitive.attributes.find("JOINTS_0")->second];
        const tinygltf::BufferView& jointView = glModel.bufferViews[jointAccessor.bufferView];
        bufferJoints = &(glModel.buffers[jointView.buffer].data[jointAccessor.byteOffset + jointView.byteOffset]);
        jointComponentType = jointAccessor.componentType;
        jointByteStride = jointAccessor.ByteStride(jointView) ? (jointAccessor.ByteStride(jointView) / tinygltf::GetComponentSizeInBytes(jointComponentType)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC4);
    }

    if (glPrimitive.attributes.find("WEIGHTS_0") != glPrimitive.attributes.end()) {
        const tinygltf::Accessor& weightAccessor = glModel.accessors[glPrimitive.attributes.find("WEIGHTS_0")->second];
        const tinygltf::BufferView& weightView = glModel.bufferViews[weightAccessor.bufferView];
        bufferWeights = reinterpret_cast<const float*>(&(glModel.buffers[weightView.buffer].data[weightAccessor.byteOffset + weightView.byteOffset]));
        weightByteStride = weightAccessor.ByteStride(weightView) ? (weightAccessor.ByteStride(weightView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC4);
    }

    bool hasSkin = false;
    if (bufferJoints && bufferWeights)
    {
        hasSkin = true;
    }

    for (size_t v = 0; v < posAccessor.count; v++) {
        Vertex vert;
        vert.pos = glm::vec3(glm::make_vec3(&bufferPos[v * posByteStride]));
        vert.normal = glm::normalize(glm::vec3(bufferNormals ? glm::make_vec3(&bufferNormals[v * normByteStride]) : glm::vec3(0.0f)));
        vert.texCoord = bufferTexCoordSet0 ? glm::make_vec2(&bufferTexCoordSet0[v * uv0ByteStride]) : glm::vec3(0.0f);
        //vert.uv1 = bufferTexCoordSet1 ? glm::make_vec2(&bufferTexCoordSet1[v * uv1ByteStride]) : glm::vec3(0.0f);
        vert.color = bufferColorSet0 ? glm::make_vec4(&bufferColorSet0[v * color0ByteStride]) : glm::vec4(1.0f);

        if (hasSkin)
        {
            switch (jointComponentType) {
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
                const uint16_t* buf = static_cast<const uint16_t*>(bufferJoints);
                vert.boneID1 = glm::uvec4(glm::make_vec4(&buf[v * jointByteStride]));
                break;
            }
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
                const uint8_t* buf = static_cast<const uint8_t*>(bufferJoints);
                vert.boneID1 = glm::vec4(glm::make_vec4(&buf[v * jointByteStride]));
                break;
            }
            default:
                // Not supported by spec
                std::cerr << "Joint component type " << jointComponentType << " not supported!" << std::endl;
                break;
            }
        }
        else {
            vert.boneID1 = glm::vec4(0.0f);
        }
        vert.weight1 = hasSkin ? glm::make_vec4(&bufferWeights[v * weightByteStride]) : glm::vec4(0.0f);
        // Fix for all zero weights
        if (glm::length(vert.weight1) == 0.0f) {
            vert.weight1 = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
        }

        meshes->pushBackVertex(vert);
    }

    int indexCount;
    if (glPrimitive.indices > -1)
    {
        const tinygltf::Accessor& accessor = glModel.accessors[glPrimitive.indices > -1 ? glPrimitive.indices : 0];
        const tinygltf::BufferView& bufferView = glModel.bufferViews[accessor.bufferView];
        const tinygltf::Buffer& buffer = glModel.buffers[bufferView.buffer];

        indexCount = static_cast<int>(accessor.count);
        const void* dataPtr = &(buffer.data[accessor.byteOffset + bufferView.byteOffset]);

        switch (accessor.componentType) {
        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
            const uint32_t* buf = static_cast<const uint32_t*>(dataPtr);
            for (size_t index = 0; index < accessor.count; index++) {
                meshes->pushBackIndex(buf[index] + indexStart);
            }
            break;
        }
        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
            const uint16_t* buf = static_cast<const uint16_t*>(dataPtr);
            for (size_t index = 0; index < accessor.count; index++) {
                meshes->pushBackIndex(buf[index] + indexStart);
            }
            break;
        }
        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
            const uint8_t* buf = static_cast<const uint8_t*>(dataPtr);
            for (size_t index = 0; index < accessor.count; index++) {
                meshes->pushBackIndex(buf[index] + indexStart);
            }
            break;
        }
        default:
            std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
            return;
        }
    }

    std::shared_ptr<Material> material = processMaterial(glModel, glPrimitive.material);

    Primitive primitive = { indexStart,indexCount,material };

    meshes->pushBackPrimitive(primitive);

    indexStart += indexCount;
}

void FileManager::processMeshBones(const tinygltf::Scene& scene, const tinygltf::Model gltfModel, GltfModel* model)
{
    tinygltf::Skin skin = gltfModel.skins[0];
    if (skin.skeleton <= -1)
    {
        return;
    }

    for (int i = 0; i < skin.joints.size(); i++)
    {
        std::string nodeName = gltfModel.nodes[skin.joints[i]];

    }
}

/*
void FileManager::processMeshBones(const FbxMesh* mesh, uint32_t meshNumVertices, GltfModel* gltfModel,Meshes* meshes)
{
    FbxSkin* skin = static_cast<FbxSkin*>(mesh->GetDeformer(0, FbxDeformer::eSkin));
    int boneCount = skin->GetClusterCount();
    for (uint32_t i = 0; i < boneCount; i++)
    {
        loadSingleBone(skin->GetCluster(i), meshNumVertices, gltfModel, meshes);
    }
}

void FileManager::loadSingleBone(const FbxCluster* bone, uint32_t meshNumVertices, GltfModel* gltfModel, Meshes* meshes)
{
    int boneID = getBoneID(bone, gltfModel);

    glm::mat4 offset;
    FbxAMatrix transform,matrix,linkMatrix;
    bone->GetTransformLinkMatrix(linkMatrix);
    bone->GetTransformMatrix(matrix);
    transform = linkMatrix.Inverse() * matrix;
    offset = FbxMatrix4x4ToGlm(transform);
    gltfModel->setBoneInfo(boneID, offset);

    int* vertexArray = bone->GetControlPointIndices();
    double* weightArray = bone->GetControlPointWeights();
    for (uint32_t i = 0; i < bone->GetControlPointIndicesCount(); i++)
    {
        meshes->addBoneData(vertexArray[i], boneID, weightArray[i]);
    }
}
*/

int FileManager::getBoneID(const std::string boneName, GltfModel* gltfModel)
{
    return gltfModel->getBoneToMap(boneName);
}

/*
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
    , AnimNode* parentNode, unsigned int childIdx, GltfModel* gltfModel)
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
        ReadNodeHeirarchy(scene, node->mChildren[i], currentNode, i, gltfModel);
    }
}

void FileManager::ReadNodeHeirarchy(FbxAnimLayer* layer, FbxNode* node
    , AnimNode* parentNode, GltfModel* gltfModel, Animation* animation)
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
        ReadNodeHeirarchy(scene, node->mChildren[i], currentNode, i, gltfModel);
    }

    animation->setRootNode(currentNode);
}

std::shared_ptr<Animation> FileManager::loadAnimations(GltfModel* model, OBJECT obj)
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
    animation->setGlobalInverseTransform(model->getGlobalInverseTransform());

    scene->SetCurrentAnimationStack(scene->GetSrcObject<FbxAnimStack>(0));

    loadAnimation(scene, model, animation);

    Storage::GetInstance()->addAnimation(obj, animation);
    return Storage::GetInstance()->getAnimation(obj);
}

void FileManager::loadAnimation(const FbxScene* scene, GltfModel* gltfModel, Animation* animation)
{
    AnimNode* rootNode = nullptr;

    FbxAnimStack* stack = scene->GetSrcObject<FbxAnimStack>(0);

    ReadNodeHeirarchy(stack->GetSrcObject<FbxAnimLayer>(0), scene->GetRootNode(), rootNode, gltfModel, animation);
}

void FileManager::loadPoses(GltfModel* model)
{
    void* ptr = nullptr;
    int size = 0;
    loadFbxModel(164, &ptr, size);

    const FbxScene* scene = importer.ReadFileFromMemory(ptr, size, aiProcess_SortByPType | aiProcess_PopulateArmatureData);

    loadPose(scene, model);
}

void FileManager::ReadNodeHeirarchy(const FbxScene* scene, const FbxNode* node,aiMatrix4x4 matrix, std::array<glm::mat4, 250>& matrixArray, GltfModel* model)
{
    std::string nodeName(node->mName.data);

    aiMatrix4x4 NodeTransformation(node->mTransformation);

    matrix = matrix * NodeTransformation;

    if (model->containBone(nodeName))
    {
        int boneID = model->getBoneToMap(nodeName);
        matrixArray[boneID] = aiMatrix4x4ToGlm(&matrix) * model->getBoneOffset(boneID);
    }

    for (uint32_t i = 0; i < node->mNumChildren; i++)
    {
        ReadNodeHeirarchy(scene, node->mChildren[i], matrix,matrixArray,model);
    }
}

void FileManager::ReadNodeHeirarchy(const FbxScene* scene, const FbxNode* node, std::array<glm::mat4, 250>& matrixArray, GltfModel* model)
{
    std::string nodeName(node->mName.data);

    aiMatrix4x4 NodeTransformation(node->mTransformation);

    if (model->containBone(nodeName))
    {
        int boneID = model->getBoneToMap(nodeName);
        matrixArray[boneID] = aiMatrix4x4ToGlm(&NodeTransformation) * model->getBoneOffset(boneID);
    }

    for (uint32_t i = 0; i < node->mNumChildren; i++)
    {
        ReadNodeHeirarchy(scene, node->mChildren[i],NodeTransformation,matrixArray,model);
    }
}

void FileManager::loadPose(const FbxScene* scene, GltfModel* model)
{
    std::shared_ptr<Pose> pose = std::shared_ptr<Pose>(new Pose());

    std::array<glm::mat4, 250> boneMatrixArray;
    std::fill(boneMatrixArray.begin(), boneMatrixArray.end(), glm::mat4(1.0f));

    ReadNodeHeirarchy(scene, scene->mRootNode, boneMatrixArray,model);

    pose->setPoseMatrix(boneMatrixArray);

    model->setPose("idle", pose);
}
*/

std::string FileManager::splitFileName(std::string filePath)
{
    int pos = filePath.rfind('/');
    filePath = filePath.substr(pos + 1, filePath.length() - pos);

    return filePath;
}

std::shared_ptr<Material> FileManager::processMaterial(tinygltf::Model gltfModel,int materialIndex)
{
    std::shared_ptr<Material> material = std::shared_ptr<Material>(new Material());

    tinygltf::Material mat = gltfModel.materials[materialIndex];

    if (mat.values.find("baseColorTexture") != mat.values.end()) 
    {
        tinygltf::Image image = gltfModel.images[gltfModel.textures[mat.values["baseColorTexture"].TextureIndex()].source];

        std::shared_ptr<ImageData> imageData = std::shared_ptr<ImageData>(new ImageData(image.width, image.height, image.component, image.image.data()));
        material->setImageData(mat.values["baseColorTexture"].TextureTexCoord(),imageData);
    }
    if (mat.values.find("baseColorFactor") != mat.values.end()) {
        material->setDiffuse(glm::make_vec3(mat.values["baseColorFactor"].ColorFactor().data()));
    }

    return material;
}

/*
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
*/

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

/*
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
*/