#pragma once

//�R���C�_�[�̌`��
enum COLIDER
{
	BOX
};

//�I�u�W�F�N�g�̃^�C�v������
enum ObjNum
{
	OBJECT = 0,
	MODEL = 1,
	PLAYER,
	BULLET,
	POINTLIGHT = 101,
	DIRECTIONALLIGHT
};

enum UINum
{
	UI = 0,
	TEXT
};

#define TYPEOBJECT 0
#define TYPEMODEL 1
#define TYPELIGHT 101

//�I�u�W�F�N�g�̐���������
enum Tag
{
	NONE = 0,
	GROUND
};