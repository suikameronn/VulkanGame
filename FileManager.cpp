#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT

#include"FileManager.h"

FileManager* FileManager::fileManager = nullptr;

FileManager::FileManager()
{
}

//埋め込められたgltfモデルを取得する
int FileManager::getModelResource(GLTFOBJECT obj)
{
    switch (obj)
    {
    case GLTFOBJECT::gltfTEST:
        return IDR_MODEL1;
    case GLTFOBJECT::ASPHALT:
        return IDR_MODEL2;
    case GLTFOBJECT::LEATHER:
        return IDR_MODEL3;
    case GLTFOBJECT::CUBEMAP:
        return IDR_MODEL4;
    }

    return -1;
}

//埋め込まれたgltfモデルを取得する
void FileManager::loadgltfModel(int id, void** ptr, int& size)
{
    HRESULT hr = S_OK;
    HRSRC hrsrc = FindResource(NULL, MAKEINTRESOURCE(id), "MODEL");
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

//luaスクリプトからSceneクラスを介して呼び出される。新しいモデルを求められたときのみ、解析処理をする
std::shared_ptr<GltfModel> FileManager::loadModel(GLTFOBJECT obj)//3Dモデルを返す
{
    Storage* storage = Storage::GetInstance();
    if (storage->containModel(obj))//すでに3Dモデルをファイルから読み取り済みなら
    {
        return storage->getgltfModel(obj);//ストレージクラスから渡す
    }

    void* ptr = nullptr;
    int size = 0;
    loadgltfModel(getModelResource(obj),&ptr, size);//ファイルを取得する

    tinygltf::Model gltfModel;
    tinygltf::TinyGLTF gltfContext;
    std::string error;
    std::string warning;
    bool binary = false;
    binary = gltfContext.LoadBinaryFromMemory(&gltfModel, &error, &warning, static_cast<unsigned char*>(ptr), size);
    const tinygltf::Scene& scene = gltfModel.scenes[gltfModel.defaultScene > -1 ? gltfModel.defaultScene : 0];//デフォルトシーンがあればそれを、なければ最初のシーン

    minPos = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
    maxPos = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    GltfModel* model = loadGLTFModel(scene, gltfModel);//ファイルからgltfモデルのデータを読み取る
    model->initPoseMin = minPos;
    model->initPoseMax = maxPos;

    storage->addModel(obj, model);//ストレージにモデルを加える

    return storage->getgltfModel(obj);
}

//テクスチャを読み込む
void FileManager::loadTextures(GltfModel* model, const tinygltf::Model gltfModel)//画像データを読み取り、テクスチャデータを作成する
{
    for (tinygltf::Texture tex : gltfModel.textures)
    {
        tinygltf::Image source = gltfModel.images[tex.source];

        std::shared_ptr<ImageData> image =
            std::shared_ptr<ImageData>(new ImageData(source.width, source.height, source.component, source.image.data()));

        model->imageDatas.push_back(image);
    }

    model->textureDatas.resize(gltfModel.textures.size());
    for (size_t i = 0; i < model->textureDatas.size(); i++)
    {
        model->textureDatas[i] = new TextureData();
    }
}

//埋め込まれたgltfモデルを取得する
GltfModel* FileManager::loadGLTFModel(const tinygltf::Scene& scene,const tinygltf::Model& gltfModel)//gltfモデルのデータを読み取る
{
    float scale = 1.0f;
    GltfNode* root = new GltfNode();
    GltfModel* model = new GltfModel(root);

    minPos = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
    maxPos = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    vertexNum = 0;
    indexNum = 0;

    loadTextures(model, gltfModel);//テクスチャの読み取り
    loadMaterial(model, gltfModel);//マテリアルデータの読み取り

    for (size_t i = 0; i < scene.nodes.size(); i++)//gltfのノードの読み取り
    {
        const tinygltf::Node gltfNode = gltfModel.nodes[scene.nodes[i]];
        loadNode(nullptr, model->getRootNode(),model, gltfNode, scene.nodes[i], gltfModel, scale);
    }

    model->vertexNum = vertexNum;
    model->indexNum = indexNum;

    loadSkin(model, gltfModel);//スキンメッシュアニメーション用のスキンを読み取り
    for (size_t i = 0; i < model->skins.size(); i++)
    {
        setSkin(model->getRootNode(), model);//スキンの設定
    }

    if (gltfModel.animations.size() > 0)
    {
        loadAnimations(model, scene, gltfModel);//アニメーションの取得
    }

    return model;
}

//gltfモデルのノードを再帰的に読み込む
void FileManager::loadNode(GltfNode* parent, GltfNode* current, GltfModel* model, const tinygltf::Node& gltfNode, uint32_t nodeIndex, const tinygltf::Model& gltfModel, float globalscale)
{
    current->index = nodeIndex;
    current->parent = parent;
    current->name = gltfNode.name;
    current->skinIndex = gltfNode.skin;
    current->matrix = glm::mat4(1.0f);

    glm::vec3 translation = glm::vec3(0.0f);//平行移動行列
    if (gltfNode.translation.size() == 3) {
        translation = glm::make_vec3(gltfNode.translation.data());
        current->translation = translation;
    }
    glm::mat4 rotation = glm::mat4(1.0f);//回転行列
    if (gltfNode.rotation.size() == 4) {
        glm::quat q = glm::make_quat(gltfNode.rotation.data());
        current->rotation = glm::mat4(q);
    }
    glm::vec3 scale = glm::vec3(1.0f);//拡大行列
    if (gltfNode.scale.size() == 3) {
        scale = glm::make_vec3(gltfNode.scale.data());
        current->scale = scale;
    }
    if (gltfNode.matrix.size() == 16) {
        current->matrix = glm::make_mat4x4(gltfNode.matrix.data());
    };

    if (gltfNode.children.size() > 0)
    {
        for (size_t i = 0; i < gltfNode.children.size(); i++)
        {
            GltfNode* newNode = new GltfNode();
            loadNode(current, newNode,model, gltfModel.nodes[gltfNode.children[i]], gltfNode.children[i], gltfModel, globalscale);
        }
    }

    if (gltfNode.mesh > -1)
    {
        processMesh(gltfNode, gltfModel, current,model);//メッシュの読み取り
    }

    if (parent)
    {
        parent->children.push_back(current);
    }
}

//メッシュの読み取り
void FileManager::processMesh(const tinygltf::Node& gltfNode, const tinygltf::Model gltfModel, GltfNode* currentNode, GltfModel* model)
{
    const tinygltf::Mesh gltfMesh = gltfModel.meshes[gltfNode.mesh];
    int indexStart = 0;
    int vertexNum = 0;;

    Mesh* mesh = new Mesh();
    mesh->meshIndex = model->meshCount;
    model->meshCount++;

    for (size_t i = 0; i < gltfMesh.primitives.size(); i++)
    {
        const tinygltf::Primitive glPrimitive = gltfMesh.primitives[i];

        processPrimitive(mesh, indexStart, glPrimitive, gltfModel,model);//プリミティブの読み取り

        model->primitiveCount++;
    }

    currentNode->mesh = mesh;
}

//aabb用の頂点座標の最小と最大を計算
void FileManager::calcMinMaxVertexPos(glm::vec3 min,glm::vec3 max)
{
    for (int i = 0; i < 3; i++)
    {
        if (minPos[i] > min[i])
        {
            minPos[i] = min[i];
        }
    }

    for (int i = 0; i < 3; i++)
    {
        if (maxPos[i] < max[i])
        {
            maxPos[i] = max[i];
        }
    }
}

//プリミティブの読み取り
void FileManager::processPrimitive(Mesh* mesh,int& indexStart, tinygltf::Primitive glPrimitive, tinygltf::Model glModel,GltfModel* model)
{
    const float* bufferPos = nullptr;
    int vertexCount;

    const float* bufferNormals = nullptr;
    const float* bufferTexCoordSet0 = nullptr;
    const float* bufferTexCoordSet1 = nullptr;
    const float* bufferColorSet0 = nullptr;
    const void* bufferJoints = nullptr;
    const float* bufferWeights = nullptr;
    glm::vec3 posMin{};
    glm::vec3 posMax{};

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
    posMin = glm::vec3(posAccessor.minValues[0], posAccessor.minValues[1], posAccessor.minValues[2]);
    posMax = glm::vec3(posAccessor.maxValues[0], posAccessor.maxValues[1], posAccessor.maxValues[2]);
    vertexCount = static_cast<uint32_t>(posAccessor.count);//このプリミティブの持つ頂点の数
    posByteStride = posAccessor.ByteStride(posView) ? (posAccessor.ByteStride(posView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);//頂点を読み取る際の頂点のデータの幅を取得

    calcMinMaxVertexPos(posMin, posMax);

    //以下頂点座標と同じように、取得していく

    if (glPrimitive.attributes.find("NORMAL") != glPrimitive.attributes.end()) {
        const tinygltf::Accessor& normAccessor = glModel.accessors[glPrimitive.attributes.find("NORMAL")->second];
        const tinygltf::BufferView& normView = glModel.bufferViews[normAccessor.bufferView];
        bufferNormals = reinterpret_cast<const float*>(&(glModel.buffers[normView.buffer].data[normAccessor.byteOffset + normView.byteOffset]));
        normByteStride = normAccessor.ByteStride(normView) ? (normAccessor.ByteStride(normView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);
    }

    //uvの読み取り
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

    //頂点カラーの読み取り
    if (glPrimitive.attributes.find("COLOR_0") != glPrimitive.attributes.end()) {
        const tinygltf::Accessor& accessor = glModel.accessors[glPrimitive.attributes.find("COLOR_0")->second];
        const tinygltf::BufferView& view = glModel.bufferViews[accessor.bufferView];
        bufferColorSet0 = reinterpret_cast<const float*>(&(glModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
        color0ByteStride = accessor.ByteStride(view) ? (accessor.ByteStride(view) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);
    }

    //スケルトンの番号の読み取り
    if (glPrimitive.attributes.find("JOINTS_0") != glPrimitive.attributes.end()) {
        const tinygltf::Accessor& jointAccessor = glModel.accessors[glPrimitive.attributes.find("JOINTS_0")->second];
        const tinygltf::BufferView& jointView = glModel.bufferViews[jointAccessor.bufferView];
        bufferJoints = &(glModel.buffers[jointView.buffer].data[jointAccessor.byteOffset + jointView.byteOffset]);
        jointComponentType = jointAccessor.componentType;
        jointByteStride = jointAccessor.ByteStride(jointView) ? (jointAccessor.ByteStride(jointView) / tinygltf::GetComponentSizeInBytes(jointComponentType)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC4);
    }

    //スケルトンのウェイトの読み取り
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

    //頂点に各要素を設定していく
    for (size_t v = 0; v < posAccessor.count; v++) {
        Vertex vert;
        vert.pos = glm::vec3(glm::make_vec3(&bufferPos[v * posByteStride]));
        vert.normal = glm::normalize(glm::vec3(bufferNormals ? glm::make_vec3(&bufferNormals[v * normByteStride]) : glm::vec3(0.0f)));
        vert.texCoord0 = bufferTexCoordSet0 ? glm::make_vec2(&bufferTexCoordSet0[v * uv0ByteStride]) : glm::vec2(0.0f);
        vert.texCoord1 = bufferTexCoordSet1 ? glm::make_vec2(&bufferTexCoordSet1[v * uv1ByteStride]) : glm::vec2(0.0f);
        vert.color = bufferColorSet0 ? glm::make_vec4(&bufferColorSet0[v * color0ByteStride]) : glm::vec4(1.0f);

        if (hasSkin)
        {
            switch (jointComponentType) {
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
                const uint16_t* buf = static_cast<const uint16_t*>(bufferJoints);
                vert.boneID1 = glm::ivec4(glm::make_vec4(&buf[v * jointByteStride]));
                break;
            }
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
                const uint8_t* buf = static_cast<const uint8_t*>(bufferJoints);
                vert.boneID1 = glm::ivec4(glm::make_vec4(&buf[v * jointByteStride]));
                break;
            }
            }
        }
        else {
            vert.boneID1 = glm::ivec4(0);
        }
        vert.weight1 = hasSkin ? glm::make_vec4(&bufferWeights[v * weightByteStride]) : glm::vec4(0.0f);
        if (glm::length(vert.weight1) == 0.0f) {
            vert.weight1 = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
        }

        mesh->vertices.push_back(vert);
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
                mesh->indices.push_back(buf[index] + indexStart);
            }
            break;
        }
        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
            const uint16_t* buf = static_cast<const uint16_t*>(dataPtr);
            for (size_t index = 0; index < accessor.count; index++) {
                mesh->indices.push_back(buf[index] + indexStart);
            }
            break;
        }
        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
            const uint8_t* buf = static_cast<const uint8_t*>(dataPtr);
            for (size_t index = 0; index < accessor.count; index++) {
                mesh->indices.push_back(buf[index] + indexStart);
            }
            break;
        }
        }
    }



    Primitive primitive = { model->primitiveCount,indexStart,indexCount,vertexCount,glPrimitive.material };
    primitive.setBoundingBox(posMin, posMax);

    mesh->primitives.push_back(primitive);

    indexStart += indexCount;
    vertexNum += vertexCount;
    indexNum += indexStart;
}

//gltfモデルのアニメーションを読み込む
void FileManager::loadAnimations(GltfModel* model, const tinygltf::Scene& scene, const tinygltf::Model& gltfModel)
{
    for (tinygltf::Animation anim : gltfModel.animations) 
    {
        Animation animation{};
        animation.name = anim.name;
        if (anim.name.empty()) {
            animation.name = std::to_string(model->animations.size());
        }

        //アニメーションのサンプリングの設定
        for (auto& samp : anim.samplers) {
            AnimationSampler sampler{};

            if (samp.interpolation == "LINEAR") {
                sampler.interpolation = AnimationSampler::InterpolationType::LINEAR;
            }
            if (samp.interpolation == "STEP") {
                sampler.interpolation = AnimationSampler::InterpolationType::STEP;
            }
            if (samp.interpolation == "CUBICSPLINE") {
                sampler.interpolation = AnimationSampler::InterpolationType::CUBICSPLINE;
            }

            //アニメーションのキーの時間の取得
            {
                const tinygltf::Accessor& accessor = gltfModel.accessors[samp.input];
                const tinygltf::BufferView& bufferView = gltfModel.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = gltfModel.buffers[bufferView.buffer];

                assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

                const void* dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];
                const float* buf = static_cast<const float*>(dataPtr);
                for (size_t index = 0; index < accessor.count; index++) {
                    sampler.inputs.push_back(buf[index]);
                }

                for (auto input : sampler.inputs) {
                    if (input < animation.start) {
                        animation.start = input;
                    };
                    if (input > animation.end) {
                        animation.end = input;
                    }
                }
            }

            //アニメーションのキーの移動/回転/拡大の設定
            {
                const tinygltf::Accessor& accessor = gltfModel.accessors[samp.output];
                const tinygltf::BufferView& bufferView = gltfModel.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = gltfModel.buffers[bufferView.buffer];

                assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

                const void* dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];

                switch (accessor.type) {
                case TINYGLTF_TYPE_VEC3: {
                    const glm::vec3* buf = static_cast<const glm::vec3*>(dataPtr);
                    for (size_t index = 0; index < accessor.count; index++) {
                        sampler.outputsVec4.push_back(glm::vec4(buf[index], 0.0f));
                        sampler.outputs.push_back(buf[index][0]);
                        sampler.outputs.push_back(buf[index][1]);
                        sampler.outputs.push_back(buf[index][2]);
                    }
                    break;
                }
                case TINYGLTF_TYPE_VEC4: {
                    const glm::vec4* buf = static_cast<const glm::vec4*>(dataPtr);
                    for (size_t index = 0; index < accessor.count; index++) {
                        sampler.outputsVec4.push_back(buf[index]);
                        sampler.outputs.push_back(buf[index][0]);
                        sampler.outputs.push_back(buf[index][1]);
                        sampler.outputs.push_back(buf[index][2]);
                        sampler.outputs.push_back(buf[index][3]);
                    }
                    break;
                }
                default: {
                    std::cout << "unknown type" << std::endl;
                    break;
                }
                }
            }

            animation.samplers.push_back(sampler);
        }

        //移動/回転/拡大の設定
        for (auto& source : anim.channels) {
            AnimationChannel channel{};

            if (source.target_path == "rotation") {
                channel.path = AnimationChannel::PathType::ROTATION;
            }
            if (source.target_path == "translation") {
                channel.path = AnimationChannel::PathType::TRANSLATION;
            }
            if (source.target_path == "scale") {
                channel.path = AnimationChannel::PathType::SCALE;
            }
            if (source.target_path == "weights") {
                std::cout << "weights not yet supported, skipping channel" << std::endl;
                continue;
            }
            channel.samplerIndex = source.sampler;
            channel.node = model->nodeFromIndex(source.target_node);
            if (!channel.node) {
                continue;
            }

            animation.channels.push_back(channel);
        }

        model->animations[animation.name] = animation;
    }
}

//gltfモデルのスケルトンを読み込む
void FileManager::loadSkin(GltfModel* model, tinygltf::Model gltfModel)
{
    for (tinygltf::Skin& source : gltfModel.skins) {
        Skin* newSkin = new Skin{};
        newSkin->name = source.name;

        //ルートノードの設定
        if (source.skeleton > -1) {
            newSkin->skeletonRoot = model->nodeFromIndex(source.skeleton);
        }
        else
        {
            newSkin->skeletonRoot = nullptr;
        }


        //スケルトンが番号で指定したノードを見つけ、Modelクラスのスケルトンにノードを設定していく
        int globalHasSkinNodeIndex = 1;
        for (int jointIndex : source.joints) {
            GltfNode* node = model->nodeFromIndex(jointIndex);
            if (node) {
                node->globalHasSkinNodeIndex = globalHasSkinNodeIndex;
                newSkin->joints.push_back(node);
                globalHasSkinNodeIndex++;
            }
        }
        model->jointNum = globalHasSkinNodeIndex;

        //ボーン空間に戻す行列を設定していく
        if (source.inverseBindMatrices > -1) {
            const tinygltf::Accessor& accessor = gltfModel.accessors[source.inverseBindMatrices];
            const tinygltf::BufferView& bufferView = gltfModel.bufferViews[accessor.bufferView];
            const tinygltf::Buffer& buffer = gltfModel.buffers[bufferView.buffer];
            newSkin->inverseBindMatrices.resize(accessor.count);
            memcpy(newSkin->inverseBindMatrices.data(), &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(glm::mat4));
        }

        model->skins.push_back(newSkin);
    }
}

//自前のModelクラスにスケルトンを設定する
void FileManager::setSkin(GltfNode* node,GltfModel* model)
{
    if (node->skinIndex > -1)
    {
        node->skin = model->skins[node->skinIndex];
    }

    for (size_t i = 0; i < node->children.size(); i++)
    {
        setSkin(node->children[i], model);
    }
}

//与えられた文字列からファイルの名前のみを取得する
std::string FileManager::splitFileName(std::string filePath)
{
    int pos = static_cast<int>(filePath.rfind('/'));
    filePath = filePath.substr(pos + 1, filePath.length() - pos);

    return filePath;
}

//マテリアルをメッシュに設定する
void FileManager::loadMaterial(GltfModel* model,tinygltf::Model gltfModel)
{
    for (tinygltf::Material& mat : gltfModel.materials)
    {
        std::shared_ptr<Material> material = std::shared_ptr<Material>(new Material());

        if (mat.values.find("baseColorTexture") != mat.values.end()) {
            material->baseColorTextureIndex = mat.values["baseColorTexture"].TextureIndex();
            material->texCoordSets.baseColor = mat.values["baseColorTexture"].TextureTexCoord();
        }
        if (mat.values.find("metallicRoughnessTexture") != mat.values.end()) {
            material->metallicRoughnessTextureIndex = mat.values["metallicRoughnessTexture"].TextureIndex();
            material->texCoordSets.metallicRoughness = mat.values["metallicRoughnessTexture"].TextureTexCoord();
        }
        if (mat.values.find("roughnessFactor") != mat.values.end()) {
            material->roughnessFactor = static_cast<float>(mat.values["roughnessFactor"].Factor());
        }
        if (mat.values.find("metallicFactor") != mat.values.end()) {
            material->metallicFactor = static_cast<float>(mat.values["metallicFactor"].Factor());
        }
        if (mat.values.find("baseColorFactor") != mat.values.end()) {
            material->baseColorFactor = glm::make_vec4(mat.values["baseColorFactor"].ColorFactor().data());
        }
        if (mat.additionalValues.find("normalTexture") != mat.additionalValues.end()) {
            material->normalTextureIndex = mat.additionalValues["normalTexture"].TextureIndex();
            material->texCoordSets.normal = mat.additionalValues["normalTexture"].TextureTexCoord();
        }
        if (mat.additionalValues.find("emissiveTexture") != mat.additionalValues.end()) {
            material->emissiveTextureIndex = mat.additionalValues["emissiveTexture"].TextureIndex();
            material->texCoordSets.emissive = mat.additionalValues["emissiveTexture"].TextureTexCoord();
        }
        if (mat.additionalValues.find("occlusionTexture") != mat.additionalValues.end()) {
            material->occlusionTextureIndex = mat.additionalValues["occlusionTexture"].TextureIndex();
            material->texCoordSets.occlusion = mat.additionalValues["occlusionTexture"].TextureTexCoord();
        }
        if (mat.additionalValues.find("alphaMode") != mat.additionalValues.end()) {
            tinygltf::Parameter param = mat.additionalValues["alphaMode"];
            if (param.string_value == "BLEND") {
                material->alphaMode = Material::ALPHAMODE_BLEND;
            }
            if (param.string_value == "MASK") {
                material->alphaCutoff = 0.5f;
                material->alphaMode = Material::ALPHAMODE_MASK;
            }
        }
        if (mat.additionalValues.find("alphaCutoff") != mat.additionalValues.end()) {
            material->alphaCutoff = static_cast<float>(mat.additionalValues["alphaCutoff"].Factor());
        }
        if (mat.additionalValues.find("emissiveFactor") != mat.additionalValues.end()) {
            material->emissiveFactor = glm::vec4(glm::make_vec3(mat.additionalValues["emissiveFactor"].ColorFactor().data()), 1.0);
        }

        material->index = static_cast<int>(model->materials.size());
        model->materials.push_back(material);
    }
}

//画像の読み取り
std::shared_ptr<ImageData> FileManager::loadImage(std::string filePath)
{
    std::string registerImageName = splitFileName(filePath);

    Storage* storage = Storage::GetInstance();
    if (storage->containImageData(registerImageName))
    {
        return storage->getImageData(registerImageName);
    }

    int width;
    int height;
    int texChannels;
    unsigned char* pixels;

    pixels = stbi_load(filePath.c_str(), &width, &height, &texChannels,0);

    ImageData* imageData = new ImageData(width, height, texChannels, pixels);

    storage->addImageData(registerImageName,imageData);

    return storage->getImageData(registerImageName);
}