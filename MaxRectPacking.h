#pragma once

#include<iostream>
#include<vector>

struct Rect
{
	float x;
	float y;
	float width;
	float height;

	//��]���������ǂ���
	bool rotated;

	Rect()
	{
		x = 0.0f;
		y = 0.0f;
		width = 0.0f;
		height = 0.0f;

		rotated = false;
	}

	//������rect������rect�����S�Ɋ܂�ł��邩�ǂ���
	bool isContain(const Rect& rect) const
	{
		return x >= rect.x && y >= rect.y
			&& x + width <= rect.x + rect.width
			&& y + height <= rect.y + rect.height;
	}
};

class MaxRectPacking
{
private:

	//�r���̕��ƍ���
	float binWidth;
	float binHeight;

	//�ŐV�̐V�����󂫗̈�̐�
	size_t newFreeRectLastSize;
	//�ŐV�̋󂫗̈�
	std::vector<Rect> newFreeRects;

	//�g�p�ς݂̗̈�
	std::vector<Rect> usedRects;
	//�ŐV�ł͂Ȃ��󂫗̈�
	std::vector<Rect> freeRects;

	//Rect���r���ɔz�u����
	void placeRect(const Rect& node);

	//�ʐς��l���������݂̋󂫗̈�̒��ōœK�ȗ̈��I������
	Rect findPositionNewBAF(float width, float height
		, float& bestAreaFit, float& bestShortSideFit);

	//�V�����󂫋�`���󂫃��X�g�ɓ����
	void insertNewFreeRect(const Rect& rect);

	//�w�肳�ꂽ�󂫃m�[�h�Ǝg�p�ς݃m�[�h����󂫃m�[�h�𕪊�����
	//�������ꂽ�ꍇtrue��Ԃ�
	bool splitFreeNode(const Rect& freeNode, const Rect& usedNode);

	//�󂫋�`���X�g�𒲂ׂāA�璷�ȃm�[�h���폜����
	void pruneFreeList();

	MaxRectPacking() {};

public:

	MaxRectPacking(float width, float height);

	~MaxRectPacking() {};

	//�r����}������
	Rect insert(bool& packingSuccess,float width, float height);
};