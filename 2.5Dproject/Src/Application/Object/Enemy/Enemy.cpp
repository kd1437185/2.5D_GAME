#include "Enemy.h"

#include "../../Scene/SceneManager.h"
#include "../Player/Player.h"

void Enemy::Update()
{
	//===================================================================
	// 死亡演出中の処理
	//===================================================================
	if (m_deathState == DeathState::Dissolve)
	{
		m_dissolve += DissolveSpeed;
		if (m_dissolve >= 1.0f)
		{
			m_isExpired = true;
		}
		return;
	}

	//===================================================================
	// 無敵タイマーの更新
	//===================================================================
	if (m_invincibleTimer > 0)
	{
		m_invincibleTimer--;
	}

	//===================================================================
	// ノックバック処理
	//===================================================================
	if (m_knockBackVec.Length() > KnockBackThreshold)
	{
		// ノックバック中は攻撃をキャンセルする
		if (m_isAttacking)
		{
			m_isAttacking = false;
			m_attackAnimeCnt = 0.0f;
			m_isDamageGiven = false;

			// 通常テクスチャに戻す
			m_polygon->SetMaterial(m_spTexWalk);
			m_polygon->SetSplit(9, 1);
		}

		m_pos += m_knockBackVec;
		m_knockBackVec *= KnockBackDecay;
	}
	else
	{
		m_knockBackVec = Math::Vector3::Zero;

		if (m_deathState == DeathState::KnockBack)
		{
			m_deathState = DeathState::Dissolve;
			return;
		}

		//===================================================================
		// 攻撃中の処理
		//===================================================================
		if (m_isAttacking)
		{
			m_attackAnimeCnt += AttackAnimeSpeed;

			// ダメージを与えるコマに達したらプレイヤーにダメージ
			if ((int)m_attackAnimeCnt == AttackDamageFrame && !m_isDamageGiven)
			{
				m_isDamageGiven = true;

				// プレイヤーを探してダメージを与える
				for (auto& obj : SceneManager::Instance().GetObjList())
				{
					std::shared_ptr<Player> player =
						std::dynamic_pointer_cast<Player>(obj);
					if (player == nullptr) { continue; }

					player->TakeDamage(AttackDamage);
					break;
				}
			}

			// 攻撃アニメーション終了
			if ((int)m_attackAnimeCnt >= AttackFrameCount)
			{
				m_isAttacking = false;
				m_attackAnimeCnt = 0.0f;
				m_isDamageGiven = false;

				// 通常テクスチャに戻す
				m_polygon->SetMaterial(m_spTexWalk);
				m_polygon->SetSplit(9, 1);
			}
		}
		else
		{
			//===================================================================
			// ホーミング処理（攻撃中でない場合のみ）
			//===================================================================
			Math::Vector3 playerPos = {};

			if (SearchPlayer(playerPos))
			{
				Math::Vector3 enemyMove = playerPos - m_pos;

				static const float flipThreshold = 0.5f;
				if (enemyMove.x > flipThreshold)
				{
					m_isFlip = false;
				}
				else if (enemyMove.x < -flipThreshold)
				{
					m_isFlip = true;
				}

				// プレイヤーとの距離を計算
				float distToPlayer = enemyMove.Length();

				// クールタイムカウンタを更新
				if (m_attackCoolTimer > 0)
				{
					m_attackCoolTimer--;
				}

				if (distToPlayer <= AttackRange && m_attackCoolTimer <= 0)
				{
					// 攻撃開始
					m_isAttacking = true;
					m_attackAnimeCnt = 0.0f;
					m_isDamageGiven = false;

					// クールタイムをセット
					m_attackCoolTimer = AttackCoolTime;

					m_polygon->SetMaterial(m_spTexAttack);
					m_polygon->SetSplit(7, 1);
				}
				else
				{
					// 通常移動
					enemyMove.Normalize();
					m_dir = enemyMove;
					m_pos += m_dir * m_speed;
				}
			}
		}
	}

	//===================================================================
	// アニメーション更新
	//===================================================================
	if (m_isAttacking)
	{
		// 攻撃アニメーションはUpdate()内で進めているのでここでは何もしない
	}
	else if (m_isHurt)
	{
		m_hurtAnimeCnt += HurtAnimeSpeed;

		if (m_hurtAnimeCnt >= (float)HurtFrameCount)
		{
			m_isHurt = false;
			m_hurtAnimeCnt = 0.0f;
			m_animeCnt = 0.0f;

			m_polygon->SetMaterial(m_spTexWalk);
			m_polygon->SetSplit(9, 1);
		}
	}
	else
	{
		m_animeCnt += m_animeSpeed;

		if (m_animeCnt >= (float)FrameCount)
		{
			m_animeCnt = 0;
		}
	}
}

void Enemy::Init()
{
	m_polygon = std::make_shared<KdSquarePolygon>();

	m_spTexWalk = std::make_shared<KdTexture>("Asset/Textures/Enemy/MobEnemy/WALK.png");
	m_spTexHurt = std::make_shared<KdTexture>("Asset/Textures/Enemy/MobEnemy/HURT.png");

	m_polygon->SetMaterial(m_spTexWalk);
	m_polygon->SetPivot(KdSquarePolygon::PivotType::Center_Bottom);
	m_polygon->SetScale(2.0f);
	m_polygon->SetSplit(9, 1);
	m_polygon->SetUVRect(0);

	m_animeCnt = 0;
	m_animeSpeed = 0.1f;

	m_pos = {};
	m_dir = {};
	m_speed = 0.03f;

	m_hp = MaxHP;

	m_knockBackVec = Math::Vector3::Zero;
	m_invincibleTimer = 0;

	m_isHurt = false;
	m_hurtAnimeCnt = 0.0f;

	// 死亡演出初期化
	m_deathState = DeathState::Alive;
	m_dissolve = 0.0f;

	m_mWorld = Math::Matrix::Identity;

	m_pDebugWire = std::make_unique<KdDebugWireFrame>();

	m_pCollider = std::make_unique<KdCollider>();
	m_pCollider->RegisterCollisionShape(
		"BumpCollision",
		Math::Vector3(0.0f, 0.5f, 0.0f),
		0.5f,
		KdCollider::TypeBump
	);
	m_pCollider->RegisterCollisionShape(
		"DamageCollision",
		Math::Vector3(0.0f, 0.5f, 0.0f),
		0.5f,
		KdCollider::TypeDamage
	);

	// 攻撃テクスチャのロード
	m_spTexAttack = std::make_shared<KdTexture>(
		"Asset/Textures/Enemy/MobEnemy/DOWN_SWING.png"
	);

	// 攻撃関連初期化
	m_isAttacking = false;
	m_attackAnimeCnt = 0.0f;
	m_isDamageGiven = false;

	// クールタイム初期化
	m_attackCoolTimer = 0;
}

void Enemy::PostUpdate()
{
	//===================================================================
	// 敵同士の押し戻し処理
	// ディゾルブ中はスキップ
	//===================================================================
	if (m_deathState != DeathState::Dissolve)
	{
		for (auto& obj : SceneManager::Instance().GetObjList())
		{
			if (obj.get() == this) { continue; }

			std::shared_ptr<Enemy> otherEnemy = std::dynamic_pointer_cast<Enemy>(obj);
			if (otherEnemy == nullptr) { continue; }
			if (!otherEnemy->m_pCollider) { continue; }

			KdCollider::SphereInfo sphere;
			sphere.m_sphere.Center = m_pos;
			sphere.m_sphere.Center.y += 0.5f;
			sphere.m_sphere.Radius = 0.5f;
			sphere.m_type = KdCollider::TypeBump;

			std::list<KdCollider::CollisionResult> results;
			if (otherEnemy->Intersects(sphere, &results))
			{
				for (auto& result : results)
				{
					Math::Vector3 pushDir = result.m_hitDir;
					pushDir.y = 0.0f;
					pushDir.Normalize();
					m_pos += pushDir * result.m_overlapDistance;
				}
			}
		}
	}

	m_mWorld = Math::Matrix::CreateTranslation(m_pos);
}

void Enemy::DrawLit()
{
	// 現在の状態に応じてコマ数と分割数を決定
	int   currentFrame = 0;
	int   splitX = 0;

	if (m_isAttacking)
	{
		// 攻撃アニメーション
		currentFrame = (int)m_attackAnimeCnt;
		splitX = AttackFrameCount;
	}
	else if (m_isHurt)
	{
		// 被弾アニメーション
		currentFrame = (int)m_hurtAnimeCnt;
		splitX = HurtFrameCount;
	}
	else
	{
		// 歩行アニメーション
		currentFrame = (int)m_animeCnt;
		splitX = FrameCount;
	}

	float w = 1.0f / splitX;
	int   x = currentFrame % splitX;

	if (m_isFlip)
	{
		m_polygon->SetUVRect(
			Math::Vector2((x + 1) * w, 0.0f),
			Math::Vector2(x * w, 1.0f)
		);
	}
	else
	{
		m_polygon->SetUVRect(
			Math::Vector2(x * w, 0.0f),
			Math::Vector2((x + 1) * w, 1.0f)
		);
	}

	// ディゾルブ描画
	if (m_deathState == DeathState::Dissolve)
	{
		float         range = 0.3f;
		Math::Vector3 color = { 1.0f, 0.3f, 0.3f };
		KdShaderManager::Instance().m_StandardShader.SetDissolve(
			m_dissolve, &range, &color
		);
	}

	KdShaderManager::Instance().m_StandardShader.DrawPolygon(*m_polygon, m_mWorld);
}

void Enemy::GenerateDepthMapFromLight()
{
	int   currentFrame = 0;
	int   splitX = 0;

	if (m_isAttacking)
	{
		currentFrame = (int)m_attackAnimeCnt;
		splitX = AttackFrameCount;
	}
	else if (m_isHurt)
	{
		currentFrame = (int)m_hurtAnimeCnt;
		splitX = HurtFrameCount;
	}
	else
	{
		currentFrame = (int)m_animeCnt;
		splitX = FrameCount;
	}

	float w = 1.0f / splitX;
	int   x = currentFrame % splitX;

	if (m_isFlip)
	{
		m_polygon->SetUVRect(
			Math::Vector2((x + 1) * w, 0.0f),
			Math::Vector2(x * w, 1.0f)
		);
	}
	else
	{
		m_polygon->SetUVRect(
			Math::Vector2(x * w, 0.0f),
			Math::Vector2((x + 1) * w, 1.0f)
		);
	}

	KdShaderManager::Instance().m_StandardShader.DrawPolygon(*m_polygon, m_mWorld);
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// ダメージを受ける関数
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void Enemy::TakeDamage(int _damage, const Math::Vector3& _knockBack)
{
	if (m_invincibleTimer > 0) { return; }

	m_hp -= _damage;
	m_knockBackVec = _knockBack;
	m_invincibleTimer = InvincibleTime;

	//===================================================================
	// 攻撃中にダメージを受けた場合は攻撃をキャンセルする
	//===================================================================
	if (m_isAttacking)
	{
		m_isAttacking = false;
		m_attackAnimeCnt = 0.0f;
		m_isDamageGiven = false;
	}

	// 被弾アニメーション開始
	m_isHurt = true;
	m_hurtAnimeCnt = 0.0f;

	m_polygon->SetMaterial(m_spTexHurt);
	m_polygon->SetSplit(2, 1);

	if (m_hp <= 0)
	{
		m_deathState = DeathState::KnockBack;
		m_pCollider->SetEnableAll(false);
	}
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// プレイヤーを探して座標を取得する
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