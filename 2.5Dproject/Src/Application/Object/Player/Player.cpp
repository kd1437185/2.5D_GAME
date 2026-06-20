#include "Player.h"

#include "../../Scene/SceneManager.h"
#include "../Effect/AttackEffect.h"
#include "../Effect/SpecialEffect.h"

void Player::Update()
{
	//===================================================================
	// クールタイムタイマーの更新
	//===================================================================
	if (m_dashCoolTimer > 0) { m_dashCoolTimer--; }

	//===================================================================
	// 被弾無敵タイマーの更新
	//===================================================================
	if (m_damageInvincibleTimer > 0)
	{
		m_damageInvincibleTimer--;
	}

	//===================================================================
	// 減速効果の管理
	// 効果時間が残っている間は敵を半速にする
	//===================================================================
	if (m_slowEffectTimer > 0)
	{
		m_slowEffectTimer--;

		// 敵を半速に
		SceneManager::Instance().SetEnemySpeedRate(0.5f);

		// 効果終了時に通常速度へ戻す
		if (m_slowEffectTimer <= 0)
		{
			SceneManager::Instance().SetEnemySpeedRate(1.0f);
		}
	}

	//===================================================================
	// 残像の更新
	// 既存の残像を薄くしていき、減速効果中は新しい残像を追加する
	//===================================================================

	// 既存の残像を薄くしていく
	for (auto it = m_afterImages.begin(); it != m_afterImages.end();)
	{
		it->m_alpha -= 0.05f;

		if (it->m_alpha <= 0.0f)
		{
			// 完全に消えたら削除
			it = m_afterImages.erase(it);
		}
		else
		{
			++it;
		}
	}

	// 減速効果中は一定間隔で残像を追加する
	if (m_slowEffectTimer > 0)
	{
		m_afterImageTimer++;

		// 3フレームごとに残像を追加
		if (m_afterImageTimer >= 3)
		{
			m_afterImageTimer = 0;

			AfterImage img;
			img.m_pos = m_pos;
			img.m_isFlip = (m_lastDirType & DirType::Left) != 0;
			img.m_alpha = 0.5f;	// 初期透明度

			// 現在表示中のテクスチャを残像として保存
			if (m_pCurrentTextures != nullptr)
			{
				int cnt = (int)m_animeInfo.count % (int)m_pCurrentTextures->size();
				img.m_spTex = (*m_pCurrentTextures)[cnt];
			}

			m_afterImages.push_back(img);
		}
	}

	//===================================================================
	// 必殺技中の処理
	// プレイヤーアニメーション以外の動きを停止する
	//===================================================================
	if (m_isSpecial)
	{
		m_specialAnimeCnt += SpecialAnimeSpeed;

		int animeCnt = (int)m_specialAnimeCnt;

		if (animeCnt >= SpecialFrameCount)
		{
			m_isSpecial = false;
			m_specialAnimeCnt = 0.0f;
			m_animeInfo.count = 0;

			SceneManager::Instance().SetCutScene(false);
		}
		else
		{
			m_polygon.SetMaterial(m_animTexturesSpecial[animeCnt]);
		}

		return;
	}

	//===================================================================
	// 減速発動アニメーション中の処理
	// 発動アニメ中は他の動きを止める
	//===================================================================
	if (m_isSlow)
	{
		m_slowAnimeCnt += SlowAnimeSpeed;

		int animeCnt = (int)m_slowAnimeCnt;

		if (animeCnt >= SlowFrameCount)
		{
			// 発動アニメ終了 → 減速効果開始
			m_isSlow = false;
			m_slowAnimeCnt = 0.0f;
			m_slowEffectTimer = SlowEffectTime;
			m_animeInfo.count = 0;

			// 敵の完全停止を解除（ここから半速になる）
			SceneManager::Instance().SetCutScene(false);
		}
		else
		{
			m_polygon.SetMaterial(m_animTexturesSlow[animeCnt]);
		}

		return;	// 発動アニメ中は以降をスキップ
	}

	//===================================================================
	// 回避中の処理
	// ダッシュ中は他の動作を行えない
	//===================================================================
	if (m_isDashing)
	{
		// 回避アニメーションを進める
		m_dashAnimeCnt += DashAnimeSpeed;

		// 向いている方向に高速移動
		Math::Vector3 dashDir = Math::Vector3::Zero;

		if (m_lastDirType & DirType::Left)
		{
			dashDir = Math::Vector3(-1.0f, 0.0f, 0.0f);
		}
		else
		{
			dashDir = Math::Vector3(1.0f, 0.0f, 0.0f);
		}

		m_pos += dashDir * DashSpeed;

		// 回避アニメーション終了
		if ((int)m_dashAnimeCnt >= DashFrameCount)
		{
			m_isDashing = false;
			m_isDashInvincible = false;
			m_dashAnimeCnt = 0.0f;
			m_animeInfo.count = 0;
		}

		// 重力処理
		m_gravity += 0.005f;
		m_pos.y -= m_gravity;

		//===================================================================
		// 当たり判定・・・レイ判定（地面との判定）
		//===================================================================
		KdCollider::RayInfo rayInfo;
		rayInfo.m_pos = m_pos;

		static const float enableStepHigh = 0.2f;
		rayInfo.m_pos.y += enableStepHigh;
		rayInfo.m_dir = { 0.0f, -1.0f, 0.0f };
		rayInfo.m_range = enableStepHigh + m_gravity;
		rayInfo.m_type = KdCollider::TypeGround;

		std::list<KdCollider::CollisionResult> retRayList;
		for (auto& obj : SceneManager::Instance().GetObjList())
		{
			obj->Intersects(rayInfo, &retRayList);
		}

		bool  hit = false;
		float maxOverLap = 0;
		Math::Vector3 groundPos = {};

		for (auto& ret : retRayList)
		{
			if (maxOverLap < ret.m_overlapDistance)
			{
				maxOverLap = ret.m_overlapDistance;
				groundPos = ret.m_hitPos;
				hit = true;
			}
		}

		if (hit)
		{
			m_pos = groundPos;
			m_gravity = 0.0f;
		}

		//===================================================================
		// 球（スフィア）判定（壁との判定）
		//===================================================================
		KdCollider::SphereInfo sphere;
		sphere.m_sphere.Center = m_pos;
		sphere.m_sphere.Center.y += 0.5f;
		sphere.m_sphere.Radius = 0.3f;
		sphere.m_type = KdCollider::TypeGround;

		std::list<KdCollider::CollisionResult> retSphereList;
		for (auto& obj : SceneManager::Instance().GetObjList())
		{
			obj->Intersects(sphere, &retSphereList);
		}

		maxOverLap = 0;
		hit = false;
		Math::Vector3 hitDir;

		for (auto& ret : retSphereList)
		{
			if (maxOverLap < ret.m_overlapDistance)
			{
				maxOverLap = ret.m_overlapDistance;
				hitDir = ret.m_hitDir;
				hit = true;
			}
		}

		if (hit == true)
		{
			hitDir.Normalize();
			m_pos += hitDir * maxOverLap;
		}

		// テクスチャをセット
		int animeCnt = (int)m_dashAnimeCnt;
		if (animeCnt >= DashFrameCount) { animeCnt = DashFrameCount - 1; }

		if (m_lastDirType & DirType::Left)
		{
			m_polygon.SetMaterial(m_animTexturesDashL[animeCnt]);
		}
		else
		{
			m_polygon.SetMaterial(m_animTexturesDashR[animeCnt]);
		}

		return;
	}

	//===================================================================
	// 攻撃入力
	//===================================================================
	if (GetAsyncKeyState('Z') & 0x8000)
	{
		if (!m_keyFlg && !m_isAttacking)
		{
			m_keyFlg = true;
			m_isAttacking = true;

			m_animeInfo.count = 0;
			m_animeInfo.speed = 0.2f;

			KdAudioManager::Instance().Play("Asset/Sounds/Attack.WAV", false);

			auto effect = std::make_shared<AttackEffect>();
			effect->Init();

			Math::Vector3 effectPos = m_pos;
			effectPos.y += 0.5f;

			bool isFlip = false;

			if (m_lastDirType & DirType::Left)
			{
				effectPos.x -= 1.0f;
				isFlip = true;
			}
			else
			{
				effectPos.x += 1.0f;
				isFlip = false;
			}

			effect->Setup(effectPos, isFlip);
			SceneManager::Instance().AddObject(effect);
		}
	}
	else
	{
		m_keyFlg = false;
	}

	//===================================================================
	// 回避入力
	//===================================================================
	if (GetAsyncKeyState('X') & 0x8000)
	{
		if (!m_isAttacking && m_dashCoolTimer <= 0)
		{
			m_isDashing = true;
			m_isDashInvincible = true;
			m_dashAnimeCnt = 0.0f;

			m_dashCoolTimer = DashCoolTime;
		}
	}

	//===================================================================
	// 必殺技入力
	//===================================================================
	if (GetAsyncKeyState('A') & 0x8000)
	{
		// 攻撃中・回避中でない かつ SPが足りていれば発動
		if (!m_isAttacking && !m_isDashing && m_sp >= SpecialCostSP)
		{
			// SPを消費（足りていれば true が返る）
			if (UseSp(SpecialCostSP))
			{
				// 必殺技開始
				m_isSpecial = true;
				m_specialAnimeCnt = 0.0f;

				SceneManager::Instance().SetCutScene(true);

				auto effect = std::make_shared<SpecialEffect>();
				effect->Init();
				SceneManager::Instance().AddObject(effect);
			}
		}
	}

	//===================================================================
	// 減速アクション入力（Sキー）
	// 攻撃中・回避中・必殺技中でない・クールタイム中でない場合のみ
	//===================================================================
	if (GetAsyncKeyState('S') & 0x8000)
	{
		// 攻撃中・回避中・必殺技中でない かつ SPが足りていれば発動
		if (!m_isAttacking && !m_isDashing && !m_isSpecial && m_sp >= SlowCostSP)
		{
			// SPを消費（足りていれば true が返る）
			if (UseSp(SlowCostSP))
			{
				m_isSlow = true;
				m_slowAnimeCnt = 0.0f;

				// 発動アニメ中は敵を完全停止
				SceneManager::Instance().SetCutScene(true);
			}
		}
	}

	//===================================================================
	// 移動入力（攻撃中は受け付けない）
	//===================================================================
	if (!m_isAttacking)
	{
		m_dir = {};
		UINT oldDirType = m_dirType;
		m_dirType = 0;

		if (GetAsyncKeyState(VK_UP) & 0x8000)
		{
			m_dir += { 0, 0, 1 };
			m_dirType |= DirType::Up;
		}
		if (GetAsyncKeyState(VK_DOWN) & 0x8000)
		{
			m_dir += { 0, 0, -1 };
			m_dirType |= DirType::Down;
		}
		if (GetAsyncKeyState(VK_LEFT) & 0x8000)
		{
			m_dir += { -1, 0, 0 };
			m_dirType |= DirType::Left;
		}
		if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
		{
			m_dir += { 1, 0, 0 };
			m_dirType |= DirType::Right;
		}

		if (m_dirType != 0 && m_dirType != oldDirType)
		{
			ChangeAnimation();
		}

		m_dir.Normalize();
		m_pos += m_dir * m_speed;
	}

	// ジャンプ処理
	if (GetAsyncKeyState(VK_SPACE) & 0x8000)
	{
		m_gravity = -0.1f;
	}

	m_gravity += 0.005f;
	m_pos.y -= m_gravity;

	//===================================================================
	// 向きに応じてテクスチャリストを切り替える
	//===================================================================
	if (m_dirType & DirType::Left)
	{
		m_lastDirType = DirType::Left;
	}
	else if (m_dirType & DirType::Right)
	{
		m_lastDirType = DirType::Right;
	}

	//===================================================================
	// アニメーションのテクスチャリストを状態に応じて切り替える
	// 攻撃中 > 移動中 > 待機 の優先順位
	//===================================================================
	if (m_isAttacking)
	{
		if (m_lastDirType & DirType::Left)
		{
			m_pCurrentTextures = &m_animTexturesAttackL;
		}
		else
		{
			m_pCurrentTextures = &m_animTexturesAttackR;
		}
	}
	else if (m_dirType != 0)
	{
		if (m_lastDirType & DirType::Left)
		{
			m_pCurrentTextures = &m_animTexturesL;
		}
		else
		{
			m_pCurrentTextures = &m_animTexturesR;
		}
	}
	else
	{
		if (m_lastDirType & DirType::Left)
		{
			m_pCurrentTextures = &m_animTexturesIdleL;
		}
		else
		{
			m_pCurrentTextures = &m_animTexturesIdleR;
		}
	}

	//===================================================================
	// アニメーション更新
	//===================================================================
	m_animeInfo.count += m_animeInfo.speed;
	int animeCnt = static_cast<int>(m_animeInfo.count);

	if (animeCnt >= (int)m_pCurrentTextures->size())
	{
		if (m_isAttacking)
		{
			m_isAttacking = false;
			m_animeInfo.count = 0;
			animeCnt = 0;
		}
		else
		{
			animeCnt = 0;
			m_animeInfo.count = 0;
		}
	}

	m_polygon.SetMaterial((*m_pCurrentTextures)[animeCnt]);

	// ==========================================
	// 当たり判定・・・レイ判定
	// ==========================================
	KdCollider::RayInfo rayInfo;
	rayInfo.m_pos = m_pos;

	static const float enableStepHigh = 0.2f;
	rayInfo.m_pos.y += enableStepHigh;
	rayInfo.m_dir = { 0.0f, -1.0f, 0.0f };
	rayInfo.m_range = enableStepHigh + m_gravity;
	rayInfo.m_type = KdCollider::TypeGround;

	//m_pDebugWire->AddDebugLine(rayInfo.m_pos, rayInfo.m_dir, rayInfo.m_range);

	std::list<KdCollider::CollisionResult> retRayList;

	for (auto& obj : SceneManager::Instance().GetObjList())
	{
		obj->Intersects(rayInfo, &retRayList);
	}

	bool hit = false;
	float maxOverLap = 0;
	Math::Vector3 groundPos = {};

	for (auto& ret : retRayList)
	{
		if (maxOverLap < ret.m_overlapDistance)
		{
			maxOverLap = ret.m_overlapDistance;
			groundPos = ret.m_hitPos;
			hit = true;
		}
	}

	if (hit)
	{
		m_pos = groundPos;
		m_gravity = 0.0f;
	}

	// ========================
	// 球（スフィア）判定
	// ========================
	KdCollider::SphereInfo sphere;
	sphere.m_sphere.Center = m_pos;
	sphere.m_sphere.Center.y += 0.5f;
	sphere.m_sphere.Radius = 0.3f;
	sphere.m_type = KdCollider::TypeGround;

	//m_pDebugWire->AddDebugSphere(sphere.m_sphere.Center, sphere.m_sphere.Radius);

	std::list<KdCollider::CollisionResult> retSphereList;

	for (auto& obj : SceneManager::Instance().GetObjList())
	{
		obj->Intersects(sphere, &retSphereList);
	}

	maxOverLap = 0;
	hit = false;
	Math::Vector3 hitDir;

	for (auto& ret : retSphereList)
	{
		if (maxOverLap < ret.m_overlapDistance)
		{
			maxOverLap = ret.m_overlapDistance;
			hitDir = ret.m_hitDir;
			hit = true;
		}
	}

	if (hit == true)
	{
		hitDir.Normalize();
		m_pos += hitDir * maxOverLap;
	}
}

void Player::PostUpdate()
{
	Math::Matrix transMat;
	transMat = Math::Matrix::CreateTranslation(m_pos);
	m_mWorld = transMat;
}

void Player::GenerateDepthMapFromLight()
{
	KdShaderManager::Instance().m_StandardShader.DrawPolygon(m_polygon, m_mWorld);
}

void Player::DrawLit()
{
	//===================================================================
	// 残像の描画
	//===================================================================
	for (auto& img : m_afterImages)
	{
		if (img.m_spTex == nullptr) { continue; }

		m_afterImagePolygon.SetMaterial(img.m_spTex);

		float bright = img.m_alpha;
		m_afterImagePolygon.SetColor(
			Math::Color(bright * 2.0f, bright * 0.6f, bright * 0.6f, 1.0f)
		);

		if (img.m_isFlip)
		{
			m_afterImagePolygon.SetUVRect(
				Math::Vector2(1.0f, 0.0f), Math::Vector2(0.0f, 1.0f)
			);
		}
		else
		{
			m_afterImagePolygon.SetUVRect(
				Math::Vector2(0.0f, 0.0f), Math::Vector2(1.0f, 1.0f)
			);
		}

		Math::Matrix afterMat = Math::Matrix::CreateTranslation(img.m_pos);
		KdShaderManager::Instance().m_StandardShader.DrawPolygon(
			m_afterImagePolygon, afterMat
		);
	}

	//===================================================================
	// 被弾無敵中の点滅処理
	// BlinkInterval ごとに描画する・しないを切り替える
	//===================================================================
	if (m_damageInvincibleTimer > 0)
	{
		// タイマーを間隔で割った余りで表示・非表示を決める
		// 例：間隔5なら 0〜4は表示、5〜9は非表示 を繰り返す
		int blinkPhase = (m_damageInvincibleTimer / BlinkInterval) % 2;

		// 非表示のタイミングなら本体を描画しない
		if (blinkPhase == 1)
		{
			return;
		}
	}

	// 本体の描画
	KdShaderManager::Instance().m_StandardShader.DrawPolygon(m_polygon, m_mWorld);
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 2D描画
// HPバーとSPバーを画面左上に表示する
// 座標系：画面中央が(0,0)・右が+X・上が+Y
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void Player::DrawSprite()
{
	//===================================================================
	// バーの基本サイズ
	//===================================================================
	const float barW = 512.0f;	// バーの幅
	const float barH = 48.0f;	// バーの高さ

	//===================================================================
	// HPバーの表示位置（画面左上）
	// 中央原点なので X = -640 + 余白 + 幅半分、Y = 360 - 余白 - 高さ半分
	//===================================================================
	float posX = -640.0f + 20.0f + barW * 0.5f;
	float posY = 360.0f - 20.0f - barH * 0.5f;

	//===================================================================
	// === HPバー ===
	//===================================================================

	// HP割合を計算（0.0〜1.0）
	float hpRate = (float)m_hp / (float)MaxHP;

	//-------------------------------------------------
	// HPバーの枠を先に描画
	//-------------------------------------------------
	KdShaderManager::Instance().m_spriteShader.DrawTex(
		m_spHpBarBg.get(),
		(int)posX,
		(int)posY,
		(int)barW,
		(int)barH,
		nullptr,
		&kWhiteColor,
		Math::Vector2(0.5f, 0.5f)
	);

	//-------------------------------------------------
	// HPバーの中身を枠の上に描画（HP割合に応じて右から減らす）
	//-------------------------------------------------
	// 中身の描画幅
	float fillW = barW * hpRate;

	// 中身の左端を枠の左端に合わせるため中心X座標をずらす
	float fillCenterX = (posX - barW * 0.5f) + fillW * 0.5f;

	// UV範囲も hpRate に合わせて切り取る（右から減らすので左側を残す）
	Math::Rectangle srcRect;
	srcRect.x = 0;
	srcRect.y = 0;
	srcRect.width = (long)(m_spHpBarFill->GetWidth() * hpRate);
	srcRect.height = (long)m_spHpBarFill->GetHeight();

	// HPが0より大きいときだけ描画
	if (fillW > 0.0f)
	{
		KdShaderManager::Instance().m_spriteShader.DrawTex(
			m_spHpBarFill.get(),
			(int)fillCenterX,
			(int)posY,
			(int)fillW,
			(int)barH,
			&srcRect,
			&kWhiteColor,
			Math::Vector2(0.5f, 0.5f)
		);
	}

	//===================================================================
	// === SPバー（HPバーの下に配置・HPより短め） ===
	//===================================================================

	// SPバー専用の幅（HPバーより短くする）
	float spBarW = barW * 0.7f;	// HPバーの70%の長さ

	// SPバーのY座標（HPバーの下に配置・上に寄せる）
	float spPosY = posY - barH + 10.0f;

	// SPバーの左端をHPバーの左端に揃える
	// 中央原点なので左端基準で中心X座標を計算しなおす
	float spPosX = (posX - barW * 0.5f) + spBarW * 0.5f;

	//-------------------------------------------------
	// SPバーの枠を先に描画（HPバーと同じ枠画像を使う）
	//-------------------------------------------------
	KdShaderManager::Instance().m_spriteShader.DrawTex(
		m_spHpBarBg.get(),
		(int)spPosX,
		(int)spPosY,
		(int)spBarW,
		(int)barH,
		nullptr,
		&kWhiteColor,
		Math::Vector2(0.5f, 0.5f)
	);

	//-------------------------------------------------
	// SPバーの中身を枠の上に描画
	//-------------------------------------------------
	// SP割合を計算（0.0〜1.0）
	float spRate = (float)m_sp / (float)MaxSP;

	// SP中身の描画幅
	float spFillW = spBarW * spRate;

	// 中身の左端を枠の左端に合わせる
	float spFillCenterX = (spPosX - spBarW * 0.5f) + spFillW * 0.5f;

	// UV範囲も spRate に合わせて切り取る
	Math::Rectangle spSrcRect;
	spSrcRect.x = 0;
	spSrcRect.y = 0;
	spSrcRect.width = (long)(m_spSpBarFill->GetWidth() * spRate);
	spSrcRect.height = (long)m_spSpBarFill->GetHeight();

	// SPが0より大きいときだけ描画
	if (spFillW > 0.0f)
	{
		KdShaderManager::Instance().m_spriteShader.DrawTex(
			m_spSpBarFill.get(),
			(int)spFillCenterX,
			(int)spPosY,
			(int)spFillW,
			(int)barH,
			&spSrcRect,
			&kWhiteColor,
			Math::Vector2(0.5f, 0.5f)
		);
	}
}

void Player::Init()
{
	//===================================================================
	// アニメーション用テクスチャを事前にロードしておく
	//===================================================================

	// 歩行・左向きテクスチャ（000〜007）
	for (int i = 0; i <= 7; ++i)
	{
		char fileName[256];
		sprintf_s(fileName, "Asset/Textures/Player/WalkL/frame_%03d.png", i);
		m_animTexturesL.push_back(std::make_shared<KdTexture>(fileName));
	}

	// 歩行・右向きテクスチャ（000〜007）
	for (int i = 0; i <= 7; ++i)
	{
		char fileName[256];
		sprintf_s(fileName, "Asset/Textures/Player/WalkR/frame_%03d.png", i);
		m_animTexturesR.push_back(std::make_shared<KdTexture>(fileName));
	}

	// 待機・左向きテクスチャ（000〜003）
	for (int i = 0; i <= 3; ++i)
	{
		char fileName[256];
		sprintf_s(fileName, "Asset/Textures/Player/IdelL/frame_%03d.png", i);
		m_animTexturesIdleL.push_back(std::make_shared<KdTexture>(fileName));
	}

	// 待機・右向きテクスチャ（000〜003）
	for (int i = 0; i <= 3; ++i)
	{
		char fileName[256];
		sprintf_s(fileName, "Asset/Textures/Player/IdelR/frame_%03d.png", i);
		m_animTexturesIdleR.push_back(std::make_shared<KdTexture>(fileName));
	}

	// 攻撃・左向きテクスチャ（000〜008）
	for (int i = 0; i <= 8; ++i)
	{
		char fileName[256];
		sprintf_s(fileName, "Asset/Textures/Player/AttackL/frame_%03d.png", i);
		m_animTexturesAttackL.push_back(std::make_shared<KdTexture>(fileName));
	}

	// 攻撃・右向きテクスチャ（000〜008）
	for (int i = 0; i <= 8; ++i)
	{
		char fileName[256];
		sprintf_s(fileName, "Asset/Textures/Player/AttackR/frame_%03d.png", i);
		m_animTexturesAttackR.push_back(std::make_shared<KdTexture>(fileName));
	}

	// 初期は待機・左向きを使用
	m_pCurrentTextures = &m_animTexturesIdleL;

	m_polygon.SetMaterial((*m_pCurrentTextures)[0]);
	m_polygon.SetPivot(KdSquarePolygon::PivotType::Center_Bottom);
	m_polygon.SetSplit(1, 1);

	// アニメーション初期値
	m_animeInfo.start = 0;
	m_animeInfo.end = 3;
	m_animeInfo.count = 0;
	m_animeInfo.speed = 0.2f;

	// 座標・移動速度
	m_pos = {};
	m_dir = {};
	m_speed = 0.1f;

	m_gravity = 0.0f;
	m_mWorld = Math::Matrix::Identity;

	m_keyFlg = false;
	m_isAttacking = false;

	m_pDebugWire = std::make_unique<KdDebugWireFrame>();

	// 回避・左向きテクスチャ（000〜008）
	for (int i = 0; i <= 8; ++i)
	{
		char fileName[256];
		sprintf_s(fileName, "Asset/Textures/Player/DashL/frame_%03d.png", i);
		m_animTexturesDashL.push_back(std::make_shared<KdTexture>(fileName));
	}

	// 回避・右向きテクスチャ（000〜008）
	for (int i = 0; i <= 8; ++i)
	{
		char fileName[256];
		sprintf_s(fileName, "Asset/Textures/Player/DashR/frame_%03d.png", i);
		m_animTexturesDashR.push_back(std::make_shared<KdTexture>(fileName));
	}

	// 回避関連初期化
	m_isDashing = false;
	m_isDashInvincible = false;
	m_dashAnimeCnt = 0.0f;
	m_dashCoolTimer = 0;

	// 必殺技テクスチャ（000〜008・正面なので左右共通）
	for (int i = 0; i <= 8; ++i)
	{
		char fileName[256];
		sprintf_s(fileName, "Asset/Textures/Player/Special3/frame_%03d.png", i);
		m_animTexturesSpecial.push_back(std::make_shared<KdTexture>(fileName));
	}

	// 必殺技関連初期化
	m_isSpecial = false;
	m_specialAnimeCnt = 0.0f;
	m_specialCoolTimer = 0;

	//===================================================================
	// 減速発動テクスチャ（000〜008・正面共通）
	//===================================================================
	for (int i = 0; i <= 8; ++i)
	{
		char fileName[256];
		sprintf_s(fileName, "Asset/Textures/Player/Special1/frame_%03d.png", i);
		m_animTexturesSlow.push_back(std::make_shared<KdTexture>(fileName));
	}

	// 減速関連初期化
	m_isSlow = false;
	m_slowAnimeCnt = 0.0f;
	m_slowEffectTimer = 0;
	m_slowCoolTimer = 0;
	m_afterImageTimer = 0;

	//===================================================================
	// 残像用ポリゴンの初期設定
	// 毎フレーム生成しないようにメンバとして1つ持つ
	//===================================================================
	m_afterImagePolygon.SetPivot(KdSquarePolygon::PivotType::Center_Bottom);
	m_afterImagePolygon.SetSplit(1, 1);

	//===================================================================
	// HP初期化
	//===================================================================
	m_hp = MaxHP;

	//===================================================================
	// HPバー画像のロード
	//===================================================================
	m_spHpBarBg = std::make_shared<KdTexture>("Asset/Textures/UI/bg.png");
	m_spHpBarFill = std::make_shared<KdTexture>("Asset/Textures/UI/green.png");

	//===================================================================
	// SP初期化
	//===================================================================
	m_sp = 0;

	// SPバー中身画像のロード（枠はHPバーと共用）
	m_spSpBarFill = std::make_shared<KdTexture>("Asset/Textures/UI/blue.png");

	// 被弾無敵初期化
	m_damageInvincibleTimer = 0;

}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 向きが変わった時にアニメーションカウントをリセットする
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void Player::ChangeAnimation()
{
	m_animeInfo.count = 0;
	m_animeInfo.speed = 0.2f;
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// ダメージを受ける関数
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void Player::TakeDamage(int _damage)
{
	// 回避中は無敵なのでダメージを受けない
	if (m_isDashInvincible) { return; }

	// 被弾無敵中はダメージを受けない
	if (m_damageInvincibleTimer > 0) { return; }

	// HPを減らす
	m_hp -= _damage;

	// HPが0以下にならないように制限
	if (m_hp < 0) { m_hp = 0; }

	// 被弾無敵をセット（この間点滅して無敵になる）
	m_damageInvincibleTimer = DamageInvincibleTime;

	KdDebugGUI::Instance().AddLog("Player TakeDamage:%d HP:%d", _damage, m_hp);
}