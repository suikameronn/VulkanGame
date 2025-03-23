#pragma once
#include<vector>
#include<bitset>
#include <time.h>

#include"EnumList.h"
#include"GltfModel.h"
#include"Colider.h"

class Scene;

//3D���f�������I�u�W�F�N�g��S���N���X
class Model:public Object
{
protected:

	//���C�L���X�g���g�p
	Scene* scene;

	//���������p
	std::unique_ptr<PhysicBase> physicBase;

	//gltf���f���ւ̎Q��
	std::shared_ptr<GltfModel> gltfModel;

	//mvp�s��p�̃o�b�t�@�[
	MappedBuffer modelViewMappedBuffer;
	//gltf���f���̒��_�֘A�p�̃o�b�t�@
	std::vector<BufferObject> pointBuffers;
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
	//gltf���f���̎��A�j���[�V�����̖��O�̔z��
	std::vector<std::string> animationNames;
	//���g�����̏ꍇ�A��ɍڂ��Ă���I�u�W�F�N�g�̔z��A���g�̈ړ����A�����̃I�u�W�F�N�g���Ǐ]������
	std::vector<std::weak_ptr<Model>> groundingObjects;

public:

	Model();
	Model(std::string luaScriptPath);
	~Model() {};

	void registerGlueFunctions() override;//glue�֐��̐ݒ�

	float gravity;//�d�͂̋���
	void setZeroVelocity();//���x�̃��Z�b�g
	void cancelGravity();//�d�͂̑ł�����

	//�X�P�[��
	glm::vec3 scale;

	//�R���C�_�[�̓��b�V�����ǂ���
	bool isMeshColider;

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

	//�Փ˂��������A���̃I�u�W�F�N�g�𓮂����邩�ǂ����A��Q���̏ꍇ�A���̃I�u�W�F�N�g�͓������Ȃ�����,false�ƂȂ�
	bool isMovable;
	//�R���C�_�[�������Ă��邩�ǂ���
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
			Model * model = dynamic_cast<Model*>(obj);
			model->scale.x = static_cast<float>(lua_tonumber(lua, -4));
			model->scale.y = static_cast<float>(lua_tonumber(lua, -3));
			model->scale.z = static_cast<float>(lua_tonumber(lua, -2));
			break;
		}

		return 0;
	}
}