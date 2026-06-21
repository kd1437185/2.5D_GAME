#include "Enemy.h"

#include "../../Scene/SceneManager.h"
#include "../Player/Player.h"


//===================================================================
// static メンバの実体定義
// 全Enemyで共有するテクスチャ
//===================================================================
std::shared_ptr<KdTexture> Enemy::m_spTexWalk = nullptr;
std::shared_ptr<KdTexture> Enemy::m_spTexHurt = nullptr;
std::shared_ptr<KdTexture> Enemy::m_spTexAttack = nullptr;
std::shared_ptr<KdTexture> Enemy::m_spHpBarBg = nullptr;
std::shared_ptr<KdTexture> Enemy::m_spHpBarFill = nullptr;

void Enemy::Update()
{
	//===================================================================
	// 演出中（必殺技など）はUpdateをスキップする
	//===================================================================
	if (SceneManager::Instance().IsCutScene())
	{
		return;
	}

	//===================================================================
	// 敵の速度倍率を取得する
	// 減速アクション中は 0.5 になり、速度・アニメ速度が半分になる
	//===================================================================
	float speedRate = SceneManager::Instance().GetEnemySpeedRate();

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

			auto se = KdAudioManager::Instance().Play("Asset/Sounds/消滅.wav", false);
			if (se) { se->SetVolume(1.0f); }
			return;
		}

		//===================================================================
		// 攻撃中の処理
		//===================================================================
		if (m_isAttacking)
		{
			// 攻撃アニメ速度に減速倍率を反映
			m_attackAnimeCnt += AttackAnimeSpeed * speedRate;

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

				// Y成分を消して水平方向のみで追従する
				// （ジャンプ中のプレイヤーを浮いて追わないようにする）
				enemyMove.y = 0.0f;

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
					// 通常移動（移動速度に減速倍率を反映）
					enemyMove.Normalize();
					m_dir = enemyMove;
					m_pos += m_dir * m_speed * speedRate;
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
		// 被弾アニメ速度に減速倍率を反映
		m_hurtAnimeCnt += HurtAnimeSpeed * speedRate;

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
		// 歩行アニメ速度に減速倍率を反映
		m_animeCnt += m_animeSpeed * speedRate;

		if (m_animeCnt >= (float)FrameCount)
		{
			m_animeCnt = 0;
		}
	}
}

void Enemy::Init()
{
	m_polygon = std::make_shared<KdSquarePolygon>();

	//===================================================================
	// テクスチャのロード（まだロードされていない場合のみ）
	// static なので最初の1体目だけロードし、以降は使い回す
	//===================================================================
	if (m_spTexWalk == nullptr)
	{
		m_spTexWalk = std::make_shared<KdTexture>("Asset/Textures/Enemy/MobEnemy/WALK.png");
	}
	if (m_spTexHurt == nullptr)
	{
		m_spTexHurt = std::make_shared<KdTexture>("Asset/Textures/Enemy/MobEnemy/HURT.png");
	}
	if (m_spTexAttack == nullptr)
	{
		m_spTexAttack = std::make_shared<KdTexture>("Asset/Textures/Enemy/MobEnemy/DOWN_SWING.png");
	}
	if (m_spHpBarBg == nullptr)
	{
		m_spHpBarBg = std::make_shared<KdTexture>("Asset/Textures/UI/bg.png");
	}
	if (m_spHpBarFill == nullptr)
	{
		m_spHpBarFill = std::make_shared<KdTexture>("Asset/Textures/UI/green.png");
	}

	m_polygon->SetMaterial(m_spTexWalk);
	m_polygon->SetPivot(KdSquarePolygon::PivotType::Center_Bottom);
	m_polygon->SetScale(1.0f);
	m_polygon->SetSplit(9, 1);
	m_polygon->SetUVRect(0);

	m_animeCnt = 0;
	m_animeSpeed = 0.1f;

	m_pos = {};
	m_dir = {};
	m_speed = 0.02f;

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

	//===================================================================
	// 当たり判定の登録
	//===================================================================
	m_pCollider = std::make_unique<KdCollider>();

	// TypeBump：敵同士が重ならないための判定
	m_pCollider->RegisterCollisionShape(
		"BumpCollision",
		Math::Vector3(0.0f, 0.5f, 0.0f),
		0.5f,
		KdCollider::TypeBump
	);

	// TypeDamage：攻撃を受ける側の判定
	m_pCollider->RegisterCollisionShape(
		"DamageCollision",
		Math::Vector3(0.0f, 0.5f, 0.0f),
		0.5f,
		KdCollider::TypeDamage
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

	//===================================================================
	// Y座標を固定する
	// ホーミング中も常に一定の高さを保つ
	// （ジャンプ中のプレイヤーを浮いて追わないようにする）
	//===================================================================
	m_pos.y = -0.25f;

	m_mWorld = Math::Matrix::CreateTranslation(m_pos);
}

void Enemy::DrawLit()
{
	// 現在の状態に応じてコマ数と分割数を決定
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

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 2D描画
// 敵の足元にHPバーを表示する（3D座標→2D座標に変換）
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void Enemy::DrawSprite()
{
	// 死亡演出中はHPバーを出さない
	if (m_deathState != DeathState::Alive) { return; }

	// カメラを取得
	KdCamera* camera = SceneManager::Instance().GetCamera();
	if (camera == nullptr) { return; }

	//===================================================================
	// 敵の足元の3D座標を2D座標に変換する
	//===================================================================
	Math::Vector3 worldPos = m_pos;
	worldPos.y += 0.1f;	// 足元より少し上（地面とのちらつき防止）

	Math::Vector3 screenPos = Math::Vector3::Zero;
	camera->ConvertWorldToScreenDetail(worldPos, screenPos);

	//===================================================================
	// HPバーのサイズ（プレイヤーより小さく）
	//===================================================================
	const int barW = 40;	// 幅（小さめ）
	const int barH = 10;	// 高さ（小さめ）

	//===================================================================
	// HP割合を計算
	//===================================================================
	float hpRate = (float)m_hp / (float)MaxHP;

	//===================================================================
	// 枠を先に描画
	//===================================================================
	KdShaderManager::Instance().m_spriteShader.DrawTex(
		m_spHpBarBg.get(),
		(int)screenPos.x,
		(int)screenPos.y,
		barW, barH,
		nullptr, &kWhiteColor,
		Math::Vector2(0.5f, 0.5f)
	);

	//===================================================================
	// 中身を枠の上に描画（HP割合に応じて右から減らす）
	//===================================================================
	float fillW = barW * hpRate;
	float fillCenterX = (screenPos.x - barW * 0.5f) + fillW * 0.5f;

	Math::Rectangle srcRect;
	srcRect.x = 0;
	srcRect.y = 0;
	srcRect.width = (long)(m_spHpBarFill->GetWidth() * hpRate);
	srcRect.height = (long)m_spHpBarFill->GetHeight();

	if (fillW > 0.0f)
	{
		KdShaderManager::Instance().m_spriteShader.DrawTex(
			m_spHpBarFill.get(),
			(int)fillCenterX,
			(int)screenPos.y,
			(int)fillW, barH,
			&srcRect, &kWhiteColor,
			Math::Vector2(0.5f, 0.5f)
		);
	}
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

		// 討伐数を加算
		SceneManager::Instance().AddKillCount();
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