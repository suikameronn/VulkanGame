#pragma once
#include<iostream>

#include<GLFW/glfw3.h>

struct MButtonStat
{
	int button;
	int clicked;
	int mods;
};

struct MPos
{
	double x;
	double y;
};

struct MScroll
{
	double y;
};

struct KeyChar
{
	unsigned int charInfo;
};

static int mouseX, mouseY;
static bool mouseR, mouseL;
static int mouseW;
static unsigned int keyInput;

static MButtonStat mButtonStat;
static MPos mPos;
static MScroll mScroll;
static KeyChar keyChar;

class Controller
{
public:
	Controller(GLFWwindow* window);

	virtual void mouseButton(int button, int action, int mods);
	virtual void mousePos(double x, double y);
	virtual void mouseScroll(double x, double y);
	virtual void charFun(unsigned int charInfo);

	static MButtonStat* getMButtonStat() { return &mButtonStat; }
	static MPos* getMPos() { return &mPos; }
	static KeyChar* getKeyChar() { return &keyChar; }

protected:

	void setMouseButton(int button, int action, int mods);
	void setMousePos(double x, double y);
	void setMouseScroll(double x, double y);
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

	static void charFunCB(GLFWwindow* window, unsigned int charInfo)
	{
		getThisPtr(window)->setCharFun(charInfo);
	}
};