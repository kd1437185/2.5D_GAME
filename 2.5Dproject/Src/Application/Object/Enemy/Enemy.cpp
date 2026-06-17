#include "Enemy.h"

#include "../../Scene/SceneManager.h"
#include "../Player/Player.h"

void Enemy::Update()
{
	//===================================================================
	// ホーミング処理
	//===================================================================
	Math::Vector3 playerPos = {};

	if (SearchPlayer(playerPos))
	{
		// 目的地 - 現在地 で方向ベクトルを求める
		Math::Vector3 enemyMove = playerPos - m_pos;

		//===================================================================
		// 向きの判定
		// しきい値より X の差が大きい場合だけ反転フラグを更新する
		// しきい値以内のときは前回の向きを維持して連続反転を防ぐ
		//===================================================================
		static const float flipThreshold = 0.5f;
		if (enemyMove.x > flipThreshold)
		{
			// プレイヤーが右側にいる → 左に向かって移動 → そのまま描画
			m_isFlip = false;
		}
		else if (enemyMove.x < -flipThreshold)
		{
			// プレイヤーが左側にいる → 右に向かって移動 → X反転
			m_isFlip = true;
		}
		// しきい値以内は m_isFlip を変えない（連続反転防止）

		// 正規化（長さを1にして方向だけ残す）
		enemyMove.Normalize();

		// スピードをかけて移動
		m_pos += enemyMove * m_speed;
	}

	//===================================================================
	// アニメーション更新
	//===================================================================
	m_animeCnt += m_animeSpeed;

	if (m_animeCnt >= (float)m_animTextures.size())
	{
		m_animeCnt = 0;
	}
}

void Enemy::Init()
{
	m_polygon = std::make_shared<KdSquarePolygon>();

	//===================================================================
	// アニメーション用テクスチャを事前にロードしておく
	// KdTexture のコンストラクタにファイル名を渡すだけでロードできる
	//===================================================================
	m_animTextures =
	{
		std::make_shared<KdTexture>("Asset/Textures/Enemy/MobEnemy/character-run-0.png"),
		std::make_shared<KdTexture>("Asset/Textures/Enemy/MobEnemy/character-run-1.png"),
		std::make_shared<KdTexture>("Asset/Textures/Enemy/MobEnemy/character-run-2.png"),
		std::make_shared<KdTexture>("Asset/Textures/Enemy/MobEnemy/character-run-3.png"),
	};

	// 最初のテクスチャをセット
	m_polygon->SetMaterial(m_animTextures[0]);

	// 板ポリの基準点（真ん中下段を指定）
	m_polygon->SetPivot(KdSquarePolygon::PivotType::Center_Bottom);

	// 1枚画像なので分割なし
	m_polygon->SetSplit(1, 1);

	// アニメーション初期値
	m_animeCnt = 0;
	m_animeSpeed = 0.1f;

	// 座標・方向・移動速度
	m_pos = {};
	m_dir = {};
	m_speed = 0.03f;

	// 行列
	m_mWorld = Math::Matrix::Identity;

	// デバッグワイヤー生成
	m_pDebugWire = std::make_unique<KdDebugWireFrame>();

	// 当たり判定の登録
	m_pCollider = std::make_unique<KdCollider>();

	// TypeBump：敵同士が重ならないための判定
	m_pCollider->RegisterCollisionShape(
		"BumpCollision",
		Math::Vector3(0.0f, 0.5f, 0.0f),
		0.5f,
		KdCollider::TypeBump
	);

	// TypeDamage：攻撃を受ける側の判定
	// AttackEffect のスフィアと当たったら SetExpired() が呼ばれる
	m_pCollider->RegisterCollisionShape(
		"DamageCollision",
		Math::Vector3(0.0f, 0.5f, 0.0f),
		0.5f,
		KdCollider::TypeDamage
	);

}

void Enemy::PostUpdate()
{
	//===================================================================
	// 敵同士の押し戻し処理
	// オブジェクトリストから他の Enemy を探して重なりを解消する
	//===================================================================
	for (auto& obj : SceneManager::Instance().GetObjList())
	{
		// 自分自身はスキップ
		if (obj.get() == this) { continue; }

		// Enemy かどうか確認
		std::shared_ptr<Enemy> otherEnemy = std::dynamic_pointer_cast<Enemy>(obj);
		if (otherEnemy == nullptr) { continue; }

		// 相手の Enemy に当たり判定があるか確認
		if (!otherEnemy->m_pCollider) { continue; }

		// スフィア判定で押し戻しを行う
		// 自分のスフィアを作成
		KdCollider::SphereInfo sphere;
		sphere.m_sphere.Center = m_pos;
		sphere.m_sphere.Center.y += 0.5f;	// 中心を少し上に
		sphere.m_sphere.Radius = CollisionRadius;
		sphere.m_type = KdCollider::TypeBump;

		// 相手の Enemy のワールド行列で判定
		std::list<KdCollider::CollisionResult> results;
		if (otherEnemy->Intersects(sphere, &results))
		{
			// 押し戻し処理
			for (auto& result : results)
			{
				Math::Vector3 pushDir = result.m_hitDir;
				pushDir.y = 0.0f;			// Y方向の押し戻しは無効
				pushDir.Normalize();
				m_pos += pushDir * result.m_overlapDistance;
			}
		}
	}

	// Update()で確定したパラメーターから座標行列を作成
	m_mWorld = Math::Matrix::CreateTranslation(m_pos);
}

void Enemy::GenerateDepthMapFromLight()
{
	m_polygon->SetMaterial(m_animTextures[(int)m_animeCnt]);

	if (m_isFlip)
	{
		// X座標だけ入れ替えて左右反転
		m_polygon->SetUVRect(
			Math::Vector2(1.0f, 0.0f),	// uvMin
			Math::Vector2(0.0f, 1.0f)	// uvMax
		);
	}
	else
	{
		// そのまま描画
		m_polygon->SetUVRect(
			Math::Vector2(0.0f, 0.0f),	// uvMin
			Math::Vector2(1.0f, 1.0f)	// uvMax
		);
	}

	KdShaderManager::Instance().m_StandardShader.DrawPolygon(*m_polygon, m_mWorld);
}

void Enemy::DrawLit()
{
	m_polygon->SetMaterial(m_animTextures[(int)m_animeCnt]);
	if (m_isFlip)
	{
		// X座標だけ入れ替えて左右反転
		m_polygon->SetUVRect(
			Math::Vector2(1.0f, 0.0f),	// uvMin
			Math::Vector2(0.0f, 1.0f)	// uvMax
		);
	}
	else
	{
		// そのまま描画
		m_polygon->SetUVRect(
			Math::Vector2(0.0f, 0.0f),	// uvMin
			Math::Vector2(1.0f, 1.0f)	// uvMax
		);
	}

	KdShaderManager::Instance().m_StandardShader.DrawPolygon(*m_polygon, m_mWorld);
}


// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// プレイヤーを探して座標を取得する
// ・オブジェクトリストを走査して Player を探す
// ・見つかったら座標を _outPlayerPos に格納して true を返す
// ・見つからなかったら false を返す
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
bool Enemy::SearchPlayer(Math::Vector3& _outPlayerPos)
{
	for (auto& obj : SceneManager::Instance().GetObjList())
	{
		std::shared_ptr<Player> player = std::dynamic_pointer_cast<Player>(obj);

		if (player == nullptr) { continue; }

		_outPlayerPos = player->GetPos();

		return true;
	}

	return false;
}