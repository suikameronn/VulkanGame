#include"ECSManager.h"

// �X�^�e�B�b�N�ϐ��̏�����
size_t ECSManager::nextCcompTypeID = 0;
template<typename CompType>
size_t ECSManager::ComponentPool<CompType>::compTypeID = 0;