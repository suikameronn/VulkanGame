#pragma once
#include<vector>
#include<bitset>
#include <time.h>

#include"EnumList.h"
#include"GltfModel.h"
#include"Colider.h"

class Scene;

template<typename T>
uint32_t getSize(T v)
{
	return v.size();
}

class Model:public Object
{
protected:
	Scene* scene;

	uint32_t imageDataCount;

	std::unique_ptr<PhysicBase> physicBase;

	std::shared_ptr<GltfModel> gltfModel;

	MappedBuffer modelViewMappedBuffer;
	std::vector<BufferObject> pointBuffers;
	std::vector<MappedBuffer> animationMappedBuffers;
	std::vector<std::array<glm::mat4, 128>> jointMatrices;

	clock_t startTime;
	clock_t currentTime;
	double deltaTime;
	bool animationChange;

	std::shared_ptr<Material> material;
	std::shared_ptr<Colider> colider;

	std::string defaultAnimationName;
	std::string currentPlayAnimationName;
	std::vector<std::string> animationNames;

	std::vector<std::weak_ptr<Model>> groundingObjects;

public:

	Model();
	Model(std::string luaScriptPath);
	~Model() {};

	void registerGlueFunctions() override;//glue�֐��̐ݒ�

	float gravity;
	void setZeroVelocity();//���x�̃��Z�b�g
	void cancelGravity();//�d�͂̑ł�����

	float slippery;

	glm::vec3 scale;

	void setDefaultAnimationName(std::string name);//�f�t�H���g�̃A�j���[�V������ݒ�

	void sendPosToChildren();//�q�I�u�W�F�N�g�ɐe�̈ړ��𔽉f

	void setgltfModel(std::shared_ptr<GltfModel> model);//3DCG���f����o�^
	GltfNode* getRootNode() { return gltfModel->getRootNode(); }
	std::shared_ptr<GltfModel> getGltfModel() { return gltfModel; }

	void setBaseColor(glm::vec4 baseColor);//���f���̐F���㏑��

	void switchPlayAnimation();//�A�j���[�V�������f�t�H���g�ɐ؂�ւ���
	void switchPlayAnimation(std::string nextAnimation);//�w��̃A�j���[�V�����ɐ؂�ւ���
	void playAnimation();//�A�j���[�V�����̍Đ�
	std::array<glm::mat4, 128>& getJointMatrices(int index);//����̃X�P���g���̃A�j���[�V�����s����v�Z

	std::vector<DescSetData> descSetDatas;
	BufferObject* getPointBufferData() { return pointBuffers.data(); }//���_�p�o�b�t�@�̎擾
	MappedBuffer& getModelViewMappedBuffer() { return modelViewMappedBuffer; }//���f���r���[�s��p�̃o�b�t�@�̎擾
	MappedBuffer* getAnimationMappedBufferData();//�A�j���[�V�����p�̍s��̃o�b�t�@�̎擾
	uint32_t getimageDataCount();

	bool isMovable;
	bool hasColider();
	void setColider();//�R���C�_�[�̐ݒ�
	std::shared_ptr<Colider> getColider() { return colider; }

	std::shared_ptr<Model> rayCast(glm::vec3 origin,glm::vec3 dir,float maxLength);

	void updateTransformMatrix() override;//���W�ϊ��s��̍X�V

	void cleanupVulkan();//Vulkan�̕ϐ��̔j��

	void setPosition(glm::vec3 pos) override;//�ʒu�̐ݒ�

	void Update() override;//�X�V����
	void customUpdate() override;//����ȍX�V����

	glm::vec3 getLastScale();
	void setLastFrameTransform() override;//��O�̃t���[���̍��W�Ȃǂ̃f�[�^��ݒ�

	void createTransformTable() override;

	void sendTransformToLua() override;//lua�ɍ��W�Ȃǂ𑗂�
	void receiveTransformFromLua() override;//lua������W�Ȃǂ��󂯎��

	void initFrameSetting() override;//�����t���[���݂̂̏���

	bool isGround();//�I�u�W�F�N�g�����ɐڂ��Ă��邩�ǂ���
	void addGroundingObject(std::weak_ptr<Model> object);//���ɐڂ��Ă����炻���ǉ�
	void clearGroundingObject();
};

namespace glueModelFunction//Model�N���X�p��glue�֐�
{
	static int glueSetScale(lua_State* lua)
	{
		//lua_getglobal(lua, "object");
		Object* obj = static_cast<Object*>(lua_touserdata(lua, -1));

		switch (obj->getObjNum())
		{
		case 1:
			Model * model = dynamic_cast<Model*>(obj);
			model->scale.x = static_cast<float>(lua_tonumber(lua, -4));
			model->scale.y = static_cast<float>(lua_tonumber(lua, -3));
			model->scale.z = static_cast<float>(lua_tonumber(lua, -2));
			break;
		}

		return 0;
	}
}