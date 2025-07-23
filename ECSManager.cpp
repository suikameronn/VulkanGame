#include"ECSManager.h"

// スタティック変数の初期化
size_t ECSManager::nextCcompTypeID = 0;
template<typename CompType>
size_t ECSManager::ComponentPool<CompType>::compTypeID = 0;