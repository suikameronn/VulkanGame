#include"Scene.h"

#include"Object.h"

Object::Object()//�Q�[�����ɓo�ꂷ��I�u�W�F�N�g�̂��ׂĂ��p������N���X
{
	objNum = ObjNum::OBJECT;

	physicBase = std::make_unique<PhysicBase>();

	uniformBufferChange = true;

	position = { 0,0,0 };
	posOffSet = 0.0f;

	forward = glm::vec3{ 0,0,-1 };
	right = glm::vec3{ 1,0,0 };
	
	rotate.x = rotate.getRadian(0.0f);
	rotate.y = rotate.getRadian(0.0f);
	rotate.z = rotate.getRadian(0.0f);

	childObjects.clear();
	spherePos = false;

	rotateSpeed = 0.1f;
	length = 1.0f;

	lua = nullptr;
	coroutine = nullptr;
}

bool Object::containTag(Tag tag)//�w�肳�ꂽ�^�O�������Ă��邩�ǂ�����Ԃ�
{
	for (auto itr = tags.begin(); itr != tags.end(); itr++)
	{
		if (tag == (*itr))
		{
			return true;
		}
	}

	return false;
}

void Object::setLuaScript(std::string path)//�s���p�^�[���p��lua�X�N���v�g��ݒ肷�� ���s�͂��Ȃ�
{
	luaPath = path;
	lua = luaL_newstate();
	luaL_openlibs(lua);

	registerGlueFunctions();
}

//lua�X�N���v�g�����s����̂�x������t���[������ݒ肷��
void Object::setDelayFrameCount(int delay)
{
	delayFrameCount = delay;
}

//lua����Ăяo�����ÓI�֐��̓o�^
void Object::registerGlueFunctions()
{
	lua_register(lua, "setPos", glueObjectFunction::glueSetPos);
	lua_register(lua, "setRotate", glueObjectFunction::glueSetRotate);
}

void Object::bindObject(Object* obj)//�����̃I�u�W�F�N�g���q�I�u�W�F�N�g�Ƃ��Đݒ肷��
{
	childObjects.push_back(obj);

	sendPosToChildren();
}

void Object::bindCamera(std::weak_ptr<Camera> camera)//�J�����������ɒǏ]������
{
	cameraObj = camera;

	sendPosToChildren();
}

glm::vec3 Object::inputMove()//�L�[���͂���ړ�������
{
	glm::vec3 moveDirec;
	auto controller = Controller::GetInstance();

	if (cameraObj.expired())//���̃I�u�W�F�N�g���J������Ǐ]�����Ă���ꍇ
	{
		//���̃I�u�W�F�N�g�̐��ʁA�E�̃x�N�g�����J�����ɍ��킹��
		//����ŁA�I�u�W�F�N�g�����_�̂ɍ��킹�Đi�ނ悤�ɂȂ�
		forward = cameraObj.lock()->forward;
		right = cameraObj.lock()->right;
	}

	if (controller->getKey(GLFW_KEY_W))
	{
		moveDirec = -forward;
	}
	else if (controller->getKey(GLFW_KEY_A))
	{
		moveDirec = -right;
	}
	else if (controller->getKey(GLFW_KEY_D))
	{
		moveDirec = right;
	}
	else if (controller->getKey(GLFW_KEY_S))
	{
		moveDirec = forward;
	}
	else
	{
		moveDirec = { 0,0,0 };
	}

	return moveDirec;
}

//lua�X�N���v�g�Ɏ��g�̌��݂̍��W�A��]�̒l�𑗂�
void Object::sendTransformToLua()
{
	if (lua)
	{
		lua_getglobal(lua, "position");
		lua_pushnumber(lua, position.x);
		lua_setfield(lua, -2, "x");
		lua_pushnumber(lua, position.y);
		lua_setfield(lua, -2, "y");
		lua_pushnumber(lua, position.z);
		lua_setfield(lua, -2, "z");
		lua_pop(lua, -1);

		lua_getglobal(lua, "rotate");
		lua_pushnumber(lua, rotate.x);
		lua_setfield(lua, -2, "x");
		lua_pushnumber(lua, rotate.y);
		lua_setfield(lua, -2, "y");
		lua_pushnumber(lua, rotate.z);
		lua_setfield(lua, -2, "z");
		lua_pop(lua, -1);
	}
}

//lua�X�N���v�g����A���W�A��]�̒l���󂯎��
void Object::receiveTransformFromLua()
{
	if (lua)
	{
		float x, y, z;

		lua_getglobal(lua, "position");
		x = static_cast<float>(lua_tonumber(lua, -4));
		y = static_cast<float>(lua_tonumber(lua, -3));
		z = static_cast<float>(lua_tonumber(lua, -2));
		lua_pop(lua, -1);

		lua_getglobal(lua, "rotate");
		x = static_cast<float>(lua_tonumber(lua, -4));
		y = static_cast<float>(lua_tonumber(lua, -3));
		z = static_cast<float>(lua_tonumber(lua, -2));
		lua_pop(lua, -1);
	}
}

bool Object::Update()//�X�V����
{
	customUpdate();
	
	return SHOULD_KEEP;
}

void Object::setPosition(glm::vec3 pos)//���W�̐ݒ�
{
	if (position == pos)
	{
		return;
	}

	position = pos;

	sendPosToChildren();//�q�I�u�W�F�N�g������ꍇ�A�Ǐ]������

	uniformBufferChange = true;
}

glm::vec3 Object::getPosition()
{
	return position;
}

//���f���s����擾
glm::mat4 Object::getTransformMatrix()
{
	return transformMatrix;
}

void Object::sendPosToChildren()//�����̈ړ����q�I�u�W�F�N�g�ɑ���A�q�I�u�W�F�N�g�������̈ړ��ɒǏ]������
{
	for (auto itr = childObjects.begin(); itr != childObjects.end(); itr++)
	{
		if (*itr)
		{
			(*itr)->setParentPos(lastPos,position);
		}
	}

	if (!cameraObj.expired())
	{
		cameraObj.lock()->setParentPos(position);
	}
}

void Object::setParentPos(glm::vec3 lastPos,glm::vec3 currentPos)//�e�I�u�W�F�N�g�̈ړ����󂯎��A�e�I�u�W�F�N�g�̈ړ��ɒǏ]����
{
	this->lastPos = position;
	position += currentPos - lastPos;

	sendPosToChildren();
}

//lua�̉��z�}�V���ɁA���g�̍��W�Ɖ�]���L�^����ϐ����쐬
void Object::createTransformTable()
{
	lua_newtable(coroutine);
	lua_pushstring(coroutine, "x");
	lua_pushnumber(coroutine, position.x);
	lua_pushstring(coroutine, "y");
	lua_pushnumber(coroutine, position.y);
	lua_pushstring(coroutine,"z");
	lua_pushnumber(coroutine, position.z);
	lua_setglobal(coroutine, "Position");

	lua_newtable(coroutine);
	lua_pushstring(coroutine, "x");
	lua_pushnumber(coroutine, rotate.x);
	lua_pushstring(coroutine, "y");
	lua_pushnumber(coroutine, rotate.y);
	lua_pushstring(coroutine, "z");
	lua_pushnumber(coroutine, rotate.z);
	lua_setglobal(coroutine, "Rotate");
}

void Object::initFrameSetting()//����t���[���̂ݎ��s�Alua�X�N���v�g�����s����
{
	if (lua)
	{
		luaL_dofile(lua, luaPath.c_str());
		coroutine = lua_newthread(lua);
		lua_getglobal(coroutine, "Update");

		createTransformTable();//lua��ɕϐ��̍쐬
	}
}

//�O��t���[���̃g�����X�t�H�[�����擾����
glm::vec3 Object::getLastPosition()
{
	return lastPos;
}

Rotate Object::getLastRotate()
{
	return lastRotate;
}

//��O�̃t���[���̍��W�Ȃǂ̃f�[�^��ݒ�
void Object::setLastFrameTransform()
{
	lastPos = position;
	lastRotate = rotate;
}