#pragma once
#include<iostream>
#include<GLFW/glfw3.h>

extern GLFWwindow* window;

enum
{
	a = 97,
	b,
	c,
	d,
	e,
	f,
	g,
	h,
	i,
	j,
	k,
	l,
	m,
	n,
	o,
	p,
	q,
	r,
	s,
	t,
	u,
	v,
	w,
	x,
	y,
	z
};

struct KeyInput
{
	int key;
	int scancode;
	int action;
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

class Controller
{
private:
	static Controller* controller;

	Controller();
	~Controller() {};

public:

	KeyInput keyInput;
	MPos mPos;
	MScroll mScroll;
	KeyChar keyChar;
	
	static Controller* GetInstance()
	{
		if (!controller)
		{
			controller = new Controller();
		}

		return controller;
	}

	void FinishController()
	{
		delete controller;
		controller = nullptr;
	}

	void initInput();

	virtual void mousePos(double x, double y);
	virtual void mouseScroll(double x, double y);
	virtual void charFun(unsigned int charInfo);
	virtual void keyFun(int key, int scancode, int action, int mods);

	bool getKey(int key);

protected:

	void setMousePos(double x, double y);
	void setMouseScroll(double x, double y);
	void setCharFun(unsigned int charInfo);
	void setKeyFun(int key, int scancode, int action, int mods);

	static Controller* getThisPtr(GLFWwindow* window)
	{
		return static_cast<Controller*>(glfwGetWindowUserPointer(window));
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

	static void keyInputCB(GLFWwindow* const window, int key, int scancode,int action, int mods)
	{
		getThisPtr(window)->setKeyFun(key, scancode, action, mods);
	}
};