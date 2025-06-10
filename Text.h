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

	//������̃t�H���g�̃f�[�^
	std::vector<CharFont> charFonts;

	//utf8�̕������󔒂������łȂ���
	//�󔒂̏ꍇtrue
	bool isSpaceCharacter(const char32_t code);
	
	//���s�L�����ǂ���
	bool isStartNewLine(const char32_t code);

	//�����񂩂�K�v�ȃt�H���g���擾����
	void loadFont();

	//�����񂩂璸�_�f�[�^��ݒ肷��
	void setVertIndices(std::vector<int>& startNewLinePoint);

	void cleanupVulkan() override;

public:

	Text(std::string str);

	void initFrameSettings() override;

	VkDescriptorSet& getImageDescriptorSet() override { return FontManager::GetInstance()->getDescriptorSet(); }
};