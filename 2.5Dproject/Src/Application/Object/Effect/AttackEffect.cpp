#include "AttackEffect.h"

#include "../../Scene/SceneManager.h"
#include "../Enemy/Enemy.h"

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 初期化
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void AttackEffect::Init()
{
	m_polygon = std::make_shared<KdSquarePolygon>();

	// スプライトシートを読み込む
	m_polygon->SetMaterial("Asset/Textures/Effect/sprite-sheet.png");

	// 板ポリの基準点（中央）
	m_polygon->SetPivot(KdSquarePolygon::PivotType::Center_Middle);

	// 横5・縦2に分割
	m_polygon->SetSplit(5, 2);

	// 最初のコマをセット
	m_polygon->SetUVRect(0);

	// エフェクトのサイズ
	m_polygon->SetScale(2.0f);

	// アニメーション初期値
	m_animeCnt = 0.0f;
	m_animeSpeed = 0.3f;

	//===================================================================
	// 当たり判定の登録
	// TypeDamage：攻撃判定（敵の TypeDamage 判定に当たったら敵を消滅させる）
	//===================================================================
	m_pCollider = std::make_unique<KdCollider>();
	m_pCollider->RegisterCollisionShape(
		"AttackCollision",
		Math::Vector3(0.0f, 0.0f, 0.0f),	// 中心
		CollisionRadius,					// 半径
		KdCollider::TypeDamage
	);
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 更新
// ・アニメーションを進める
// ・敵との当たり判定を行う
// ・全コマ再生したら消滅する
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void AttackEffect::Update()
{
	// アニメーションを進める
	m_animeCnt += m_animeSpeed;

	// 全コマ再生したら消滅
	if ((int)m_animeCnt >= FrameCount)
	{
		m_isExpired = true;
		return;
	}

	// 現在のコマをセット
	m_polygon->SetUVRect((int)m_animeCnt);

	//===================================================================
	// 敵との当たり判定
	// TypeDamage のスフィアで全オブジェクトを走査して
	// Enemy に当たったら即座に消滅させる
	//===================================================================
	KdCollider::SphereInfo sphere;
	sphere.m_sphere.Center = m_pos;
	sphere.m_sphere.Radius = CollisionRadius;
	sphere.m_type = KdCollider::TypeDamage;

	for (auto& obj : SceneManager::Instance().GetObjList())
	{
		// 自分自身はスキップ
		if (obj.get() == this) { continue; }

		// Enemy かどうか確認する
		// dynamic_pointer_cast で Enemy 以外は nullptr が返る
		std::shared_ptr<Enemy> enemy = std::dynamic_pointer_cast<Enemy>(obj);
		if (enemy == nullptr) { continue; }

		// 当たり判定を実行
		std::list<KdCollider::CollisionResult> results;
		if (obj->Intersects(sphere, &results))
		{
			// ノックバック方向：敵の進行方向の逆
			// enemy->m_dir の逆方向にノックバックさせる
			// m_dir は private なので GetDir() を追加するか
			// エフェクトからプレイヤー→敵の方向で計算する
			Math::Vector3 knockBackDir = -(enemy->GetDir());
			knockBackDir.Normalize();

			// ノックバックの強さ
			static constexpr float KnockBackPower = 0.3f;

			// ダメージとノックバックを与える
			enemy->TakeDamage(1, knockBackDir * KnockBackPower);
		}
	}
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// PostUpdate
// ・ワールド行列の更新
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void AttackEffect::PostUpdate()
{
	m_mWorld = Math::Matrix::CreateTranslation(m_pos);
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 描画
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void AttackEffect::DrawBright()
{
	if (m_isFlip)
	{
		// 現在のコマのUV座標を取得してX軸を反転
		int frame = (int)m_animeCnt;
		int x = frame % 5;
		int y = frame / 5;

		float w = 1.0f / 5;
		float h = 1.0f / 2;

		m_polygon->SetUVRect(
			Math::Vector2((x + 1) * w, y * h),
			Math::Vector2(x * w, (y + 1) * h)
		);
	}

	KdShaderManager::Instance().m_StandardShader.DrawPolygon(*m_polygon, m_mWorld);
}