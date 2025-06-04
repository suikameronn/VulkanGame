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

	//�����񂩂�X�y�[�X��������������̒����𐔂���
	textLengthNonSpace = 0;
	const char* begin = text.c_str();
	const char* end = begin + strlen(text.c_str());

	const std::u8string space = u8" ";

	while (begin < end)
	{
		uint32_t code = utf8::next(begin, end);

		if (!isSpaceCharacter(code))
		{
			//���̕������󔒂ł͂Ȃ��ꍇ
			textLengthNonSpace++;

			//utf8�̃R�[�h���L�^����
			utf8Codes.push_back(code);
		}
	}

	//�X�y�[�X���������������̐������A�|���S�����쐬����
	vertices.resize(UIVertexCount * textLengthNonSpace);
	mappedBuffers.resize(textLengthNonSpace);
	descriptorSets.resize(textLengthNonSpace);

	indices = { 0, 1, 2, // �ŏ��̎O�p�`
				2, 3, 1 };  // 2�Ԗڂ̎O�p�`
}

//utf8�̕������󔒂������łȂ���
//�󔒂̏ꍇtrue
bool Text::isSpaceCharacter(const char32_t code)
{
	return code == U'\u0020' || code == U'\u00A0'
		|| code == U'\u3000';
}