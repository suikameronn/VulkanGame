#include"Object.h"

Object::Object()//�Q�[�����ɓo�ꂷ��I�u�W�F�N�g�̂��ׂĂ��p������N���X
{
	objNum = ObjNum::OBJECT;

	physicBase = std::make_unique<PhysicBase>();

	uniformBufferChange = true;

	position = { 0,0,0 };
	posOffSet = 0.0f;

	forward = glm::vec3{ 0,0,1 };
	right = glm::vec3{ 1,0,0 };
	
	rotate.x = rotate.getRadian(0.0f);
	rotate.y = rotate.getRadian(0.0f);
	rotate.z = rotate.getRadian(0.0f);

	childObjects.clear();
	spherePos = false;

	rotateSpeed = 0.1f;
	length = 1.0f;
}

bool Object::containTag(Tag tag)//�I�u�W�F�N�g�����̃^�O�������Ă��邩�ǂ���
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

void Object::setLuaScript(std::string path)//lua�X�N���v�g��ݒ�
{
	luaPath = path;
	lua = luaL_newstate();
	luaL_openlibs(lua);
}

void Object::registerGlueFunctions()
{

}

void Object::bindObject(Object* obj)//�I�u�W�F�N�g�ɐe�q�֌W��ݒ肷��
{
	parentPos = obj->getPosition();
	childObjects.push_back(obj);

	sendPosToChildren(position);
}

void Object::bindCamera(std::weak_ptr<Camera> camera)//�J���������̃I�u�W�F�N�g��Ǐ]������
{
	cameraObj = camera;

	sendPosToChildren(position);
}

glm::vec3 Object::inputMove()//�L�[���͂���ړ�������
{
	glm::vec3 moveDirec;
	auto controller = Controller::GetInstance();

	if (cameraObj.expired())
	{
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

void Object::Update()//�X�V����
{
	customUpdate();
}

void Object::setPosition(glm::vec3 pos)//���W�̐ݒ�
{
	if (position == pos)
	{
		return;
	}

	position = pos;

	sendPosToChildren(position);//�q�I�u�W�F�N�g������ꍇ�A������X�V����

	uniformBufferChange = true;
}

glm::vec3 Object::getPosition()
{
	return position;
}

glm::mat4 Object::getTransformMatrix()
{
	return transformMatrix;
}

void Object::sendPosToChildren(glm::vec3 pos)//�q�I�u�W�F�N�g��Ǐ]������
{
	for (auto itr = childObjects.begin(); itr != childObjects.end(); itr++)
	{
		if (*itr)
		{
			(*itr)->setParentPos(pos);
		}
	}

	if (!cameraObj.expired())
	{
		cameraObj.lock()->setParentPos(pos);
	}
}

void Object::setParentPos(glm::vec3 pos)//�e�I�u�W�F�N�g��Ǐ]����
{
	position += pos - parentPos;

	parentPos = pos;
}

void Object::initFrameSetting()//����t���[���̐ݒ���s��
{
	if (lua)
	{
		lua_pushlightuserdata(lua, this);
		lua_setglobal(lua, "Data");

		luaL_dofile(lua, luaPath.c_str());
	}
}