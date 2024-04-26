#pragma once
#include<iostream>

#include<GLFW/glfw3.h>

class Controller
{
public:
	Controller(GLFWwindow* window);

	virtual void mouseButton(int button, int action, int mods);
	virtual void mousePos(double x, double y);
	virtual void mouseScroll(double x, double y);
	virtual void keyFun(int key, int scancode, int action, int mods);
	virtual void charFun(unsigned int charInfo);

protected:

	static int mouseX, mouseY;
	static bool mouseR, mouseL;
	static int mouseW;
	static unsigned int keyInput;

	void setMouseButton(int button, int action, int mods);
	void setMousePos(double x, double y);
	void setMouseScroll(double x, double y);
	void setKeyFun(int key, int scancode, int action, int mods);
	void setCharFun(unsigned int charInfo);

	static Controller* getThisPtr(GLFWwindow* window)
	{
		return static_cast<Controller*>(glfwGetWindowUserPointer(window));
	}

	static void mouseButtonCB(GLFWwindow* window, int button, int action, int mods)
	{
		getThisPtr(window)->setMouseButton(button, action, mods);
	}

	static void mousePosCB(GLFWwindow* window, double x, double y)
	{
		getThisPtr(window)->setMousePos(x, y);
	}

	static void mouseScrollCB(GLFWwindow* window, double x, double y)
	{
		getThisPtr(window)->setMouseScroll(x, y);
	}

	static void keyFunCB(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		getThisPtr(window)->setKeyFun(key, scancode, action, mods);
	}

	static void charFunCB(GLFWwindow* window, unsigned int charInfo)
	{
		getThisPtr(window)->setCharFun(charInfo);
	}
};