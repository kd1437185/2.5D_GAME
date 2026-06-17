#include "BackGround.h"

void BackGround::DrawUnLit()
{
	// 背景描画
	// 背景に陰影がつくのはおかしいので DrawUnLit() で描画する
	KdShaderManager::Instance().m_StandardShader.DrawPolygon(m_polygon, m_mWorld);
}

void BackGround::Init()
{
	m_polygon.SetMaterial("Asset/Textures/BackGround.png");

	// 板ポリのサイズを拡大
	m_polygon.SetScale(200.0f);

	// 板ポリの原点 (真ん中下段を指定)
	m_polygon.SetPivot(KdSquarePolygon::PivotType::Center_Bottom);

	m_mWorld = Math::Matrix::CreateTranslation
	(Math::Vector3(0.0f, 0.0f, 100.0f));
}
