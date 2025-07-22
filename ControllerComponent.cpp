#include"ControllerComponent.h"

ControllerComponent::ControllerComponent()
{
	glfwSetWindowUserPointer(window, this);

	glfwSetCursorPosCallback(window, mousePosCB);
	glfwSetScrollCallback(window, mouseScrollCB);

	glfwSetCharCallback(window, charFunCB);

	glfwSetKeyCallback(window, keyInputCB);
}

void ControllerComponent::initInput()
{
	keyChar.charInfo = 128;

	keyInput.key = -1;
	keyInput.scancode = -1;
	keyInput.action = 2;
	keyInput.mods = 0;
}

void ControllerComponent::mousePos(double x, double y)
{
	mPos.x = x;
	mPos.y = y;
}

void ControllerComponent::mouseScroll(double x, double y)
{
	mScroll.y = y;
}

void ControllerComponent::charFun(unsigned int charInfo)
{
	keyChar.charInfo = charInfo;
}

void ControllerComponent::keyFun(int key, int scancode, int action, int mods)
{
	keyInput.key = key;
	keyInput.scancode = scancode;
	keyInput.action = action;
	keyInput.mods = mods;
}

void ControllerComponent::setMousePos(double x, double y)
{
	mousePos(x, y);
}

void ControllerComponent::setMouseScroll(double x, double y)
{
	mouseScroll(x, y);
}

void ControllerComponent::setCharFun(unsigned int charInfo)
{
	charFun(charInfo);
}

void ControllerComponent::setKeyFun(int key, int scancode, int action, int mods)
{
	keyFun(key, scancode, action, mods);
}

bool ControllerComponent::getKey(int key)
{
	return glfwGetKey(window,key);
}

void ControllerComponent::getMousePos(double& x, double& y)
{
	glfwGetCursorPos(window, &x, &y);
}

bool ControllerComponent::getMouseButton(int mouseButton)
{
	return glfwGetMouseButton(window, mouseButton);
}