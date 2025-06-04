#pragma once

#include"UI.h"
#include"FontManager.h"

#include"utf8/checked.h"

class Text : public UI
{
private:

	//�e�L�X�g��\������ۂ̃|���S���̃C���f�b�N�X���L�^���Ă���
	//BufferObject�̔z��̓Y���������߂�
	const int indexBufferNum = 0;

	//�\�����镶��
	std::string text;
	//utf8�̕����R�[�h
	std::vector<uint32_t> utf8Codes;
	//�X�y�[�X��������������
	int textLengthNonSpace;

	//�����̃t�H���g�𒣂�t����|���S���̒��_
	//�|���S����ɕ����𒣂�t����
	std::vector<Vertex2D> vertices;
	std::vector<BufferObject> pointBuffers;

	//�������ƂɃ|���S�����ړ�������s���p�ӂ���
	std::vector<glm::mat4> transformMatrices;
	//�������ƂɃ|���S�����ړ�������s���p�ӂ���
	std::vector<MappedBuffer> mappedBuffers;
	//�������Ƃ̃|���S���p��VkDescriptorSet
	std::vector<VkDescriptorSet> descriptorSets;

	//utf8�̕������󔒂������łȂ���
	//�󔒂̏ꍇtrue
	bool isSpaceCharacter(const char32_t code);

public:

	Text(std::string str);

	//���_�z���Ԃ�
	const std::vector<Vertex2D>& getVertices() { return vertices; }
	//���_�p�̃o�b�t�@�̔z���Ԃ�
	std::vector<BufferObject> getPointBuffer() { return pointBuffers; }
	//�s��L�^�p�̃o�b�t�@�̔z���Ԃ�
	std::vector<MappedBuffer>& getMappedBuffer() { return mappedBuffers; }
	//VkDescriptorSet��Ԃ�
	std::vector<VkDescriptorSet>& getDescriptorSet() { return descriptorSets; }

	//�e�L�X�g��`�悷��ۂ̃|���S���̃C���f�b�N�X���L�^����Ă���
	//BufferObject�̔z��̓Y������Ԃ�
	const int getIndexBufferNum() { return indexBufferNum; }
};