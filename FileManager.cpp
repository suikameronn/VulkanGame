#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT


#include"FileManager.h"

FileManager* FileManager::fileManager = nullptr;

FileManager::FileManager()
{
}

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
    }

    return -1;
}

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

std::shared_ptr<GltfModel> FileManager::loadModel(GLTFOBJECT obj)//3D���f����Ԃ�
{
    Storage* storage = Storage::GetInstance();
    if (storage->containModel(obj))//���ł�3D���f�����t�@�C������ǂݎ��ς݂Ȃ�
    {
        return storage->getgltfModel(obj);//�X�g���[�W�N���X����n��
    }

    void* ptr = nullptr;
    int size = 0;
    loadgltfModel(getModelResource(obj),&ptr, size);//�t�@�C�����擾����

    tinygltf::Model gltfModel;
    tinygltf::TinyGLTF gltfContext;
    std::string error;
    std::string warning;
    bool binary = false;
    binary = gltfContext.LoadBinaryFromMemory(&gltfModel, &error, &warning, static_cast<unsigned char*>(ptr), size);
    const tinygltf::Scene& scene = gltfModel.scenes[gltfModel.defaultScene > -1 ? gltfModel.defaultScene : 0];//�f�t�H���g�V�[��������΂�����A�Ȃ���΍ŏ��̃V�[��

    allVertNum = 0;
    pivot = glm::vec3(0.0);
    minPos = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
    maxPos = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    GltfModel* model = loadGLTFModel(scene, gltfModel);//�t�@�C������gltf���f���̃f�[�^��ǂݎ��
    model->initPoseMin = minPos;
    model->initPoseMax = maxPos;

    storage->addModel(obj, model);//�X�g���[�W�Ƀ��f����������

    return storage->getgltfModel(obj);
}

void FileManager::loadTextures(GltfModel* model, const tinygltf::Model gltfModel)//�摜�f�[�^��ǂݎ��A�e�N�X�`���f�[�^���쐬����
{
    for (tinygltf::Texture tex : gltfModel.textures)
    {
        tinygltf::Image source = gltfModel.images[tex.source];

        std::shared_ptr<ImageData> image =
            std::shared_ptr<ImageData>(new ImageData(source.width, source.height, source.component, source.image.data()));

        model->imageDatas.push_back(image);
    }

    model->textureDatas.resize(gltfModel.textures.size());
    for (int i = 0; i < model->textureDatas.size(); i++)
    {
        model->textureDatas[i] = new TextureData();
    }
}

GltfModel* FileManager::loadGLTFModel(const tinygltf::Scene& scene,const tinygltf::Model& gltfModel)//gltf���f���̃f�[�^��ǂݎ��
{
    float scale = 1.0f;
    GltfNode* root = new GltfNode();
    GltfModel* model = new GltfModel(root);

    minPos = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
    maxPos = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    loadTextures(model, gltfModel);//�e�N�X�`���̓ǂݎ��
    loadMaterial(model, gltfModel);//�}�e���A���f�[�^�̓ǂݎ��

    for (size_t i = 0; i < scene.nodes.size(); i++)//gltf�̃m�[�h�̓ǂݎ��
    {
        const tinygltf::Node gltfNode = gltfModel.nodes[scene.nodes[i]];
        loadNode(nullptr, model->getRootNode(),model, gltfNode, scene.nodes[i], gltfModel, scale);
    }

    loadSkin(model, gltfModel);//�X�L�����b�V���A�j���[�V�����p�̃X�L����ǂݎ��
    for (int i = 0; i < model->skins.size(); i++)
    {
        setSkin(model->getRootNode(), model);//�X�L���̐ݒ�
    }

    if (gltfModel.animations.size() > 0)
    {
        loadAnimations(model, scene, gltfModel);//�A�j���[�V�����̎擾
    }

    return model;
}


void FileManager::loadNode(GltfNode* parent, GltfNode* current, GltfModel* model, const tinygltf::Node& gltfNode, uint32_t nodeIndex, const tinygltf::Model& gltfModel, float globalscale)
{
    current->index = nodeIndex;
    current->parent = parent;
    current->name = gltfNode.name;
    current->skinIndex = gltfNode.skin;
    current->matrix = glm::mat4(1.0f);

    glm::vec3 translation = glm::vec3(0.0f);//���s�ړ��s��
    if (gltfNode.translation.size() == 3) {
        translation = glm::make_vec3(gltfNode.translation.data());
        current->translation = translation;
    }
    glm::mat4 rotation = glm::mat4(1.0f);//��]�s��
    if (gltfNode.rotation.size() == 4) {
        glm::quat q = glm::make_quat(gltfNode.rotation.data());
        current->rotation = glm::mat4(q);
    }
    glm::vec3 scale = glm::vec3(1.0f);//�g��s��
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
        processMesh(gltfNode, gltfModel, current,model);//���b�V���̓ǂݎ��
    }

    if (parent)
    {
        parent->children.push_back(current);
    }
}

//���b�V���̓ǂݎ��
void FileManager::processMesh(const tinygltf::Node& gltfNode, const tinygltf::Model gltfModel, GltfNode* currentNode, GltfModel* model)
{
    const tinygltf::Mesh gltfMesh = gltfModel.meshes[gltfNode.mesh];
    int indexStart = 0;

    Mesh* mesh = new Mesh();
    mesh->meshIndex = model->meshCount;
    model->meshCount++;

    for (unsigned int i = 0; i < gltfMesh.primitives.size(); i++)
    {
        const tinygltf::Primitive glPrimitive = gltfMesh.primitives[i];

        processPrimitive(mesh, indexStart, glPrimitive, gltfModel,model);//�v���~�e�B�u�̓ǂݎ��

        model->primitiveCount++;
    }

    currentNode->mesh = mesh;
}

//aabb�p�̒��_���W�̍ŏ��ƍő���v�Z
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

//�v���~�e�B�u�̓ǂݎ��
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

    const tinygltf::Accessor& posAccessor = glModel.accessors[glPrimitive.attributes.find("POSITION")->second];//�v���~�e�B�u����ʒu��\���o�b�t�@�ɃA�N�Z�X���邽�߂̕ϐ�
    const tinygltf::BufferView& posView = glModel.bufferViews[posAccessor.bufferView];//�A�N�Z�b�T�[����ăo�b�t�@�[����l��ǂݎ��
    bufferPos = reinterpret_cast<const float*>(&(glModel.buffers[posView.buffer].data[posAccessor.byteOffset + posView.byteOffset]));//���ꂼ��̃I�t�Z�b�g�𑫂��ĎQ�Ƃ���ʒu�𒲐�����
    posMin = glm::vec3(posAccessor.minValues[0], posAccessor.minValues[1], posAccessor.minValues[2]);
    posMax = glm::vec3(posAccessor.maxValues[0], posAccessor.maxValues[1], posAccessor.maxValues[2]);
    vertexCount = static_cast<uint32_t>(posAccessor.count);//���̃v���~�e�B�u�̎����_�̐�
    posByteStride = posAccessor.ByteStride(posView) ? (posAccessor.ByteStride(posView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);//���_��ǂݎ��ۂ̒��_�̃f�[�^�̕����擾

    calcMinMaxVertexPos(posMin, posMax);

    //�ȉ����_���W�Ɠ����悤�ɁA�擾���Ă���

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
            default:
                // Not supported by spec
                std::cerr << "Joint component type " << jointComponentType << " not supported!" << std::endl;
                break;
            }
        }
        else {
            vert.boneID1 = glm::ivec4(0);
        }
        vert.weight1 = hasSkin ? glm::make_vec4(&bufferWeights[v * weightByteStride]) : glm::vec4(0.0f);
        // Fix for all zero weights
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
        default:
            std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
            return;
        }
    }



    Primitive primitive = { model->primitiveCount,indexStart,indexCount,vertexCount,glPrimitive.material };
    primitive.setBoundingBox(posMin, posMax);

    mesh->primitives.push_back(primitive);

    indexStart += indexCount;
}

void FileManager::loadAnimations(GltfModel* model, const tinygltf::Scene& scene, const tinygltf::Model& gltfModel)
{
    for (tinygltf::Animation anim : gltfModel.animations) 
    {
        Animation animation{};
        animation.name = anim.name;
        if (anim.name.empty()) {
            animation.name = std::to_string(model->animations.size());
        }

        // Samplers
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

            // Read sampler input time values
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

            // Read sampler output T/R/S values 
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

        // Channels
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

int FileManager::splitPos(std::string text, std::array<float, 4>& coliderSetting)
{
    int spacePos = 0;
    int beforeSpacePos = 0;
    std::string substr;

    for(int i = 0;i < 3;i++)
    {
        spacePos = text.find(" ",beforeSpacePos);
        if (spacePos == std::string::npos)
        {
            return -1;
        }

        substr = text.substr(beforeSpacePos, spacePos - beforeSpacePos - 1);
        coliderSetting[i] = std::stof(substr);

        beforeSpacePos = spacePos + 1;
    }
    substr = text.substr(beforeSpacePos, text.length() - beforeSpacePos);
    coliderSetting[3] = std::stof(substr);

    return 0;
}

void FileManager::loadSkin(GltfModel* model, tinygltf::Model gltfModel)
{
    for (tinygltf::Skin& source : gltfModel.skins) {
        Skin* newSkin = new Skin{};
        newSkin->name = source.name;

        // Find skeleton root node
        if (source.skeleton > -1) {
            newSkin->skeletonRoot = model->nodeFromIndex(source.skeleton);
        }
        else
        {
            newSkin->skeletonRoot = nullptr;
        }


        // Find joint nodes
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

        // Get inverse bind matrices from buffer
        if (source.inverseBindMatrices > -1) {
            const tinygltf::Accessor& accessor = gltfModel.accessors[source.inverseBindMatrices];
            const tinygltf::BufferView& bufferView = gltfModel.bufferViews[accessor.bufferView];
            const tinygltf::Buffer& buffer = gltfModel.buffers[bufferView.buffer];
            newSkin->inverseBindMatrices.resize(accessor.count);
            memcpy(newSkin->inverseBindMatrices.data(), &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(glm::mat4));
        }

        if (newSkin->joints.size() > 128) {
            std::cerr << "[WARNING] Skin " << newSkin->name << " has " << newSkin->joints.size() << " joints, which is higher than the supported maximum of " << 128 << "\n";
            std::cerr << "[WARNING] glTF scene may display wrong/incomplete\n";
        }

        model->skins.push_back(newSkin);
    }
}

void FileManager::setSkin(GltfNode* node,GltfModel* model)
{
    if (node->skinIndex > -1)
    {
        node->skin = model->skins[node->skinIndex];
    }

    for (int i = 0; i < node->children.size(); i++)
    {
        setSkin(node->children[i], model);
    }
}

std::string FileManager::splitFileName(std::string filePath)
{
    int pos = filePath.rfind('/');
    filePath = filePath.substr(pos + 1, filePath.length() - pos);

    return filePath;
}

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

        material->index = model->materials.size();
        model->materials.push_back(material);
    }
}

//�摜�̓ǂݎ��
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