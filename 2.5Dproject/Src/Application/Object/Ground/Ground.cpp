#include "Ground.h"

void Ground::DrawLit()
{
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_model, m_mWorld);
}

void Ground::Init()
{
	// メモリ確保
	m_model = std::make_shared<KdModelData>();

	// モデル読み込み
	m_model->Load("Asset/Models/Area/area.gltf");

	// 座標行列
	Math::Matrix transMat = Math::Matrix::CreateTranslation(0, 0, 0);

	// 拡大行列
	Math::Matrix scaleMat = Math::Matrix::CreateScale(3.0f);

	// 行列合成
	m_mWorld = scaleMat * transMat;

	// コライダー(当たり判定情報)の初期化(登録)
	m_pCollider = std::make_unique<KdCollider>();	// 1 生成
	m_pCollider->RegisterCollisionShape				// 2 判定情報を登録
	(
		"Ground",					// 登録名（任意の名前）
		m_model,					// モデルデータ
		KdCollider::TypeGround		// 判定種類
	);

}
