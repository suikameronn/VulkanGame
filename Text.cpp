#include"GameManager.h"

#include"Text.h"

Text::Text(std::string text)
{
	uiNum = UINum::TEXT;

	exist = true;

	GameManager* manager = GameManager::GetInstance();

	projMatrix = manager->getUIProjection();

	isUpdateTransformMatrix = true;

	position = glm::vec2(0.0f, 0.0f);

	rotate.z = 0.0f;

	scale = 1.0f;

	isVisible = true;
	transparent = true;

	//�����񂩂�X�y�[�X��������������̒����𐔂���
	textLengthNonSpace = 0;
	const char* begin = text.c_str();
	const char* end = begin + strlen(text.c_str());

	//���s�̈ʒu������
	std::vector<int> startNewLinePoint;

	int index = 0;
	while (begin < end)
	{
		uint32_t code = utf8::next(begin, end);

		if (isStartNewLine(code))
		{
			//���s�̈ʒu���L�^����
			startNewLinePoint.push_back(index);
			index++;
			continue;
		}

		if (!isSpaceCharacter(code))
		{
			//���̕������󔒂ł͂Ȃ��ꍇ
			textLengthNonSpace++;
		}

		//utf8�̃R�[�h���L�^����
		utf8Codes.push_back(code);

		index++;
	}

	//�����񂩂�K�v�ȃt�H���g�̃f�[�^���擾����
	loadFont();

	//�����񂩂璸�_�f�[�^��ݒ肷��
	setVertIndices(startNewLinePoint);

	//UI��\�����邽�߂̍s����L�^���邽�߂̃o�b�t�@���쐬
	VulkanBase::GetInstance()->uiCreateUniformBuffer(mappedBuffer);

	setPosition(glm::vec2(manager->getWindowWidth() / 2.0f, manager->getWindowHeight() / 2.0f));
}

//utf8�̕������󔒂������łȂ���
//�󔒂̏ꍇtrue
bool Text::isSpaceCharacter(const char32_t code)
{
	return code == U'\u0020' || code == U'\u00A0'
		|| code == U'\u3000';
}

//���s�̋L�����ǂ���
bool Text::isStartNewLine(const char32_t code)
{
	return code == U'\u000D' || code == U'\u000A';
}

//�����񂩂�K�v�ȃt�H���g�̃f�[�^���擾����
void Text::loadFont()
{
	FontManager* fontManager = FontManager::GetInstance();

	fontManager->getCharFont(utf8Codes, charFonts);

	//�����t�H���g�f�[�^���當����������_�����O�����Ƃ��̑傫�����擾����
	uiWidth = 0;
	uiHeight = fontManager->getFontHeight();
	for (const CharFont& charFont : charFonts)
	{
		uiWidth += charFont.advance;
	}
}

//�����񂩂璸�_�f�[�^��ݒ肷��
void Text::setVertIndices(std::vector<int>& startNewLinePoint)
{
	//�X�y�[�X���������������̐������A�|���S�����쐬����
	vertices.resize(textLengthNonSpace * UIVertexCount);
	indices.resize(textLengthNonSpace * UIIndexCount);

	//indices = { 0, 1, 2, // �ŏ��̎O�p�`
	//			2, 3, 1 };  // 2�Ԗڂ̎O�p�`

	//�������̃|���S���ɏ�̃C���f�b�N�X��
	//�������ɍ��킹�Ē����������̂�ݒ肷��
	for (int i = 0; i < textLengthNonSpace; i++)
	{
		indices[i * UIIndexCount] = 0 + (i * UIVertexCount);
		indices[i * UIIndexCount + 1] = 1 + (i * UIVertexCount);
		indices[i * UIIndexCount + 2] = 2 + (i * UIVertexCount);
		indices[i * UIIndexCount + 3] = 2 + (i * UIVertexCount);
		indices[i * UIIndexCount + 4] = 3 + (i * UIVertexCount);
		indices[i * UIIndexCount + 5] = 1 + (i * UIVertexCount);
	}

	//�t�H���g���璸�_�̍��W��ݒ肷��
	glm::vec2 penPos = glm::vec2(0.0f);
	for (int i = 0; i < charFonts.size(); i++)
	{
		if (charFonts[i].size.x == 0.0f)
		{
			//�����X�y�[�X�̏ꍇ�̓y���̈ʒu�������炷
			penPos.x += charFonts[i].advance;
			continue;
		}

		for (int index : startNewLinePoint)
		{
			if (i == index)
			{
				penPos.x = 0;
				penPos.y += charFonts[i].fontHeight;
			}
		}

		//���_���W�̐ݒ�
		vertices[i * UIVertexCount + 0].pos = glm::vec2(charFonts[i].bearing.x, charFonts[i].bearing.y) + penPos;

		vertices[i * UIVertexCount + 1].pos = glm::vec2(charFonts[i].bearing.x + charFonts[i].size.x
			, charFonts[i].bearing.y) + penPos;

		vertices[i * UIVertexCount + 2].pos = glm::vec2(charFonts[i].bearing.x
			, charFonts[i].bearing.y + charFonts[i].size.y) + penPos;

		vertices[i * UIVertexCount + 3].pos = glm::vec2(charFonts[i].bearing.x + charFonts[i].size.x
			, charFonts[i].bearing.y + charFonts[i].size.y) + penPos;

		penPos.x += charFonts[i].advance;

		//uv���W�̐ݒ�
		if (charFonts[i].rect.rotated)
		{
			//�r�b�g�}�b�v����]�������Ă���ꍇ
			//uv�����炷

			vertices[i * UIVertexCount + 0].uv = glm::vec2(charFonts[i].uvMin.x, charFonts[i].uvMax.y);

			vertices[i * UIVertexCount + 1].uv = glm::vec2(charFonts[i].uvMin.x, charFonts[i].uvMin.y);

			vertices[i * UIVertexCount + 2].uv = glm::vec2(charFonts[i].uvMax.x, charFonts[i].uvMax.y);

			vertices[i * UIVertexCount + 3].uv = glm::vec2(charFonts[i].uvMax.x, charFonts[i].uvMin.y);
		}
		else
		{
			//��]���Ă��Ȃ��ꍇ�́A���̂܂�uv��ݒ肷��
			vertices[i * UIVertexCount + 0].uv = glm::vec2(charFonts[i].uvMin.x, charFonts[i].uvMin.y);

			vertices[i * UIVertexCount + 1].uv = glm::vec2(charFonts[i].uvMax.x, charFonts[i].uvMin.y);

			vertices[i * UIVertexCount + 2].uv = glm::vec2(charFonts[i].uvMin.x, charFonts[i].uvMax.y);

			vertices[i * UIVertexCount + 3].uv = glm::vec2(charFonts[i].uvMax.x, charFonts[i].uvMax.y);
		}
	}
}

//���W�ϊ��s���ݒ�
void Text::initFrameSettings()
{
	updateTransformMatrix();
}

//gpu��̃o�b�t�@��j��
void Text::cleanupVulkan()
{
	VulkanBase* vulkan = VulkanBase::GetInstance();

	for (BufferObject& pointBuffer : pointBuffers)
	{
		vulkan->addDefferedDestructBuffer(pointBuffer);
	}
	vulkan->addDefferedDestructBuffer(mappedBuffer);
}