#include"InputSystem.h"

InputSystem::InputSystem()
{
	glfwSetWindowUserPointer(window, this);
}

int InputSystem::getKey(int key)
{
	return glfwGetKey(window,key);
}

void InputSystem::getMousePos(double& x, double& y)
{
	glfwGetCursorPos(window, &x, &y);
}

int InputSystem::getMouseButton(int mouseButton)
{
	return glfwGetMouseButton(window, mouseButton);
}