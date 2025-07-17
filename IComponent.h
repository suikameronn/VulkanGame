#pragma once

#include<memory>

class Entity;

class IComponent
{
private:

	std::weak_ptr<Entity> entity;

public:

	IComponent() {};
	~IComponent() {};

	//コンポーネントをアタッチした時点で実行
	virtual void OnAwake() = 0;

	//アタッチ後の次のフレーム開始時点で実行
	virtual void OnStart() = 0;

	//更新フェーズで実行
	virtual void OnUpdate() = 0;

	//更新フェーズ後に実行
	virtual void OnLateUpdate() = 0;

	//フレーム終了時に実行
	virtual void OnFrameEnd() = 0;

	//コンポーネントの所有者を設定する
	void setOwner(std::weak_ptr<Entity> e)
	{
		entity = e;

		OnAwake();
	}

	//コンポーネントの所有者を取得する
	std::weak_ptr<Entity> getOwner()
	{
		return entity;
	}
};