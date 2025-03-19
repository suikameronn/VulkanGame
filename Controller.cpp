#include"Controller.h"

Controller* Controller::controller = nullptr;

Controller::Controller()
{
	glfwSetWindowUserPointer(window, this);

	glfwSetCursorPosCallback(window, mousePosCB);
	glfwSetScrollCallback(window, mouseScrollCB);

	glfwSetCharCallback(window, charFunCB);

	glfwSetKeyCallback(window, keyInputCB);
}

void Controller::initInput()
{
	keyChar.charInfo = 128;

	keyInput.key = -1;
	keyInput.scancode = -1;
	keyInput.action = 2;
	keyInput.mods = 0;
}

void Controller::mousePos(double x, double y)
{
	mPos.x = x;
	mPos.y = y;
}

void Controller::mouseScroll(double x, double y)
{
	mScroll.y = y;
}

void Controller::charFun(unsigned int charInfo)
{
	keyChar.charInfo = charInfo;
}

void Controller::keyFun(int key, int scancode, int action, int mods)
{
	keyInput.key = key;
	keyInput.scancode = scancode;
	keyInput.action = action;
	keyInput.mods = mods;
}

void Controller::setMousePos(double x, double y)
{
	mousePos(x, y);
}

void Controller::setMouseScroll(double x, double y)
{
	mouseScroll(x, y);
}

void Controller::setCharFun(unsigned int charInfo)
{
	charFun(charInfo);
}

void Controller::setKeyFun(int key, int scancode, int action, int mods)
{
	keyFun(key, scancode, action, mods);
}

bool Controller::getKey(int key)
{
	return glfwGetKey(window,key);
}