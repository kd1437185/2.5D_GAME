#pragma once

class BackGround : public KdGameObject
{
public:

	BackGround() {}
	~BackGround() override {}

	void DrawUnLit()	override;
	void Init()			override;

private:

	// 板ポリゴン
	KdSquarePolygon m_polygon;

};