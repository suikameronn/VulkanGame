#include"MaxRectPacking.h"

#include<assert.h>

MaxRectPacking::MaxRectPacking(float width, float height)
{
	binWidth = width;
	binHeight = height;

	Rect n{};
	n.x = 0;
	n.y = 0;
	n.width = width;
	n.height = height;

	usedRects.clear();

	freeRects.clear();
	freeRects.push_back(n);

	newFreeRectLastSize = 0;
}

void MaxRectPacking::placeRect(const Rect& node)
{
	//�󂢂Ă��邷�ׂĂ̗̈�𒲂ׂ�
	for (size_t i = 0; i < freeRects.size();)
	{
		if (splitFreeNode(freeRects[i], node))
		{
			//�����󂫗̈��node�̗̈悪�d�Ȃ�����
			//���̃m�[�h���󂫃��X�g�������
			freeRects[i] = freeRects.back();
			freeRects.pop_back();
		}
		else
		{
			++i;
		}
	}

	pruneFreeList();

	//�g�p�ς݃m�[�h��node��������
	usedRects.push_back(node);
}

Rect MaxRectPacking::findPositionNewBAF(float width, float height
	, float& bestAreaFit, float& bestShortSideFit)
{
	Rect bestNode = {};

	//�ʐςƕӂ̒������ő�l�ŏ���������
	bestAreaFit = FLT_MAX;
	bestShortSideFit = FLT_MAX;

	//���ׂĂ̋󂫗̈��T������
	for (size_t i = 0; i < freeRects.size(); i++)
	{
		//�󂫗̈�ƃr���̖ʐς̍��𒲂ׂ�
		int areaFit = freeRects[i].width * freeRects[i].height - width * height;

		//�܂��̓r������]�������ɔz�u���Ă݂�
		if (freeRects[i].width >= width && freeRects[i].height >= height)
		{
			//�󂫗̈�̕��ƍ������r�������傫���A
			//�󂫗̈�Ƀr�����������Ȃ�

			//�󂫗̈�ƃr���̕��ƍ����̍������߂�
			int leftOverHoriz = abs(freeRects[i].width - width);
			int leftOverVert = abs(freeRects[i].height - height);
			//���ɁA���̍����������ق��𒲂ׂ�
			int shortSideFit = std::min(leftOverHoriz, leftOverVert);

			if (areaFit < bestAreaFit || (areaFit == bestAreaFit && shortSideFit < bestShortSideFit))
			{
				//bestAreaFit�����ʐς̍���������
				//�ӂ̒�����bestShortSideFit����������������
				//�x�X�g�̋󂫗̈�̃f�[�^���X�V����
				bestNode.x = freeRects[i].x;
				bestNode.y = freeRects[i].y;
				bestNode.width = width;
				bestNode.height = height;
				bestShortSideFit = shortSideFit;
				bestAreaFit = areaFit;
				bestNode.rotated = false;
			}
		}

		//���x�̓r������]�����āA�z�u�����Ă݂�
		if (freeRects[i].width >= height && freeRects[i].height >= width)
		{
			//�󂫗̈�̕����m�[�h�̍��������傫��
			//�󂫗̈�̍������m�[�h�̕������傫��������

			//�������A�ʐς�ӂ̒����̍������߁A
			//�ӂ̒����̏������ق������߂�
			int leftOverHoriz = abs(freeRects[i].width - height);
			int leftOverVert = abs(freeRects[i].height - width);
			int shortSideFit = std::min(leftOverHoriz, leftOverVert);

			if (areaFit < bestAreaFit || (areaFit == bestAreaFit && shortSideFit < bestShortSideFit))
			{
				//�x�X�g�ȋ󂫗̈�̃f�[�^���X�V���邪
				//�m�[�h�̕��ƍ������t�ɐݒ肷��
				bestNode.x = freeRects[i].x;
				bestNode.y = freeRects[i].y;
				bestNode.width = height;
				bestNode.height = width;
				bestShortSideFit = shortSideFit;
				bestAreaFit = areaFit;

				bestNode.rotated = true;
			}
		}
	}

	//�ʐςɊ�Â����œK�ȋ󂫗̈��Ԃ�
	return bestNode;
}

//�V�����󂫋�`���󂫃��X�g�ɓ����
void MaxRectPacking::insertNewFreeRect(const Rect& rect)
{
	for (size_t i = 0; i < newFreeRectLastSize;)
	{
		//���̐V������`���A�w��̋󂫋�`�Ɋ��S�܂܂�Ă��Ȃ����ǂ���
		//�܂܂�Ă���Ȃ�AnewFreeRect�����X�g�ɉ����Ȃ��Ă���
		if (rect.isContain(newFreeRects[i]))
		{
			return;
		}

		//���̎��_��newFreeRects[i]��rect�����S�ɕ����Ă͂��Ȃ�
		
		//rect��newFreeRects[i]�����S�ɕ����Ă��邩�ǂ���
		if (newFreeRects[i].isContain(rect))
		{
			//���̎��_�ŁArect��newFreeRects[i]�����S�ɕ����Ă���

			//newFreeRects[i]��rect�ɓ�������

			newFreeRects[i] = newFreeRects[--newFreeRectLastSize];
			newFreeRects[newFreeRectLastSize] = newFreeRects.back();
			newFreeRects.pop_back();
		}
		else
		{
			i++;
		}
	}

	newFreeRects.push_back(rect);
}

//�w�肳�ꂽ�󂫃m�[�h�Ǝg�p�ς݃m�[�h����󂫃m�[�h�𕪊�����
//�������ꂽ�ꍇtrue��Ԃ�
bool MaxRectPacking::splitFreeNode(const Rect& freeNode, const Rect& usedNode)
{
	//���̃m�[�h�Ƌ󂢂Ă���m�[�h���d�Ȃ��Ă��Ȃ��Ȃ�
	//���̋󂫃m�[�h�𒲂ׂ�����
	if (usedNode.x >= freeNode.x + freeNode.width || usedNode.x + usedNode.width <= freeNode.x
		|| usedNode.y >= freeNode.y + freeNode.height || usedNode.y + usedNode.height <= freeNode.y)
	{
		return false;
	}

	//���̎��_��usedNode�͋󂫂̃m�[�h�̈ꕔ�ɏd�Ȃ��Ă���

	//���ꂩ��A�ő��4�̐V�����󂫋�`��V�����󂫋�`���X�g�ɉ�����
	//�����͐V�����ǉ������󂫋�`�́A���݂��ɕ�܂��Ă��邩�𔻒�͂��Ȃ�
	newFreeRectLastSize = newFreeRects.size();

	if (usedNode.x < freeNode.x + freeNode.width && usedNode.x + usedNode.width > freeNode.x)
	{
		//���̃m�[�h�����Ȃ��Ƃ��Ax���ŏd�Ȃ��Ă��镔��������ꍇ

		if (usedNode.y > freeNode.y && usedNode.y < freeNode.y + freeNode.height)
		{
			//�m�[�h�̏㕔���d�Ȃ��Ă���ꍇ

			//�V�����m�[�h�̍������󂫃m�[�h�Ƃ��̃m�[�h�̌��ԕ��ɂ���
			Rect newNode = freeNode;
			newNode.rotated = false;
			newNode.height = usedNode.y - newNode.y;
			insertNewFreeRect(newNode);
		}

		if (usedNode.y + usedNode.height < freeNode.y + freeNode.height)
		{
			//�m�[�h�̉������d�Ȃ��Ă���ꍇ

			//�V�����m�[�h�̍������m�[�h�Ƃ��̃m�[�h�̌��ԕ������ɂ���
			Rect newNode = freeNode;
			newNode.rotated = false;
			newNode.y = usedNode.y + usedNode.height;
			newNode.height = freeNode.y + freeNode.height - (usedNode.y + usedNode.height);
			insertNewFreeRect(newNode);
		}
	}

	if (usedNode.y < freeNode.y + freeNode.height && usedNode.y + usedNode.height > freeNode.y)
	{
		//���̃m�[�h��y���ŏd�Ȃ��Ă���ꍇ

		if (usedNode.x > freeNode.x && usedNode.x < freeNode.x + freeNode.width)
		{
			//�m�[�h�̍������󂫗̈�Əd�Ȃ��Ă���ꍇ

			//�V�����m�[�h�̕����󂫃m�[�h�Ƃ��̃m�[�h�̌��ԕ������ɂ���
			Rect newNode = freeNode;
			newNode.rotated = false;
			newNode.width = usedNode.x - newNode.x;
			insertNewFreeRect(newNode);
		}

		if (usedNode.x + usedNode.width < freeNode.x + freeNode.width)
		{
			//�m�[�h�̉E�����󂫗̈�Əd�Ȃ��Ă��镔��������ꍇ

			//�V�����m�[�h�̕����󂫃m�[�h�Ƃ��̃m�[�h�̌��ԕ������ɂ���
			Rect newNode = freeNode;
			newNode.rotated = false;
			newNode.x = usedNode.x + usedNode.width;
			newNode.width = freeNode.x + freeNode.width - (usedNode.x + usedNode.width);
			insertNewFreeRect(newNode);
		}
	}

	return true;
}

//�󂫋�`���X�g�𒲂ׂāA�璷�ȃm�[�h���폜����
void MaxRectPacking::pruneFreeList()
{
	//�V�����󂫗̈惊�X�g�Â��󂫗̈惊�X�g�ɑ΂��ĕ�܂��e�X�g����
	for (size_t i = 0; i < freeRects.size(); i++)
	{
		for (size_t j = 0; j < newFreeRects.size();)
		{
			if (newFreeRects[j].isContain(freeRects[i]))
			{
				//�V�����ق����Â��ق��Ɋ��S�ɕ����Ă�����
				//�Â��ق��ɓ�������A�V�����ق��̓��X�g�������

				newFreeRects[j] = newFreeRects.back();
				newFreeRects.pop_back();
			}
			else
			{
				//�Â���`���V������`�Ɋ܂܂�邱�Ƃ͂Ȃ�

				j++;
			}
		}
	}

	//�Â��󂫗̈�̖����ɁA�V�����󂫗̈��������
	freeRects.insert(freeRects.end(), newFreeRects.begin(), newFreeRects.end());
	newFreeRects.clear();
}

//�r����}������
Rect MaxRectPacking::insert(bool& packingSuccess,float width, float height)
{
	float maxArea = FLT_MAX;
	float maxLength = FLT_MAX;

	//�œK�ȋ󂫗̈��I��
	Rect newNode = findPositionNewBAF(width, height, maxArea, maxLength);

	if (newNode.height < 1.0)
	{
		std::cerr << "not success" << std::endl;
		packingSuccess = false;
	}

	//�m�[�h��z�u����
	placeRect(newNode);

	//�z�u�����̈��Ԃ�
	return newNode;
}