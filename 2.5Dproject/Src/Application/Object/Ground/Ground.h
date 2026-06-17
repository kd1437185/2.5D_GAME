#pragma once

class Ground : public KdGameObject
{
public:

	Ground() {}
	~Ground() override {}

	void DrawLit()	override;
	void Init()		override;

private:

	// モデル
	std::shared_ptr<KdModelData> m_model;

};