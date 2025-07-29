#include"GltfModelFactory.h"

uint32_t GltfModelFactory::Load(const std::string& filePath)
{
	uint32_t hash = hashFilePath(filePath);

	if(modelStorage.find(hash) != modelStorage.end())
	{
		return hash;
	}

    tinygltf::Model gltfModel;
    tinygltf::TinyGLTF gltfContext;
    std::string error;
    std::string warning;
    bool binary = false;
    binary = gltfContext.LoadBinaryFromFile(&gltfModel, &error, &warning, filePath);
    if (!binary)
    {
        throw std::runtime_error("faile load file");
    }

    const tinygltf::Scene& scene = gltfModel.scenes[gltfModel.defaultScene > -1 ? gltfModel.defaultScene : 0];

	minPos = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	maxPos = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    vertexNum = 0;
    indexNum = 0;

    std::shared_ptr<GltfModel> model =
        std::make_shared<GltfModel>(bufferFactory, layoutFactory, descriptorSetFactory);

    //GltfNode���m�ۂ���
	model->nodes.resize(gltfModel.nodes.size());

    //gltf���f���̃V�[���͈�݂̂�O��Ƃ���
    loadNode(model, scene.nodes[0], gltfModel);

	loadMaterial(model, gltfModel);//�}�e���A���f�[�^�̓ǂݎ��

    if (gltfModel.animations.size() > 0)
    {
		loadAnimations(model, gltfModel);//�A�j���[�V�����̎擾
    }

	loadSkin(model, gltfModel);//�X�L�����b�V���A�j���[�V�����p�̃X�L����ǂݎ��
	setSkin(model);//�X�L���̐ݒ�

    //gpu��̒��_�o�b�t�@�Ȃǂ��쐬
    model->setPointBufferNum();

    modelStorage[hash] = model;//�X�g���[�W�Ƀ��f����������

    return hash;
}

//gltf���f���̃m�[�h���ċA�I�ɓǂݍ���
void GltfModelFactory::loadNode(std::shared_ptr<GltfModel> model, const int nodeIndex, const tinygltf::Model& gltfModel)
{
    /*
    current->index = nodeIndex;
    current->name = gltfNode.name;
    current->skinIndex = gltfNode.skin;
    current->matrix = glm::mat4(1.0f);

    if (gltfNode.matrix.size() == 16) {
        current->matrix = glm::make_mat4x4(gltfNode.matrix.data());
    };

    if (gltfNode.mesh > -1)
    {
        for (int i = 0; i <= gltfNode.mesh; i++)
        {
            //���b�V���̓ǂݎ��
            loadMesh(gltfNode, gltfModel, current, model, i);
        }
    }

    if (gltfNode.children.size() > 0)
    {
        current->children.resize(gltfNode.children.size());

        for (size_t i = 0; i < gltfNode.children.size(); i++)
        {
            GltfNode* newNode = new GltfNode();
            loadNode(newNode, model, gltfModel.nodes[gltfNode.children[i]], gltfNode.children[i], gltfModel);

            current->children[i] = newNode;
        }
    }
    */

    const tinygltf::Node& gltfNode = gltfModel.nodes[nodeIndex];

    model->nodes[0].offset = 0;

    //parentIndex�͐e�m�[�h�̔z���̔ԍ�
    model->nodes[0].parentIndex = -1;

    //index��gltf��̃m�[�h�̔ԍ�
    model->nodes[0].index = nodeIndex;
    
    model->nodes[0].name = gltfNode.name;
    model->nodes[0].skinIndex = gltfNode.skin;
    model->nodes[0].matrix = glm::mat4(1.0f);

    if (gltfNode.matrix.size() == 16) {
        model->nodes[0].matrix = glm::make_mat4x4(gltfNode.matrix.data());
    }

    if (gltfNode.mesh > -1)
    {
        model->nodes[0].meshArray.resize(1);

        //���b�V���̓ǂݎ��
        loadMesh(gltfNode, gltfModel, model->nodes[0].meshArray[0], model, gltfNode.mesh);
    }

    if (gltfNode.children.size() > 0)
    {
        size_t offset = 1;
        int parentIndex = 0;

        for (size_t i = 0; i < gltfNode.children.size(); i++)
        {
            loadNode(offset, parentIndex, model, gltfNode.children[i], gltfModel);
        }
    }
}

//gltf���f���̃m�[�h���ċA�I�ɓǂݍ���
void GltfModelFactory::loadNode(size_t& offset, int parentIndex, std::shared_ptr<GltfModel> model, const int nodeIndex
    , const tinygltf::Model& gltfModel)
{
	const tinygltf::Node& gltfNode = gltfModel.nodes[nodeIndex];

    model->nodes[offset].offset = offset;
	model->nodes[offset].parentIndex = parentIndex;
    model->nodes[offset].index = nodeIndex;
    model->nodes[offset].name = gltfNode.name;
    model->nodes[offset].skinIndex = gltfNode.skin;
    model->nodes[offset].matrix = glm::mat4(1.0f);

    if (gltfNode.matrix.size() == 16) {
        model->nodes[offset].matrix = glm::make_mat4x4(gltfNode.matrix.data());
    }

    if (gltfNode.mesh > -1)
    {
        model->nodes[offset].meshArray.resize(1);

        //���b�V���̓ǂݎ��
        loadMesh(gltfNode, gltfModel, model->nodes[offset].meshArray[0], model, gltfNode.mesh);
    }

    //�m�[�h��z�u����ʒu������ɂ���
    offset++;

    if (gltfNode.children.size() > 0)
    {

		//�e�m�[�h�̃C���f�b�N�X��ݒ�
        parentIndex++;

        for (size_t i = 0; i < gltfNode.children.size(); i++)
        {
            loadNode(offset, parentIndex, model, gltfNode.children[i], gltfModel);
        }
    }
}

//gltf���f���̃��b�V����ǂݍ���
void GltfModelFactory::loadMesh(const tinygltf::Node& gltfNode, const tinygltf::Model& gltfModel
    , Mesh& mesh, std::shared_ptr<GltfModel> model, int meshIndex)
{
    const tinygltf::Mesh gltfMesh = gltfModel.meshes[meshIndex];
    int indexStart = 0;
    int vertexNum = 0;;

    mesh.meshIndex = model->meshCount;
    model->meshCount++;

    for (size_t i = 0; i < gltfMesh.primitives.size(); i++)
    {
        const tinygltf::Primitive glPrimitive = gltfMesh.primitives[i];

        loadPrimitive(mesh, indexStart, glPrimitive, gltfModel, model);//�v���~�e�B�u�̓ǂݎ��
    }

    model->primitiveCount += static_cast<uint32_t>(gltfMesh.primitives.size());
}

//�v���~�e�B�u�̓ǂݎ��
void GltfModelFactory::loadPrimitive(Mesh& mesh, int& indexStart
    , tinygltf::Primitive glPrimitive, tinygltf::Model glModel, std::shared_ptr<GltfModel> model)
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

	//AABB�̂��߂ɒ��_�̍ŏ��l�ƍő�l�����߂�
    for (int i = 0; i < 3; i++)
    {
		std::min(minPos[i], posMin[i]);
		std::max(maxPos[i], posMax[i]);
    }

    //�ȉ����_���W�Ɠ����悤�ɁA�擾���Ă���

    if (glPrimitive.attributes.find("NORMAL") != glPrimitive.attributes.end()) {
        const tinygltf::Accessor& normAccessor = glModel.accessors[glPrimitive.attributes.find("NORMAL")->second];
        const tinygltf::BufferView& normView = glModel.bufferViews[normAccessor.bufferView];
        bufferNormals = reinterpret_cast<const float*>(&(glModel.buffers[normView.buffer].data[normAccessor.byteOffset + normView.byteOffset]));
        normByteStride = normAccessor.ByteStride(normView) ? (normAccessor.ByteStride(normView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);
    }

    //uv�̓ǂݎ��
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

    //���_�J���[�̓ǂݎ��
    if (glPrimitive.attributes.find("COLOR_0") != glPrimitive.attributes.end()) {
        const tinygltf::Accessor& accessor = glModel.accessors[glPrimitive.attributes.find("COLOR_0")->second];
        const tinygltf::BufferView& view = glModel.bufferViews[accessor.bufferView];
        bufferColorSet0 = reinterpret_cast<const float*>(&(glModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
        color0ByteStride = accessor.ByteStride(view) ? (accessor.ByteStride(view) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);
    }

    //�X�P���g���̔ԍ��̓ǂݎ��
    if (glPrimitive.attributes.find("JOINTS_0") != glPrimitive.attributes.end()) {
        const tinygltf::Accessor& jointAccessor = glModel.accessors[glPrimitive.attributes.find("JOINTS_0")->second];
        const tinygltf::BufferView& jointView = glModel.bufferViews[jointAccessor.bufferView];
        bufferJoints = &(glModel.buffers[jointView.buffer].data[jointAccessor.byteOffset + jointView.byteOffset]);
        jointComponentType = jointAccessor.componentType;
        jointByteStride = jointAccessor.ByteStride(jointView) ? (jointAccessor.ByteStride(jointView) / tinygltf::GetComponentSizeInBytes(jointComponentType)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC4);
    }

    //�X�P���g���̃E�F�C�g�̓ǂݎ��
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

    //���_�Ɋe�v�f��ݒ肵�Ă���
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

        mesh.vertices.push_back(vert);
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
                mesh.indices.push_back(buf[index] + indexStart);
            }
            break;
        }
        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
            const uint16_t* buf = static_cast<const uint16_t*>(dataPtr);
            for (size_t index = 0; index < accessor.count; index++) {
                mesh.indices.push_back(buf[index] + indexStart);
            }
            break;
        }
        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
            const uint8_t* buf = static_cast<const uint8_t*>(dataPtr);
            for (size_t index = 0; index < accessor.count; index++) {
                mesh.indices.push_back(buf[index] + indexStart);
            }
            break;
        }
        }
    }

    Primitive primitive = { model->primitiveCount,indexStart,indexCount,vertexCount,glPrimitive.material };
    primitive.setBoundingBox(posMin, posMax);

    mesh.primitives.push_back(primitive);

    indexStart += indexCount;
    vertexNum += vertexCount;
    indexNum += indexStart;
}

//�A�j���[�V������ǂݍ���
void GltfModelFactory::loadAnimations(std::shared_ptr<GltfModel> model, const tinygltf::Model& gltfModel)
{
    for (tinygltf::Animation anim : gltfModel.animations)
    {
        Animation animation{};
        animation.name = anim.name;
        if (anim.name.empty()) {
            animation.name = std::to_string(model->animations.size());
        }

        //�A�j���[�V�����̃T���v�����O�̐ݒ�
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

            //�A�j���[�V�����̃L�[�̎��Ԃ̎擾
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

            //�A�j���[�V�����̃L�[�̈ړ�/��]/�g��̐ݒ�
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

        //�ړ�/��]/�g��̐ݒ�
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
            channel.nodeOffset = model->nodeFromIndex(source.target_node);
            if (channel.nodeOffset > -1) {
                continue;
            }

            animation.channels.push_back(channel);
        }

        model->animations[animation.name] = animation;
    }
}

//�X�P���g����ǂݍ���
void GltfModelFactory::loadSkin(std::shared_ptr<GltfModel> model, tinygltf::Model gltfModel)
{
    for (tinygltf::Skin& source : gltfModel.skins) {
        Skin newSkin = Skin();
        newSkin.name = source.name;

        //���[�g�m�[�h�̐ݒ�
        if (source.skeleton > -1) {
            newSkin.skinRootNodeOffset = model->nodeFromIndex(source.skeleton);
        }
        else
        {
            newSkin.skinRootNodeOffset = 0;
        }


        //�X�P���g�����ԍ��Ŏw�肵���m�[�h�������AModel�N���X�̃X�P���g���Ƀm�[�h��ݒ肵�Ă���
        int globalHasSkinNodeIndex = 1;
        newSkin.jointNodeOffset.resize(source.joints.size());
        for (int i = 0;i < source.joints.size();i++)
        {

            int nodeOffset = model->nodeFromIndex(source.joints[i]);
            
            if (nodeOffset > -1) 
            {
                model->nodes[nodeOffset].globalHasSkinNodeIndex = globalHasSkinNodeIndex;
                newSkin.jointNodeOffset[i] = nodeOffset;
                globalHasSkinNodeIndex++;
            }
        }
        model->jointNum = globalHasSkinNodeIndex;

        //�{�[����Ԃɖ߂��s���ݒ肵�Ă���
        if (source.inverseBindMatrices > -1) 
        {
            const tinygltf::Accessor& accessor = gltfModel.accessors[source.inverseBindMatrices];
            const tinygltf::BufferView& bufferView = gltfModel.bufferViews[accessor.bufferView];
            const tinygltf::Buffer& buffer = gltfModel.buffers[bufferView.buffer];
            newSkin.inverseBindMatrices.resize(accessor.count);
            memcpy(newSkin.inverseBindMatrices.data(), &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(glm::mat4));
        }

        model->skins.push_back(newSkin);
    }

	setSkin(model);//�X�L����ݒ肷��
}

//�X�L����ݒ肷��
void GltfModelFactory::setSkin(std::shared_ptr<GltfModel> model)
{
    for (int i = 0; i < model->nodes.size(); i++)
    {
        if (model->nodes[i].skinIndex > -1)
        {
            model->nodes[i].skin = model->skins[model->nodes[i].skinIndex];
        }
    }
}

//�}�e���A����ǂݍ���
void GltfModelFactory::loadMaterial(std::shared_ptr<GltfModel> model, tinygltf::Model& gltfModel)
{
    for (tinygltf::Material& mat : gltfModel.materials)
    {
        materialBuilder->initProperty();

        if (mat.values.find("baseColorTexture") != mat.values.end()) 
        {
            tinygltf::Parameter param = mat.values["baseColorTexture"];

			const tinygltf::Image image = 
                gltfModel.images[gltfModel.textures[param.TextureIndex()].source];
            
            std::shared_ptr<Texture> texture
                = textureFactory->Create(
                    image.component,
                    image.image.data(),
					image.width,
					image.height,
                    TexturePattern::NORMAL
				);

			materialBuilder->withBaseColorTexture(param.TextureTexCoord(), texture);

        }
        if (mat.values.find("metallicRoughnessTexture") != mat.values.end()) 
        {
            tinygltf::Parameter param = mat.values["metallicRoughnessTexture"];

            const tinygltf::Image image =
                gltfModel.images[gltfModel.textures[param.TextureIndex()].source];

            std::shared_ptr<Texture> texture
                = textureFactory->Create(
                    image.component,
                    image.image.data(),
                    image.width,
                    image.height,
                    TexturePattern::NORMAL
                );

            materialBuilder->withMetallicRoughnessTexture(param.TextureTexCoord(), texture);
        }
        if (mat.values.find("roughnessFactor") != mat.values.end()) 
        {
			materialBuilder->withRoughnessFactor(static_cast<float>(mat.values["roughnessFactor"].Factor()));
        }
        if (mat.values.find("metallicFactor") != mat.values.end()) 
        {
            materialBuilder->withRoughnessFactor(static_cast<float>(mat.values["metallicFactor"].Factor()));
        }
        if (mat.values.find("baseColorFactor") != mat.values.end()) 
        {
            materialBuilder->withBaseColorFactor(glm::make_vec4(mat.values["baseColorFactor"].ColorFactor().data()));
        }
        if (mat.additionalValues.find("normalTexture") != mat.additionalValues.end()) 
        {
            tinygltf::Parameter param = mat.additionalValues["normalTexture"];

            const tinygltf::Image image =
                gltfModel.images[gltfModel.textures[param.TextureIndex()].source];

            std::shared_ptr<Texture> texture
                = textureFactory->Create(
                    image.component,
                    image.image.data(),
                    image.width,
                    image.height,
                    TexturePattern::NORMAL
                );

            materialBuilder->withNormalTexture(param.TextureTexCoord(), texture);
        }
        if (mat.additionalValues.find("emissiveTexture") != mat.additionalValues.end()) 
        {
            tinygltf::Parameter param = mat.additionalValues["emissiveTexture"];

            const tinygltf::Image image =
                gltfModel.images[gltfModel.textures[param.TextureIndex()].source];

            std::shared_ptr<Texture> texture
                = textureFactory->Create(
                    image.component,
                    image.image.data(),
                    image.width,
                    image.height,
                    TexturePattern::NORMAL
                );

            materialBuilder->withEmissiveTexture(param.TextureTexCoord(), texture);
        }
        if (mat.additionalValues.find("occlusionTexture") != mat.additionalValues.end()) 
        {
            tinygltf::Parameter param = mat.additionalValues["occlusionTexture"];

            const tinygltf::Image image =
                gltfModel.images[gltfModel.textures[param.TextureIndex()].source];

            std::shared_ptr<Texture> texture
                = textureFactory->Create(
                    image.component,
                    image.image.data(),
                    image.width,
                    image.height,
                    TexturePattern::NORMAL
                );

            materialBuilder->withOcclusionTexture(param.TextureTexCoord(), texture);
        }
        if (mat.additionalValues.find("alphaCutoff") != mat.additionalValues.end()) 
        {
            materialBuilder->withAlphaMaskCutOff(static_cast<float>(mat.additionalValues["alphaCutoff"].Factor()));
        }
        if (mat.additionalValues.find("emissiveFactor") != mat.additionalValues.end()) 
        {
            materialBuilder->withEmissiveFactor(glm::vec4(glm::make_vec3(mat.additionalValues["emissiveFactor"].ColorFactor().data()), 1.0));
        }

		std::shared_ptr<Material> material = materialBuilder->Create(materialBuilder->Build());

        model->materials.push_back(material);
    }
}