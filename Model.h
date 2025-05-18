#pragma once

#include<vector>
#include<bitset>
#include <time.h>

#include"EnumList.h"
#include"GltfModel.h"
#include"Colider.h"
#include"RTree.h"
#include"Light.h"

class Scene;

//�t���[���o�b�t�@�Ƃ��Ă�gpu��̉摜�p�̃o�b�t�@�[�̍\����
//���̉摜�ւ̃r���[������
struct FrameBufferAttachment {
	VkImage image;
	VkDeviceMemory memory;
	VkImageView view;

	void destory(VkDevice& device)
	{
		vkDestroyImageView(device, view, nullptr);
		vkDestroyImage(device, image, nullptr);
		vkFreeMemory(device, memory, nullptr);
	}
};

//�V���h�E�}�b�s���O��L���[�u�}�b�s���O�p�̃I�t�X�N���[�������_�����O�p�̍\����
struct OffScreenPass {
	int32_t width, height;//�����_�����O�̏o�̓T�C�Y
	std::vector<VkFramebuffer> frameBuffer;//�����_�����O�̏o�͐�̃o�b�t�@�[
	std::vector<FrameBufferAttachment> imageAttachment;//�����_�����O�̏o�͐����������
	VkRenderPass renderPass;//���p���郌���_�[�p�X
	VkSampler sampler;//�����_�����O���ʂւ̃T���v���[
	VkDescriptorSetLayout layout;//�����_�����O�p�̃��C�A�E�g
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;
	std::vector<VkDescriptorSet> descriptorSets;

	OffScreenPass()
	{
		width = 0;
		height = 0;

		for (auto buffer : frameBuffer)
		{
			buffer = nullptr;
		}

		for (auto attachment : imageAttachment)
		{
			attachment.image = nullptr;
			attachment.memory = nullptr;
			attachment.view = nullptr;
		}

		renderPass = nullptr;
		sampler = nullptr;
		layout = nullptr;
		pipelineLayout = nullptr;
		pipeline = nullptr;

		for (auto descriptorSet : descriptorSets)
		{
			descriptorSet = nullptr;
		}
	}

	void setFrameCount(int count)//�I�t�X�N���[�������_�����O���s���t���[�����̐ݒ�
	{
		frameBuffer.resize(count);
		imageAttachment.resize(count);
		descriptorSets.resize(count);
	}

	void destroy(VkDevice& device)
	{
		vkDestroyPipeline(device, pipeline, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(device, layout, nullptr);

		for (auto& framebuffer : frameBuffer)
		{
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		}

		for (auto& attachment : imageAttachment)
		{
			attachment.destory(device);
		}

		vkDestroySampler(device, sampler, nullptr);

		vkDestroyRenderPass(device, renderPass, nullptr);
	}
};

struct FragmentParam
{
	//�����x���㏑��
	float alphaness;

	FragmentParam()
	{
		//���蓾�Ȃ��l�ɂ���
		//���̒l���g�����g��Ȃ����̃t���O�ɂ���
		alphaness = -1.0f;
	}
};

//�ʏ�̃����_�����O�p��uniform buffer
struct MatricesUBO {
	glm::vec3 scale;//uv���W�����p
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
	alignas(16) glm::vec4 worldCameraPos;
	alignas(16) int lightCount;
	alignas(16) std::array<glm::mat4, 20> lightMVP;//���C�g�̍s��
};

//�ʏ�̃����_�����O�̃A�j���[�V�����p�̍s��
struct AnimationUBO
{
	alignas(16) glm::mat4 nodeMatrix;
	alignas(16) glm::mat4 matrix;
	alignas(16) std::array<glm::mat4, 128> boneMatrix;
	alignas(16) int boneCount;
};

//�V���h�E�}�b�v�쐬�p��uniform buffer
struct ShadowMapUBO
{
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

//�V���h�E�}�b�v�쐬�p�̍\����
struct ShadowMapData
{
	//���t���e�̍s��
	glm::mat4 proj;

	//�V���h�E�}�b�v�̉𑜓x�̔{����ݒ�
	int shadowMapScale;
	//�I�t�X�N���[�������_�����O�p�̍\����
	OffScreenPass passData;
	//�V���h�E�}�b�v�쐬�p�̍s��̔z��
	std::vector<ShadowMapUBO> matUBOs;
	//�s��p�̃o�b�t�@�̔z��
	std::vector<MappedBuffer> mappedBuffers;

	//�V���h�E�}�b�v��ʏ�̃����_�����O�Ŏg�p���邽�߂̃f�[�^
	std::vector<VkDescriptorSet> descriptorSets;

	//�V�[����̃��C�g�̐������쐬
	void setFrameCount(int frameCount)
	{
		matUBOs.resize(frameCount);
		mappedBuffers.resize(frameCount);
		passData.setFrameCount(frameCount);
	}

	void destroy(VkDevice& device)
	{
		passData.destroy(device);

		for (auto& buffer : mappedBuffers)
		{
			buffer.destroy(device);
		}
	}
};

//3D���f�������I�u�W�F�N�g��S���N���X
class Model:public Object
{
protected:

	//�I�u�W�F�N�g�̒��S�ʒu
	glm::vec3 pivot;

	//�������`��̃t���b�O
	bool transparent;

	//R-tree�p�̏�����AABB
	glm::vec3 initMin, initMax;
	//R-tree�p��AABB
	glm::vec3 min, max;

	//���ݏ������Ă���RTree�̃m�[�h
	RNode<Model>* rNode;

	//R-tree�p��MBR�ɂ��čő�l�ƍŏ��l
	glm::vec3 mbrMin, mbrMax;

	//3D���f���̏����X�P�[��
	glm::vec3 initScale;
	
	//���C�L���X�g���g�p
	Scene* scene;

	//���������p
	std::unique_ptr<PhysicBase> physicBase;

	//uv�ɃX�P�[���������A�e�N�X�`���̈������΂���h��
	bool uvScale;

	//gltf���f���ւ̎Q��
	std::shared_ptr<GltfModel> gltfModel;

	//�R���C�_�[�������ǂ���
	bool hasColiderFlag;
	//�����蔻����������邩�ǂ���
	bool trigger;

	//mvp�s��p�̃o�b�t�@�[
	MappedBuffer modelViewMappedBuffer;
	//�A�j���[�V�����p�s��̃o�b�t�@
	std::vector<MappedBuffer> animationMappedBuffers;
	//�A�j���[�V�����p�s��̔z��
	std::vector<std::array<glm::mat4, 128>> jointMatrices;

	//�A�j���[�V�����J�n����
	clock_t startTime;
	//���݂̎���
	clock_t currentTime;
	//�A�j���[�V�����̍Đ�����
	double deltaTime;
	//�A�j���[�V�����̐؂�ւ��t���b�O�A�A�j���[�V�����J�n���Ԃ����Z�b�g
	//����A�j���[�V���������[�v������ہA�A�j���[�V�������I�������Ƃ���true��
	bool animationChange;

	//gltf���f���̃}�e���A���̏㏑���p
	std::shared_ptr<Material> material;
	//�R���C�_�[
	std::shared_ptr<Colider> colider;

	//�A�C�h�����ɍĐ�����A�j���[�V�����̖��O
	std::string defaultAnimationName;
	//���ݍĐ����Ă���A�j���[�V�����̖��O
	std::string currentPlayAnimationName;
	//���̍Đ�����\��̃A�j���[�V����
	std::string nextPlayAnimationName;
	//gltf���f���̎��A�j���[�V�����̖��O�̔z��
	std::vector<std::string> animationNames;

	//���g�����̏ꍇ�A��ɍڂ��Ă���I�u�W�F�N�g�̔z��A���g�̈ړ����A�����̃I�u�W�F�N�g���Ǐ]������
	std::list<std::weak_ptr<Model>> groundingObjects;

	//�R���C�_�[�p��AABB����MBR���v�Z
	void calcMBR();

	//�t���O�����g�V�F�[�_�ɓn���p�����[�^
	FragmentParam fragParam;

	virtual void updateUniformBuffer(GltfNode* node);
	virtual void updateUniformBuffer(std::list<std::shared_ptr<DirectionalLight>>& dirLights
		, std::list<std::shared_ptr<PointLight>>& pointLights, ShadowMapData& shadowMapData);

	virtual void cleanupVulkan();//Vulkan�̕ϐ��̔j��

public:

	Model();
	Model(std::string luaScriptPath);
	~Model()
	{
		if (rNode)
		{
			rNode->deleteObject(this);
		}

		if (lua)
		{
			lua_close(lua);
		}

		cleanupVulkan();
	}

	bool isTransparent()
	{
		return transparent;
	}

	void setTransparent(bool t)
	{
		transparent = t;
	}

	glm::vec3 getPivot()
	{
		return pivot;
	}

	//���g��weak_ptr��Ԃ�
	std::weak_ptr<Model> getThisWeakPtr()
	{
		return std::dynamic_pointer_cast<Model>(shared_from_this());
	}

	void registerGlueFunctions() override;//glue�֐��̐ݒ�

	float gravity;//�d�͂̋���
	void setZeroVelocity();//���x�̃��Z�b�g
	void cancelGravity();//�d�͂̑ł�����

	//RTree�p��AABB��ݒ肷��
	void getMbrMinMax(glm::vec3& min, glm::vec3& max)
	{
		min = this->mbrMin;
		max = this->mbrMax;
	}

	glm::vec3 getMbrMin()
	{
		return mbrMin;
	}

	glm::vec3 getMbrMax()
	{
		return mbrMax;
	}

	FragmentParam& getFragmentParam() { return fragParam; }

	//�����X�P�[����ݒ�
	void setInitScale(glm::vec3 s) { initScale = s; }

	glm::vec3 getScale() { return initScale * scale; }

	//���ݏ������Ă���m�[�h��ݒ肷��
	void setRNode(RNode<Model>* node) { rNode = node; }

	//�X�P�[��
	glm::vec3 scale;

	//���������̏�ɗ����Ă��邩�ǂ����A�����Ă���ꍇ�͏d�͂̉e���𖳎�����
	bool isGrounding;

	void setDefaultAnimationName(std::string name);//�f�t�H���g�̃A�j���[�V������ݒ�

	void sendPosToChildren();//�q�I�u�W�F�N�g�ɐe�̈ړ��𔽉f
	void sendPosToCamera(glm::vec3 targetPos);//�J�����Ɏw�肵���ʒu��Ǐ]������

	void setUVScale() { uvScale = true; }//uv�ɃX�P�[���������A�e�N�X�`���̈������΂���h���悤�ݒ肷��
	bool applyScaleUV() { return uvScale; }

	void setgltfModel(std::shared_ptr<GltfModel> model);//3DCG���f����o�^
	GltfNode* getRootNode() { return gltfModel->getRootNode(); }
	std::shared_ptr<GltfModel> getGltfModel() { return gltfModel; }

	void setBaseColor(glm::vec4 baseColor);//���f���̐F���㏑��

	void switchPlayAnimation();//�A�j���[�V�������f�t�H���g�ɐ؂�ւ���
	void switchPlayAnimation(std::string nextAnimation);//�w��̃A�j���[�V�����ɐ؂�ւ���
	void playAnimation();//�A�j���[�V�����̍Đ�
	std::array<glm::mat4, 128>& getJointMatrices(int index);//����̃X�P���g���̃A�j���[�V�����s����v�Z

	std::vector<DescSetData> descSetDatas;
	BufferObject* getPointBufferData() { return gltfModel->getPointBuffer(); }//���_�p�o�b�t�@�̎擾
	MappedBuffer& getModelViewMappedBuffer() { return modelViewMappedBuffer; }//���f���r���[�s��p�̃o�b�t�@�̎擾
	MappedBuffer* getAnimationMappedBufferData();//�A�j���[�V�����p�̍s��̃o�b�t�@�̎擾

	//�Փ˂��������A���̃I�u�W�F�N�g�𓮂����邩�ǂ����A��Q���̏ꍇ�A���̃I�u�W�F�N�g�͓������Ȃ�����,false�ƂȂ�
	bool isMovable;
	//�R���C�_�[�������Ă��邩�ǂ���
	bool hasColider();
	void setColider(bool isTrigger);//�R���C�_�[�̐ݒ�
	std::shared_ptr<Colider> getColider() { return colider; }
	
	std::weak_ptr<Model> rayCast(glm::vec3 origin,glm::vec3 dir,float maxLength,glm::vec3& normal);

	void updateTransformMatrix() override;//���W�ϊ��s��̍X�V

	void setPosition(glm::vec3 pos) override;//�ʒu�̐ݒ�

	void Update() override;//�X�V����
	void customUpdate() override;//����ȍX�V����

	glm::vec3 getLastScale();
	void setLastFrameTransform() override;//��O�̃t���[���̍��W�Ȃǂ̃f�[�^��ݒ�

	void createTransformTable() override;

	void sendTransformToLua() override;//lua�ɍ��W�Ȃǂ𑗂�
	void receiveTransformFromLua() override;//lua������W�Ȃǂ��󂯎��

	void initFrameSetting() override;//�����t���[���݂̂̏���

	bool isGround(glm::vec3& normal);//�I�u�W�F�N�g�����ɐڂ��Ă��邩�ǂ���
	void addGroundingObject(std::weak_ptr<Model> object);//���ɐڂ��Ă����炻���ǉ�
	void clearGroundingObject();

	virtual void frameEnd(std::list<std::shared_ptr<DirectionalLight>>& dirLights
		, std::list<std::shared_ptr<PointLight>>& pointLights, ShadowMapData& shadowMapData);

	//�Փˎ��̏���(���ɉ������Ȃ�)
	void collision(std::weak_ptr<Model> model) {};
};

/*�ȉ��̐ÓI�֐���lua�X�N���v�g�ォ��Ăяo�����*/

namespace glueModelFunction//Model�N���X�p��glue�֐�
{
	//Model�N���X�̃X�P�[���̐ݒ�
	static int glueSetScale(lua_State* lua)
	{
		Object* obj = static_cast<Object*>(lua_touserdata(lua, -1));

		switch (obj->getObjNum())
		{
		case 1:
			Model* model = dynamic_cast<Model*>(obj);
			model->scale.x = static_cast<float>(lua_tonumber(lua, -4));
			model->scale.y = static_cast<float>(lua_tonumber(lua, -3));
			model->scale.z = static_cast<float>(lua_tonumber(lua, -2));
			break;
		}

		return 0;
	}
}