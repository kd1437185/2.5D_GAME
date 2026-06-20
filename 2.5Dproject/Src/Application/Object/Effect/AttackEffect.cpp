#include "AttackEffect.h"

#include "../../Scene/SceneManager.h"
#include "../Enemy/Enemy.h"
#include "../Player/Player.h"
#include "../Torii/Torii.h"

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
	// 一度ヒットしたら以降は判定しない（SPが増えすぎないように）
	//===================================================================
	if (m_isHit) { return; }

	KdCollider::SphereInfo sphere;
	sphere.m_sphere.Center = m_pos;
	sphere.m_sphere.Radius = CollisionRadius;
	sphere.m_type = KdCollider::TypeDamage;

	// ヒットしたかどうか（このフレームで）
	bool hitThisFrame = false;

	// プレイヤーを探すための変数
	std::shared_ptr<Player> player = nullptr;

	for (auto& obj : SceneManager::Instance().GetObjList())
	{
		// 自分自身はスキップ
		if (obj.get() == this) { continue; }

		//===================================================================
		// 敵へのヒット判定
		//===================================================================
		std::shared_ptr<Enemy> enemy = std::dynamic_pointer_cast<Enemy>(obj);
		if (enemy != nullptr)
		{
			std::list<KdCollider::CollisionResult> results;
			if (obj->Intersects(sphere, &results))
			{
				Math::Vector3 knockBackDir = -(enemy->GetDir());
				knockBackDir.Normalize();

				static constexpr float KnockBackPower = 0.3f;
				enemy->TakeDamage(1, knockBackDir * KnockBackPower);

				hitThisFrame = true;

				// プレイヤーのSPを増やす
				for (auto& obj2 : SceneManager::Instance().GetObjList())
				{
					std::shared_ptr<Player> p = std::dynamic_pointer_cast<Player>(obj2);
					if (p != nullptr)
					{
						p->AddSp(10);
						break;
					}
				}
			}
			continue;
		}

		//===================================================================
		// 鳥居へのヒット判定
		//===================================================================
		std::shared_ptr<Torii> torii = std::dynamic_pointer_cast<Torii>(obj);
		if (torii != nullptr)
		{
			std::list<KdCollider::CollisionResult> results;
			if (obj->Intersects(sphere, &results))
			{
				// 鳥居にダメージを与える
				torii->TakeDamage(1);

				hitThisFrame = true;
			}
			continue;
		}
	}

	// このフレームで1体でもヒットしたらヒット済みにする
	if (hitThisFrame)
	{
		m_isHit = true;
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