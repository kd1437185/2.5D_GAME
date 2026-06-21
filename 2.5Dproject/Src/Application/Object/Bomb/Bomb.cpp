#include "Bomb.h"

#include "../../Scene/SceneManager.h"
#include "../Player/Player.h"

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 初期化
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void Bomb::Init()
{
	//===================================================================
	// 爆弾モデルのロード
	//===================================================================
	m_spModel = std::make_shared<KdModelData>();
	m_spModel->Load("Asset/Models/Bom/Bom.gltf");

	//===================================================================
	// 着弾予告（赤い丸）の設定
	// 地面に水平に置くので X軸90度回転して使う
	//===================================================================
	m_warningPolygon = std::make_shared<KdSquarePolygon>();
	m_warningPolygon->SetMaterial("Asset/Textures/Effect/redhole.png");
	m_warningPolygon->SetPivot(KdSquarePolygon::PivotType::Center_Middle);
	m_warningPolygon->SetSplit(1, 1);
	m_warningPolygon->SetScale(2.0f);

	//===================================================================
	// 爆発エフェクトの設定
	//===================================================================
	m_explodePolygon = std::make_shared<KdSquarePolygon>();
	m_explodePolygon->SetMaterial("Asset/Textures/Effect/sprite-sheet3.png");
	m_explodePolygon->SetPivot(KdSquarePolygon::PivotType::Center_Bottom);
	m_explodePolygon->SetSplit(7, 1);
	m_explodePolygon->SetScale(3.0f);
	m_explodePolygon->SetUVRect(0);

	//===================================================================
	// 初期状態
	//===================================================================
	m_state = State::Warning;

	// 爆弾の初期位置：着弾地点の真上（高い位置）
	m_pos = m_targetPos;
	m_pos.y = StartHeight;

	m_explodeAnimeCnt = 0.0f;
	m_isDamageGiven = false;
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 更新
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void Bomb::Update()
{
	switch (m_state)
	{
		//--------------------------------------
		// 着弾予告中：爆弾が落下する
		//--------------------------------------
	case State::Warning:
	{
		// 減速倍率を取得（Sキー減速中は0.5になる）
		float speedRate = SceneManager::Instance().GetEnemySpeedRate();

		// 爆弾を落下させる（減速倍率を反映）
		m_pos.y -= FallSpeed * speedRate;

		// 着弾地点の高さまで落ちたら爆発へ
		if (m_pos.y <= m_targetPos.y)
		{
			m_pos.y = m_targetPos.y;
			m_state = State::Explode;
			m_explodeAnimeCnt = 0.0f;
		}
		break;
	}

	//--------------------------------------
	// 爆発中：爆発エフェクトを再生
	//--------------------------------------
	case State::Explode:
	{
		//===================================================================
		// 爆発の瞬間（最初のフレーム）にプレイヤーへダメージ
		//===================================================================
		if (!m_isDamageGiven)
		{
			m_isDamageGiven = true;

			// プレイヤーを探して範囲内ならダメージ
			for (auto& obj : SceneManager::Instance().GetObjList())
			{
				std::shared_ptr<Player> player =
					std::dynamic_pointer_cast<Player>(obj);
				if (player == nullptr) { continue; }

				// 爆発中心とプレイヤーの距離を計算
				Math::Vector3 diff = player->GetPos() - m_targetPos;
				float dist = diff.Length();

				// 範囲内ならダメージ
				if (dist <= ExplodeRadius)
				{
					player->TakeDamage(ExplodeDamage);
				}
				break;
			}
		}

		// 爆発アニメーションを進める
		m_explodeAnimeCnt += ExplodeAnimeSpeed;

		// 全コマ再生したら消滅
		if ((int)m_explodeAnimeCnt >= ExplodeFrameCount)
		{
			m_isExpired = true;
		}
		break;
	}
	}

	// ワールド行列を更新
	m_mWorld = Math::Matrix::CreateTranslation(m_pos);
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 3D描画（爆弾モデル）
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void Bomb::DrawLit()
{
	// 着弾予告中だけ爆弾モデルを描画（落下中）
	if (m_state == State::Warning)
	{
		KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld);
	}
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// エフェクト描画（着弾予告・爆発）
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void Bomb::DrawEffect()
{
	if (m_state == State::Warning)
	{
		//===================================================================
		// 着弾予告（赤い丸）の点滅
		// 爆弾のY座標（高さ）を使って点滅の間隔を決める
		// 着弾が近い（低い）ほど速く点滅する
		//===================================================================

		// 着弾までの高さの割合（1.0：落下開始〜0.0：着弾）
		float heightRate = (m_pos.y - m_targetPos.y) / StartHeight;
		if (heightRate < 0.0f) { heightRate = 0.0f; }

		// 点滅の間隔（高いほど遅く、低いほど速く点滅）
		// heightRate が大きい（高い）ほど間隔が長い
		int blinkInterval = 3 + (int)(heightRate * 10);

		// 点滅判定（フレームカウントの代わりに整数化した高さを使う）
		// m_pos.y を10倍して整数化し、間隔で割った余りで表示・非表示を切り替える
		int phase = ((int)(m_pos.y * 10.0f) / blinkInterval) % 2;

		// 非表示のタイミングなら描画しない
		if (phase == 0)
		{
			// 地面に水平に置くため X軸-90度回転
			Math::Matrix rotMat = Math::Matrix::CreateRotationX(
				DirectX::XMConvertToRadians(-90)
			);
			Math::Matrix transMat = Math::Matrix::CreateTranslation(
				m_targetPos + Math::Vector3(0.0f, 0.25f, 0.0f)
			);
			Math::Matrix mat = rotMat * transMat;

			KdShaderManager::Instance().m_StandardShader.DrawPolygon(*m_warningPolygon, mat);
		}
	}
	else if (m_state == State::Explode)
	{
		//===================================================================
		// 爆発エフェクトを描画
		//===================================================================
		int x = (int)m_explodeAnimeCnt;

		// 最後のコマを超えないようにクランプ（0コマ目に戻るのを防ぐ）
		if (x >= ExplodeFrameCount) { x = ExplodeFrameCount - 1; }

		float w = 1.0f / ExplodeFrameCount;

		m_explodePolygon->SetUVRect(
			Math::Vector2(x * w, 0.0f),
			Math::Vector2((x + 1) * w, 1.0f)
		);

		Math::Matrix mat = Math::Matrix::CreateTranslation(m_targetPos);
		KdShaderManager::Instance().m_StandardShader.DrawPolygon(*m_explodePolygon, mat);
	}
}