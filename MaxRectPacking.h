#pragma once

#include<iostream>
#include<vector>

struct Rect
{
	int x;
	int y;
	int width;
	int height;

	//��]���������ǂ���
	bool rotated = false;

	//������Rect������Rect�����S�Ɋ܂�ł��邩�ǂ���
	//����Rect��������Rect�̈ꕔ�ł��܂�ł��Ȃ������������
	//false��Ԃ�
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
	int binWidth;
	int binHeight;

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
	Rect findPositionNewBAF(int width, int height
		, int& bestAreaFit, int& bestShortSideFit);

	//�V�����󂫋�`���󂫃��X�g�ɓ����
	void insertNewFreeRect(const Rect& rect);

	//�w�肳�ꂽ�󂫃m�[�h�Ǝg�p�ς݃m�[�h����󂫃m�[�h�𕪊�����
	//�������ꂽ�ꍇtrue��Ԃ�
	bool splitFreeNode(const Rect& freeNode, const Rect& usedNode);

	//�󂫋�`���X�g�𒲂ׂāA�璷�ȃm�[�h���폜����
	void pruneFreeList();

	MaxRectPacking() {};

public:

	MaxRectPacking(int width, int height);

	~MaxRectPacking() {};

	//�r����}������
	Rect& insert(bool& packingSuccess,int width, int height);
};