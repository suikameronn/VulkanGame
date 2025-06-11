#include"Scene.h"
#include"VulkanBase.h"

#include"Model.h"

Model::Model()//3D���f�������N���X
{
	gltfModel = nullptr;

	uvScale = false;

	scene = Scene::GetInstance();

	objNum = ObjNum::MODEL;
	tags.push_back(Tag::GROUND);

	uniformBufferChange = true;

	position = { 0,0,0 };
	posOffSet = 0.0f;

	rotate.x = rotate.getRadian(0.0f);
	rotate.y = rotate.getRadian(0.0f);
	rotate.z = rotate.getRadian(0.0f);
	scale = glm::vec3(1.0f, 1.0f, 1.0f);
	initScale = glm::vec3(1.0f);

	forward = glm::vec3{ 0,0,-1 };
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

	defaultAnimationName = "none";
	currentPlayAnimationName = "none";

	mbrMin = glm::vec3(FLT_MAX);
	mbrMax = glm::vec3(-FLT_MAX);

	rNode = nullptr;
	rNodeIndex = 0;
}

Model::Model(std::string luaScriptPath)
{
	gltfModel = nullptr;

	uvScale = false;

	tags.push_back(Tag::GROUND);
	uniformBufferChange = true;

	position = { 0,0,0 };
	posOffSet = 0.0f;

	scale = glm::vec3(1.0f, 1.0f, 1.0f);
	initScale = glm::vec3(1.0f);

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

	mbrMin = glm::vec3(FLT_MAX);
	mbrMax = glm::vec3(-FLT_MAX);

	rNode = nullptr;
	rNodeIndex = 0;

	deltaTime = 0.0;
}

void Model::registerGlueFunctions()//glue�֐���ݒ肷��
{
	lua_register(lua, "setPos", glueObjectFunction::glueSetPos);
	lua_register(lua, "setRotate", glueObjectFunction::glueSetRotate);
	lua_register(lua, "setScale", glueModelFunction::glueSetScale);
}

void Model::cleanupVulkan()//Vulkan�̕ϐ��̌㏈��
{
	VulkanBase* vulkan = VulkanBase::GetInstance();

	vulkan->addDefferedDestructBuffer(modelViewMappedBuffer);
	for (auto& buffer : animationMappedBuffers)
	{
		vulkan->addDefferedDestructBuffer(buffer);
	}

	/*
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
	*/
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

	animationNames.resize(gltfModel->animations.size());
	int i = 0;
	for (auto itr = gltfModel->animations.begin(); itr != gltfModel->animations.end(); itr++)
	{
		animationNames[i] = itr->first;
		i++;
	}

	//�o�b�t�@��p�ӂ���
	descSetDatas.resize(gltfModel->primitiveCount);
	jointMatrices.resize(gltfModel->jointNum);
	animationMappedBuffers.resize(gltfModel->primitiveCount);
}

//�Đ�����A�j���[�V�������A�C�h�����ɍĐ�������̂ɂ���
void Model::switchPlayAnimation()
{
	//currentPlayAnimationName = defaultAnimationName;
	nextPlayAnimationName = defaultAnimationName;
}

//�A�j���[�V������؂�ւ���
void Model::switchPlayAnimation(std::string nextAnimation)
{
	nextPlayAnimationName = nextAnimation;

#define _DEBUG
	if (gltfModel->animations.find(nextAnimation) == gltfModel->animations.end())
	{
		std::cerr << "not exist animation " << nextPlayAnimationName << std::endl;
	}
}

void Model::playAnimation()//�A�j���[�V�����p�̍s����v�Z����
{
	if (currentPlayAnimationName != "none")
	{
		currentTime = clock();

		deltaTime = static_cast<double>(currentTime - startTime) / CLOCKS_PER_SEC;

		//�A�j���[�V�������Đ����I����
		//���邢�́A�A�j���[�V�������؂�ւ�����ꍇ
		if (deltaTime >= gltfModel->animationDuration(currentPlayAnimationName)
			|| currentPlayAnimationName != nextPlayAnimationName)
		{
			currentPlayAnimationName = nextPlayAnimationName;
			//�Đ����Ԃ��Ăьv�����n�߂�

			startTime = clock();

			deltaTime = 0.0;
		}

		//�A�j���[�V�����s��̌v�Z
		gltfModel->updateAnimation(deltaTime, currentPlayAnimationName, nodeTransform, jointMatrices);
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
	sendPosToCamera(position - lastPos);

	uniformBufferChange = true;
}

void Model::setBaseColor(glm::vec4 baseColor)
{

}

void Model::updateTransformMatrix()//���W�ϊ��s����v�Z����
{
	transformMatrix = glm::translate(glm::mat4(1.0), position)
		* rotate.getRotateMatrix() * glm::scale(glm::mat4(1.0f),initScale * scale);

	//AABB�̍X�V
	min = transformMatrix * glm::vec4(initMin, 1.0f);
	max = transformMatrix * glm::vec4(initMax, 1.0f);

	//MBR�̍X�V
	calcMBR();

	pivot = (mbrMin + mbrMax) / 2.0f;

	if (colider)
	{
		colider->reflectMovement(transformMatrix);
	}

	uniformBufferChange = false;

	if (rNode)
	{
		//R�c���[��̃I�u�W�F�N�g�̈ʒu���X�V����
		scene->updateObjectPos(std::dynamic_pointer_cast<Model>(shared_from_this()), rNode);
	}
}

//�R���C�_�[�p��AABB����MBR���v�Z
void Model::calcMBR()
{
	mbrMin = glm::vec3(FLT_MAX);
	mbrMax = glm::vec3(-FLT_MAX);

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
void Model::setColider(bool isTrigger)
{
	hasColiderFlag = true;
	trigger = isTrigger;
}

bool Model::hasColider()
{
	return hasColiderFlag;
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
		(*itr).lock()->setParentPos(lastPos, position);
	}
}

//�J�����ɍ��W��Ǐ]������
void Model::sendPosToCamera(glm::vec3 targetPos)
{
	if (!cameraObj.expired())
	{
		cameraObj.lock()->setParentPos(targetPos);
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

	if (defaultAnimationName != "none")
	{
		switchPlayAnimation();
	}

	std::function<void()> playAnim = [this]()
		{
			playAnimation();//�A�j���[�V�����̍Đ�
		};

	threadPool->run(playAnim);
}

void Model::customUpdate()
{

}

void Model::initFrameSetting()//����t���[���̏���
{
	initFrame = false;

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
		//switchPlayAnimation();
		currentPlayAnimationName = defaultAnimationName;
		nextPlayAnimationName = defaultAnimationName;
	}

	if (hasColiderFlag)
	{
		colider = std::shared_ptr<Colider>(new Colider(gltfModel, trigger));
		colider->initFrameSettings(glm::vec3(1.0f));
	}

	initMin = min;
	initMax = max;

	//�A�j���[�V�����s��̏�����
	for (auto& matrices : jointMatrices)
	{
		std::fill(matrices.begin(), matrices.end(), glm::mat4(1.0f));
	}

	updateTransformMatrix();

	//�V�[���S�̂�R-tree�ɂ��̃I�u�W�F�N�g��ǉ�
	scene->addModelToRTree(std::static_pointer_cast<Model>(shared_from_this()));

	//gpu��Ƀo�b�t�@���쐬
	VulkanBase::GetInstance()->setModelData(std::static_pointer_cast<Model>(shared_from_this()));
}

//�{�b�N�X���C�L���X�g�A������maxLength�܂Ŏw��̕����ɒ����̂�L�΂��āA�R���C�_�[�Ƃ̓����蔻����s��
std::weak_ptr<Model> Model::rayCast(glm::vec3 origin,glm::vec3 dir,float maxLength,glm::vec3& normal)
{
	for (float i = 0.1f; i <= maxLength; i += 0.1f)
	{
		std::weak_ptr<Model> hitModel = scene->raycast(origin,dir,i,this,normal);
		if (!hitModel.expired())
		{
			return hitModel;
		}
	}

	return std::weak_ptr<Model>();
}

//���̂ق��ɂ���I�u�W�F�N�g�������ǂ������ׂ�
bool Model::isGround(glm::vec3& normal)
{
	std::weak_ptr<Model> model = rayCast(position + up * 1.0f, glm::vec3(0.0f, -1.0, 0.0f), 2.0f,normal);
	if (!model.expired())
	{
		if (model.lock()->containTag(Tag::GROUND))
		{
			return true;
		}
	}

	return false;
}

//�����蔻��̌��ʁA�^��ɂ����I�u�W�F�N�g�������̈ړ��ɒǏ]�����邽�߁A�z��ɒǉ�����
//�������A���̔z��͖��t���[�������������
void Model::addGroundingObject(std::weak_ptr<Model> object)
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

//���j�t�H�[���o�b�t�@�̍X�V
void Model::updateUniformBuffer(GltfNode* node)
{
	for (auto mesh : node->meshArray)
	{
		std::shared_ptr<Camera> camera = Storage::GetInstance()->accessCamera();

		std::array<glm::mat4, 128> array;
		std::fill(array.begin(), array.end(), glm::mat4(1.0f));

		AnimationUBO ubo;

		ubo.matrix = glm::mat4(1.0f);
		ubo.nodeMatrix = glm::mat4(1.0f);
		if (nodeTransform.nodeTransform.size() != 0)
		{
			ubo.matrix = nodeTransform.matrix[node->index];
			ubo.nodeMatrix = nodeTransform.nodeTransform[node->index];
		}
		

		if (node->skin != nullptr && node->globalHasSkinNodeIndex > -1)
		{
			ubo.boneMatrix = getJointMatrices(node->globalHasSkinNodeIndex);
		}
		else
		{
			ubo.boneMatrix = array;
		}

		ubo.boneCount = node->getJointCount();

		memcpy(animationMappedBuffers[mesh->meshIndex].uniformBufferMapped, &ubo, sizeof(ubo));

	}

	for (int i = 0; i < node->children.size(); i++)
	{
		updateUniformBuffer(node->children[i]);
	}
}

void Model::updateUniformBuffer(std::list<std::shared_ptr<DirectionalLight>>& dirLights
	, std::list<std::shared_ptr<PointLight>>& pointLights, ShadowMapData& shadowMapData)
{
	Storage* storage = Storage::GetInstance();

	MatricesUBO ubo{};

	if (applyScaleUV())
	{
		//���f���̃X�P�[���ɍ��킹��uv���W�𒲐�����
		ubo.scale = scale * initScale;
	}
	else
	{
		ubo.scale = glm::vec3(1.0f);
	}

	std::shared_ptr<Camera> camera = storage->accessCamera();

	ubo.model = transformMatrix;
	ubo.view = camera->viewMat;
	ubo.proj = camera->perspectiveMat;
	ubo.worldCameraPos = glm::vec4(camera->getPosition(), 1.0f);

	ubo.lightCount = static_cast<int>(std::min(ubo.lightMVP.size(), dirLights.size()));

	{
		int index = 0;
		for (auto itr = dirLights.begin(); itr != dirLights.end(); itr++)
		{
			ubo.lightMVP[index] = shadowMapData.proj
				* glm::lookAt((*itr)->getPosition(), (*itr)->getPosition() + (*itr)->direction, glm::vec3(0.0f, 1.0f, 0.0f));

			if (index >= ubo.lightCount)
			{
				break;
			}

			index++;
		}
	}

	memcpy(modelViewMappedBuffer.uniformBufferMapped, &ubo, sizeof(ubo));

	updateUniformBuffer(gltfModel->getRootNode());

	if (hasColider())
	{
		MatricesUBO ubo{};

		ubo.model = transformMatrix;
		ubo.view = camera->viewMat;
		ubo.proj = camera->perspectiveMat;
		ubo.worldCameraPos = glm::vec4(camera->getPosition(), 1.0);

		memcpy(colider->getMappedBufferData()->uniformBufferMapped, &ubo, sizeof(ubo));
	}
}

void Model::frameEnd(std::list<std::shared_ptr<DirectionalLight>>& dirLights
	, std::list<std::shared_ptr<PointLight>>& pointLights, ShadowMapData& shadowMapData)
{
	if (uniformBufferChange)
	{
		updateTransformMatrix();
	}

	updateUniformBuffer(dirLights, pointLights, shadowMapData);
}