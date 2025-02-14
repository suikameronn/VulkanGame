#include"Model.h"
#include"Scene.h"
#include"VulkanBase.h"

Model::Model()//3Dモデルを持つクラス
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
	slippery = 0.0f;

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
	slippery = 0.0f;

	defaultAnimationName = "none";
	currentPlayAnimationName = "none";
}

void Model::cleanupVulkan()//Vulkanの変数の後処理
{
	VkDevice device = VulkanBase::GetInstance()->GetDevice();

	for (size_t i = 0; i < pointBuffers.size(); i++)//頂点用バッファの解放
	{
		vkDestroyBuffer(device, pointBuffers[i].vertBuffer, nullptr);
		vkFreeMemory(device, pointBuffers[i].vertHandler, nullptr);

		vkDestroyBuffer(device, pointBuffers[i].indeBuffer, nullptr);
		vkFreeMemory(device, pointBuffers[i].indeHandler, nullptr);
	}

	//uniform bufferの解放
	vkDestroyBuffer(device, modelViewMappedBuffer.uniformBuffer, nullptr);
	vkFreeMemory(device, modelViewMappedBuffer.uniformBufferMemory, nullptr);
	modelViewMappedBuffer.uniformBufferMapped = nullptr;

	//アニメーションのユニフォームバッファの解放
	for (int i = 0; i < animationMappedBuffers.size(); i++)
	{
		vkDestroyBuffer(device, animationMappedBuffers[i].uniformBuffer, nullptr);
		vkFreeMemory(device, animationMappedBuffers[i].uniformBufferMemory, nullptr);
		animationMappedBuffers[i].uniformBufferMapped = nullptr;
	}

	if (colider)
	{
		colider->cleanupVulkan();//コライダーの変数の後処理
	}
}

MappedBuffer* Model::getAnimationMappedBufferData()
{
	return animationMappedBuffers.data();
}

void Model::setDefaultAnimationName(std::string name)//初期状態で再生するアニメーションを設定する
{
#ifdef DEBUG
	bool animationExist = false;
	for (int i = 0; i < animationNames.size(); i++)
	{
		if (animationNames[i] == name)
		{
			animaitonExist = true;
			break;
		}
	}
	if (!animationExist)
	{
		defaultAnimation = "none";
		return;
	}
#endif
	defaultAnimationName = name;
	currentPlayAnimationName = name;
}

void Model::setgltfModel(std::shared_ptr<GltfModel> model)//gltfモデルを設定する
{
	gltfModel = model;

	animationNames.resize(model->animations.size());
	int i = 0;
	for (auto itr = model->animations.begin(); itr != model->animations.end(); itr++)
	{
		animationNames[i] = itr->first;
		i++;
	}

	//バッファを用意する
	descSetDatas.resize(model->primitiveCount);
	jointMatrices.resize(model->jointNum);
	pointBuffers.resize(model->meshCount);
	animationMappedBuffers.resize(model->primitiveCount);
}

uint32_t Model::getimageDataCount()
{
	return imageDataCount;
}

void Model::switchPlayAnimation()
{
	currentPlayAnimationName = defaultAnimationName;
}

void Model::switchPlayAnimation(std::string nextAnimation)
{
	if (currentPlayAnimationName != nextAnimation)
	{
		animationChange = true;
		currentPlayAnimationName = nextAnimation;
	}
}

void Model::playAnimation()//アニメーション用の行列を計算する
{
	if (currentPlayAnimationName != "none")
	{
		if (deltaTime > gltfModel->animationDuration(currentPlayAnimationName) || animationChange)
		{
			animationChange = false;
			startTime = clock();
		}

		currentTime = clock();

		deltaTime = static_cast<double>(currentTime - startTime) / CLOCKS_PER_SEC;

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

	position = pos;

	sendPosToChildren(position);

	uniformBufferChange = true;
}

void Model::updateTransformMatrix()//座標変換行列を計算する
{
	transformMatrix = glm::translate(glm::mat4(1.0), position) * rotate.getRotateMatrix() * glm::scale(glm::mat4(1.0f),scale);

	if (colider)
	{
		colider->reflectMovement(transformMatrix);
	}

	uniformBufferChange = false;
}

void Model::setColider()
{
	colider = std::shared_ptr<Colider>(new Colider(gltfModel->initPoseMin, gltfModel->initPoseMax));
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

void Model::sendPosToChildren(glm::vec3 pos)
{
	for (auto itr = childObjects.begin(); itr != childObjects.end(); itr++)
	{
		(*itr)->setParentPos(pos);
	}

	if (!cameraObj.expired())
	{
		cameraObj.lock()->setParentPos(pos);
	}
}

void Model::Update()
{
	customUpdate();//オブジェクト固有の更新処理

	if (physicBase)
	{
		physicBase->Update();
		setPosition(getPosition() + physicBase->getVelocity());
	}

	playAnimation();//アニメーションの再生
}

void Model::customUpdate()
{

}

void Model::initFrameSetting()//初回フレームの処理
{
	if (lua)
	{
		lua_pushlightuserdata(lua, this);
		lua_setglobal(lua, "Data");

		luaL_dofile(lua, luaPath.c_str());
	}

	if (defaultAnimationName != "none")
	{
		switchPlayAnimation();
	}

	if (colider)
	{
		colider->initFrameSettings();
	}
}

//ボックスコライダーと線分の衝突判定を行う
std::shared_ptr<Model> Model::rayCast(glm::vec3 origin,glm::vec3 dir,float maxLength)
{
	for (int i = 1; i <= maxLength; i++)
	{
		std::shared_ptr<Model> hitModel = scene->raycast(origin,dir,i,this);
		if (hitModel)
		{
			return hitModel;
		}
	}

	return nullptr;
}

//下のほうにあるオブジェクトが床かどうか調べる
bool Model::isGround()
{
	std::shared_ptr<Model> model = rayCast(position + up * 0.1f, glm::vec3(0.0f, -1.0, 0.0f), 1.0f);
	if (model)
	{
		if (model->containTag(Tag::GROUND))
		{
			return true;
		}
	}

	return false;
}