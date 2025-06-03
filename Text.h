#pragma once

#include"UI.h"
#include"FontManager.h"

#include"utf8/checked.h"

class Text : public UI
{
private:

	//�\�����镶��
	std::string text;
	//�X�y�[�X��������������
	int textLengthNonSpace;

	//�����̃t�H���g�𒣂�t����|���S���̒��_
	//�|���S����ɕ����𒣂�t����
	std::vector<Vertex2D> vertices;

	//�t�H���g���ƂɃ|���S�����ړ�������s���p�ӂ���
	std::vector<glm::mat4> transformMatrices;
	//�t�H���g���ƂɃ|���S�����ړ�������s���p�ӂ���
	std::vector<MappedBuffer> mappedBuffers;

public:

	Text(std::string str);
};