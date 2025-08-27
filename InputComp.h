#pragma once

#include"InputSystem.h"

struct Input
{
	KeyInput keyInput;
	MPos mPos;
	MScroll mScroll;
	KeyChar keyChar;

	void init()
	{
		keyInput.key.clear();
		keyInput.action.clear();
		keyInput.mods.clear();
		keyInput.scancode.clear();
	}

	void copy(const Input& input)
	{
		keyInput = input.keyInput;
		mPos = input.mPos;
		mScroll = input.mScroll;
		keyChar = input.keyChar;
	}
};

struct InputComp
{
	Input lastFrame;
	Input currentFrame;
};