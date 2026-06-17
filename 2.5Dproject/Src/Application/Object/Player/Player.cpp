#include "Player.h"

#include "../../Scene/SceneManager.h"
#include "../Effect/AttackEffect.h"

void Player::Update()
{
	//===================================================================
	// 攻撃入力
	// 攻撃中でない場合のみ受け付ける
	//===================================================================
	if (GetAsyncKeyState('Z') & 0x8000)
	{
		if (!m_keyFlg && !m_isAttacking)
		{
			m_keyFlg = true;
			m_isAttacking = true;

			m_animeInfo.count = 0;
			m_animeInfo.speed = 0.3f;

			// 攻撃SE再生
			KdAudioManager::Instance().Play("Asset/Sounds/Attack.WAV", false);

			//===================================================================
			// 攻撃エフェクトを生成してシーンに追加する
			// 向いている方向にオフセットをかけてプレイヤーの前方に出す
			//===================================================================
			auto effect = std::make_shared<AttackEffect>();
			effect->Init();

			// 向いている方向に応じてエフェクトの座標とフリップを設定
			// 左向きなら左前方・右向きなら右前方にエフェクトを出す
			Math::Vector3 effectPos = m_pos;
			effectPos.y += 0.5f;	// 少し上（キャラの中心あたり）

			bool isFlip = false;

			if (m_lastDirType & DirType::Left)
			{
				effectPos.x -= 1.0f;	// 左前方
				isFlip = true;	// 左向きは反転
			}
			else
			{
				effectPos.x += 1.0f;	// 右前方
				isFlip = false;
			}

			effect->Setup(effectPos, isFlip);

			// シーンのオブジェクトリストに追加
			SceneManager::Instance().AddObject(effect);
		}
	}
	else
	{
		m_keyFlg = false;
	}

	//===================================================================
// 移動入力（攻撃中は受け付けない）
//===================================================================
	if (!m_isAttacking)
	{
		// 移動関係をクリア
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

		// キー入力があった && 向きが以前と変わっていればアニメーション変更
		if (m_dirType != 0 && m_dirType != oldDirType)
		{
			ChangeAnimation();
		}

		// ベクトルを正規化（長さを1にする）
		m_dir.Normalize();

		// 座標更新
		m_pos += m_dir * m_speed;
	}

	// ジャンプ処理
	if (GetAsyncKeyState(VK_SPACE) & 0x8000)
	{
		m_gravity = -0.1f;
	}

	// 重力を更新
	m_gravity += 0.005f;

	// 重力をキャラに反映
	m_pos.y -= m_gravity;

	//===================================================================
	// 向きに応じてテクスチャリストを切り替える
	// 左右キーが押されているときは最後の向きを更新する
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
		// 攻撃中：攻撃アニメーション（最後に向いていた方向）
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
		// 移動中：歩行アニメーション
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
		// 待機中：待機アニメーション
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

	// 最後のコマまで表示し終えたら
	if (animeCnt >= (int)m_pCurrentTextures->size())
	{
		if (m_isAttacking)
		{
			// 攻撃アニメが全コマ再生されたら攻撃終了
			m_isAttacking = false;
			m_animeInfo.count = 0;
			animeCnt = 0;
		}
		else
		{
			// 通常アニメはループ
			animeCnt = 0;
			m_animeInfo.count = 0;
		}
	}

	// 現在のコマのテクスチャをセット
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

	m_pDebugWire->AddDebugLine(
		rayInfo.m_pos,
		rayInfo.m_dir,
		rayInfo.m_range
	);

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

	m_pDebugWire->AddDebugSphere(sphere.m_sphere.Center, sphere.m_sphere.Radius);

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
		// 全方向への押し戻しを有効にする
		// ※方向ベクトルは絶対長さ1
		// 正規化(長さが1)
		hitDir.Normalize();

		// 押し戻し処理
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
	KdShaderManager::Instance().m_StandardShader.DrawPolygon(m_polygon, m_mWorld);
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

	// 板ポリにテクスチャをセット
	m_polygon.SetMaterial((*m_pCurrentTextures)[0]);

	// 板ポリの原点（真ん中下段）
	m_polygon.SetPivot(KdSquarePolygon::PivotType::Center_Bottom);

	// 1枚画像なので分割なし
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

	// 重力
	m_gravity = 0.0f;

	// 行列
	m_mWorld = Math::Matrix::Identity;

	m_keyFlg = false;
	m_isAttacking = false;

	// デバッグワイヤー生成
	m_pDebugWire = std::make_unique<KdDebugWireFrame>();
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
// 将来的にHPを減らす処理を追加する
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void Player::TakeDamage(int _damage)
{
	// 現時点では受けたことをログに表示するだけ
	// 将来的にHPを減らす処理を追加する
	KdDebugGUI::Instance().AddLog("Player TakeDamage:%d", _damage);
}