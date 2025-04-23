#include"Scene.h"
#include"VulkanBase.h"

#include"Model.h"

Model::Model()//3D���f�������N���X
{
	scene = Scene::GetInstance();

	objNum = ObjNum::MODEL;
	tags.push_back(Tag::GROUND);

	uniformBufferChange = true;

	position = { 0,0,0 };
	posOffSet = 0.0f;

	rotate.x = rotate.getRadian(0.0f);
	rotate.y = rotate.getRadian(0.0f);
	rotate.z = rotate.getRadian(0.0f);
	scale = glm::vec3(1.0f, 1.0f, 1.0f) * 100.0f;

	forward = glm::vec3{ 0,0,1 };
	right = glm::vec3{ 1,0,0 };
	up = glm::vec3{ 0,1,0 };

	childObjects.clear();
	spherePos = false;

	rotateSpeed = 0.1f;

	deltaTime = 0.0;
	animationChange = true;

	isMovable = false;
	colider = nullptr;

	gravity = 0.0f;

	defaultAnimationName = "Idle";
}

Model::Model(std::string luaScriptPath)
{
	tags.push_back(Tag::GROUND);
	uniformBufferChange = true;

	position = { 0,0,0 };
	posOffSet = 0.0f;

	scale = glm::vec3(1.0f, 1.0f, 1.0f);

	forward = glm::vec3{ 0,0,1 };
	right = glm::vec3{ 1,0,0 };

	rotate.x = rotate.getRadian(0.0f);
	rotate.y = rotate.getRadian(0.0f);
	rotate.z = rotate.getRadian(0.0f);

	childObjects.clear();
	spherePos = false;

	rotateSpeed = 0.1f;

	deltaTime = 0.0;

	isMovable = false;
	colider = nullptr;

	gravity = 0.0f;

	defaultAnimationName = "none";
	currentPlayAnimationName = "none";
}

void Model::registerGlueFunctions()//glue�֐���ݒ肷��
{
	lua_register(lua, "glueSetPos", glueObjectFunction::glueSetPos);
	lua_register(lua, "glueSetRotate", glueObjectFunction::glueSetRotate);
	lua_register(lua, "glueSetScale", glueModelFunction::glueSetScale);
}

void Model::cleanupVulkan()//Vulkan�̕ϐ��̌㏈��
{
	VkDevice device = VulkanBase::GetInstance()->GetDevice();

	for (size_t i = 0; i < pointBuffers.size(); i++)//���_�p�o�b�t�@�̉��
	{
		vkDestroyBuffer(device, pointBuffers[i].vertBuffer, nullptr);
		vkFreeMemory(device, pointBuffers[i].vertHandler, nullptr);

		vkDestroyBuffer(device, pointBuffers[i].indeBuffer, nullptr);
		vkFreeMemory(device, pointBuffers[i].indeHandler, nullptr);
	}

	//uniform buffer�̉��
	vkDestroyBuffer(device, modelViewMappedBuffer.uniformBuffer, nullptr);
	vkFreeMemory(device, modelViewMappedBuffer.uniformBufferMemory, nullptr);
	modelViewMappedBuffer.uniformBufferMapped = nullptr;

	//�A�j���[�V�����̃��j�t�H�[���o�b�t�@�̉��
	for (int i = 0; i < (int)animationMappedBuffers.size(); i++)
	{
		vkDestroyBuffer(device, animationMappedBuffers[i].uniformBuffer, nullptr);
		vkFreeMemory(device, animationMappedBuffers[i].uniformBufferMemory, nullptr);
		animationMappedBuffers[i].uniformBufferMapped = nullptr;
	}

	if (colider)
	{
		colider->cleanupVulkan();//�R���C�_�[�̕ϐ��̌㏈��
	}
}

MappedBuffer* Model::getAnimationMappedBufferData()
{
	return animationMappedBuffers.data();
}

void Model::setDefaultAnimationName(std::string name)//������ԂōĐ�����A�j���[�V������ݒ肷��
{
	defaultAnimationName = name;
	currentPlayAnimationName = name;
}

void Model::setgltfModel(std::shared_ptr<GltfModel> model)//gltf���f����ݒ肷��
{
	gltfModel = model;

	min = gltfModel->initPoseMin;
	max = gltfModel->initPoseMax;

	animationNames.resize(model->animations.size());
	int i = 0;
	for (auto itr = model->animations.begin(); itr != model->animations.end(); itr++)
	{
		animationNames[i] = itr->first;
		i++;
	}

	//�o�b�t�@��p�ӂ���
	descSetDatas.resize(model->primitiveCount);
	jointMatrices.resize(model->jointNum);
	pointBuffers.resize(model->meshCount);
	animationMappedBuffers.resize(model->primitiveCount);
}

//�Đ�����A�j���[�V�������A�C�h�����ɍĐ�������̂ɂ���
void Model::switchPlayAnimation()
{
	currentPlayAnimationName = defaultAnimationName;
}

//�A�j���[�V������؂�ւ���
void Model::switchPlayAnimation(std::string nextAnimation)
{
	nextPlayAnimationName = nextAnimation;
}

void Model::playAnimation()//�A�j���[�V�����p�̍s����v�Z����
{
	if (currentPlayAnimationName != "none")
	{
		//�A�j���[�V�������Đ����I����
		//���邢�́A�A�j���[�V�������؂�ւ�����ꍇ
		if (deltaTime > gltfModel->animationDuration(currentPlayAnimationName)
			|| currentPlayAnimationName != nextPlayAnimationName)
		{
			currentPlayAnimationName = nextPlayAnimationName;
			//�Đ����Ԃ��Ăьv�����n�߂�
			animationChange = false;
			startTime = clock();
		}


		currentTime = clock();

		deltaTime = static_cast<double>(currentTime - startTime) / CLOCKS_PER_SEC;

		//�A�j���[�V�����s��̌v�Z
		gltfModel->updateAnimation(deltaTime, gltfModel->animations[currentPlayAnimationName], jointMatrices);
	}
}

std::array<glm::mat4, 128>& Model::getJointMatrices(int index)
{
	return this->jointMatrices[index];
}


void Model::setPosition(glm::vec3 pos)
{
	if (position == pos)
	{
		return;
	}

	lastPos = position;
	position = pos;

	sendPosToChildren();//�q�I�u�W�F�N�g�Ɉړ���`����

	uniformBufferChange = true;
}

void Model::setBaseColor(glm::vec4 baseColor)
{

}

void Model::updateTransformMatrix()//���W�ϊ��s����v�Z����
{
	transformMatrix = glm::translate(glm::mat4(1.0), position) * rotate.getRotateMatrix() * glm::scale(glm::mat4(1.0f),scale);

	//AABB�̍X�V
	min = transformMatrix * glm::vec4(initMin, 1.0f);
	max = transformMatrix * glm::vec4(initMax, 1.0f);

	//MBR�̍X�V
	calcMBR();

	if (colider)
	{
		if (colider->isConvex)
		{
			colider->reflectMovement(transformMatrix, jointMatrices);//�R���C�_�[�ɃI�u�W�F�N�g�̃g�����X�t�H�[���̕ύX�𔽉f������
		}
		else
		{
			colider->reflectMovement(transformMatrix);
		}
	}

	uniformBufferChange = false;

	//R�c���[��̃I�u�W�F�N�g�̈ʒu���X�V����
	scene->updateObjectPos(this, rNode);
}

//�R���C�_�[�p��AABB����MBR���v�Z
void Model::calcMBR()
{
	for (int i = 0; i < 3; i++)
	{
		mbrMin[i] = std::min(min[i], mbrMin[i]);
		mbrMin[i] = std::min(max[i], mbrMin[i]);
	}

	for (int i = 0; i < 3; i++)
	{
		mbrMax[i] = std::max(min[i], mbrMax[i]);
		mbrMax[i] = std::max(max[i], mbrMax[i]);
	}
}

//�R���C�_�[�̐ݒ�
void Model::setColider(bool isConvex)
{
	if (isConvex)
	{
		colider = std::shared_ptr<Colider>(new Colider(gltfModel));
	}
	else
	{
		colider = std::shared_ptr<Colider>(new Colider(gltfModel->initPoseMin, gltfModel->initPoseMax));
	}
}

bool Model::hasColider()
{
	if (colider)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//�J�����Ǝq�I�u�W�F�N�g�ȊO�ɂ��A�����̐^��ɍڂ��Ă���I�u�W�F�N�g���Ǐ]������
void Model::sendPosToChildren()
{
	for (auto itr = childObjects.begin(); itr != childObjects.end(); itr++)
	{
		(*itr)->setParentPos(lastPos,position);
	}

	for (auto itr = groundingObjects.begin(); itr != groundingObjects.end(); itr++)
	{
		(*itr)->setParentPos(lastPos, position);
	}

	if (!cameraObj.expired())
	{
		cameraObj.lock()->setParentPos(position);
	}
}

void Model::createTransformTable()//lua�ɍ��W�ϊ��p�̍s��̕ϐ����쐬
{
	lua_newtable(coroutine);
	lua_pushstring(coroutine, "x");
	lua_pushnumber(coroutine, position.x);
	lua_settable(coroutine, -3);
	lua_pushstring(coroutine, "y");
	lua_pushnumber(coroutine, position.y);
	lua_settable(coroutine, -3);
	lua_pushstring(coroutine, "z");
	lua_pushnumber(coroutine, position.z);
	lua_settable(coroutine, -3);
	lua_setglobal(coroutine, "Position");

	lua_newtable(coroutine);
	lua_pushstring(coroutine, "x");
	lua_pushnumber(coroutine, rotate.x);
	lua_settable(coroutine, -3);
	lua_pushstring(coroutine, "y");
	lua_pushnumber(coroutine, rotate.y);
	lua_settable(coroutine, -3);
	lua_pushstring(coroutine, "z");
	lua_pushnumber(coroutine, rotate.z);
	lua_settable(coroutine, -3);
	lua_setglobal(coroutine, "Rotate");

	lua_newtable(coroutine);
	lua_pushstring(coroutine, "x");
	lua_pushnumber(coroutine, scale.x);
	lua_settable(coroutine, -3);
	lua_pushstring(coroutine, "y");
	lua_pushnumber(coroutine, scale.y);
	lua_settable(coroutine, -3);
	lua_pushstring(coroutine, "z");
	lua_pushnumber(coroutine, scale.z);
	lua_settable(coroutine, -3);
	lua_setglobal(coroutine, "Scale");
}

void Model::sendTransformToLua()//lua�ɍ��W�𑗂�
{
	if (coroutine)
	{
		lua_getglobal(coroutine, "Position");
		lua_pushnumber(coroutine, position.x);
		lua_setfield(coroutine, -2, "x");
		lua_pushnumber(coroutine, position.y);
		lua_setfield(coroutine, -2, "y");
		lua_pushnumber(coroutine, position.z);
		lua_setfield(coroutine, -2, "z");

		lua_getglobal(coroutine, "Rotate");
		lua_pushnumber(coroutine, rotate.x);
		lua_setfield(coroutine, -2, "x");
		lua_pushnumber(coroutine, rotate.y);
		lua_setfield(coroutine, -2, "y");
		lua_pushnumber(coroutine, rotate.z);
		lua_setfield(coroutine, -2, "z");

		lua_getglobal(coroutine, "Scale");
		lua_pushnumber(coroutine, scale.x);
		lua_setfield(coroutine, -2, "x");
		lua_pushnumber(coroutine, scale.y);
		lua_setfield(coroutine, -2, "y");
		lua_pushnumber(coroutine, scale.z);
		lua_setfield(coroutine, -2, "z");
	}
}

void Model::receiveTransformFromLua()//�X�N���v�g�ŕω��������W�Ȃǂ��擾
{
	if (coroutine)
	{
		float x, y, z;

		lua_getglobal(coroutine, "Position");
		lua_pushstring(coroutine, "x");
		lua_gettable(coroutine, -2);
		x = static_cast<float>(lua_tonumber(coroutine,-1));
		lua_pushstring(coroutine, "y");
		lua_gettable(coroutine, -3);
		y = static_cast<float>(lua_tonumber(coroutine,-1));
		lua_pushstring(coroutine, "z");
		lua_gettable(coroutine, -4);
		z = static_cast<float>(lua_tonumber(coroutine,-1));
		lua_pop(lua, -1);
		setPosition(glm::vec3(x, y, z));

		lua_getglobal(coroutine, "Rotate");
		lua_pushstring(coroutine, "x");
		lua_gettable(coroutine, -2);
		x = static_cast<float>(lua_tonumber(coroutine,-1));
		lua_pushstring(coroutine, "y");
		lua_gettable(coroutine, -3);
		y = static_cast<float>(lua_tonumber(coroutine,-1));
		lua_pushstring(coroutine, "z");
		lua_gettable(coroutine, -4);
		z = static_cast<float>(lua_tonumber(coroutine,-1));
		lua_pop(coroutine, -1);
		rotate.x = x;
		rotate.y = y;
		rotate.z = z;

		lua_getglobal(coroutine, "Scale");
		lua_pushstring(coroutine, "x");
		lua_gettable(coroutine, -2);
		x = static_cast<float>(lua_tonumber(coroutine, -1));
		lua_pushstring(coroutine, "y");
		lua_gettable(coroutine, -3);
		y = static_cast<float>(lua_tonumber(coroutine, -1));
		lua_pushstring(coroutine, "z");
		lua_gettable(coroutine, -4);
		z = static_cast<float>(lua_tonumber(coroutine, -1));
		lua_pop(coroutine, -1);
		scale = glm::vec3(x, y, z);

		uniformBufferChange = true;//���W�ϊ��s��̍X�V
	}
}

glm::vec3 Model::getLastScale()
{
	return lastScale;
}

void Model::setLastFrameTransform()
{
	lastPos = position;
	lastRotate = rotate;
	lastScale = scale;
}

void Model::Update()
{
	setLastFrameTransform();

	customUpdate();//�I�u�W�F�N�g�ŗL�̍X�V����

	if (physicBase)
	{
		physicBase->Update();//�������Z�̍X�V

		setPosition(getPosition() + physicBase->getVelocity());//�������Z�̈ʒu��������
	}

	if (coroutine && passFrameCount >= delayFrameCount)
	{
		sendTransformToLua();//lua�ɍ��W�Ȃǂ𑗂�
		
		int nresult;
		lua_resume(coroutine, nullptr, 0, &nresult);//lua�̃R���[�`���̍ĊJ

		/*lua�X�N���v�g��ōX�V���ꂽ�g�����X�t�H�[����c++��ɂ����f������*/

		receiveTransformFromLua();//lua������W�Ȃǂ��󂯎��
	}

	playAnimation();//�A�j���[�V�����̍Đ�

	if (passFrameCount < MAXFRAMECOUNT)
	{
		passFrameCount++;
	}
}

void Model::customUpdate()
{

}

void Model::initFrameSetting()//����t���[���̏���
{
	if (lua)
	{
		luaL_dofile(lua, luaPath.c_str());
		coroutine = lua_newthread(lua);

		createTransformTable();//�I�u�W�F�N�g�̃g�����X�t�H�[����lua�ɑ���

		lua_getglobal(coroutine, "Update");
		
		int nresults;
		if (lua_resume(coroutine, nullptr, 0, &nresults) != LUA_YIELD)
		{
			std::cerr << "lua coroutine error" << std::endl;
		}
	}

	if (defaultAnimationName != "none")
	{
		switchPlayAnimation();
	}

	if (colider)
	{
		colider->initFrameSettings();//�R���C�_�[�̏����ݒ�
	}

	//AABB�ɃX�P�[����K�p����
	min = glm::scale(scale) * glm::vec4(min,1.0f);
	max = glm::scale(scale) * glm::vec4(max,1.0f);

	initMin = min;
	initMax = max;

	calcMBR();

	//�V�[���S�̂�R-tree�ɂ��̃I�u�W�F�N�g��ǉ�

	/////shared_from_this�łȂ����G���[���ł�
	scene->addModelToRTree(this);
}

//�{�b�N�X���C�L���X�g�A������maxLength�܂Ŏw��̕����ɒ����̂�L�΂��āA�R���C�_�[�Ƃ̓����蔻����s��
std::shared_ptr<Model> Model::rayCast(glm::vec3 origin,glm::vec3 dir,float maxLength,glm::vec3& normal)
{
	for (float i = 0.1f; i <= maxLength; i += 0.1f)
	{
		std::shared_ptr<Model> hitModel = scene->raycast(origin,dir,i,this,normal);
		if (hitModel)
		{
			return hitModel;
		}
	}

	return nullptr;
}

//���̂ق��ɂ���I�u�W�F�N�g�������ǂ������ׂ�
bool Model::isGround(glm::vec3& normal)
{
	std::shared_ptr<Model> model = rayCast(position + up * 1.0f, glm::vec3(0.0f, -1.0, 0.0f), 2.0f,normal);
	if (model)
	{
		if (model->containTag(Tag::GROUND))
		{
			return true;
		}
	}

	return false;
}

//�����蔻��̌��ʁA�^��ɂ����I�u�W�F�N�g�������̈ړ��ɒǏ]�����邽�߁A�z��ɒǉ�����
//�������A���̔z��͖��t���[�������������
void Model::addGroundingObject(Model* object)
{
	groundingObjects.push_back(object);
}

//�^��ɍڂ��Ă���I�u�W�F�N�g�̔z��̏�����
void Model::clearGroundingObject()
{
	groundingObjects.clear();
}

//���x�̒�~
void Model::setZeroVelocity()
{
	if (physicBase)
	{
		physicBase->setZeroVelocity();
	}
}

//�d�͂̑ł�����
void Model::cancelGravity()
{
	if (physicBase)
	{
		physicBase->cancelGravity();
	}
}