#pragma once

#include<iostream>

#include"Model.h"
#include"Bullet.h"
#include"Player.h"

class ModelFactory
{
private:

	static ModelFactory* instance;

	ModelFactory();
	~ModelFactory();

public:

	static ModelFactory* GetInstance()
	{
		if (!instance)
		{
			instance = new ModelFactory();
		}

		return instance;
	}

	static void FinishInstance()
	{
		delete instance;
	}

	//���f�����쐬
	std::shared_ptr<Model> createModel();
	//�v���C���[�̍쐬
	std::shared_ptr<Player> createPlayer();
};