#include "Torii.h"

#include "../../Scene/SceneManager.h"
#include "../Enemy/Enemy.h"

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 初期化
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void Torii::Init()
{
	//===================================================================
	// 鳥居モデルのロード
	//===================================================================
	m_spModel = std::make_shared<KdModelData>();
	m_spModel->Load("Asset/Models/Torii/torii.gltf");

	//===================================================================
	// ワームホール板ポリゴンの設定
	// 鳥居の中央に配置するので Center_Middle を使用
	//===================================================================
	m_polygon = std::make_shared<KdSquarePolygon>();
	m_polygon->SetMaterial("Asset/Textures/Effect/sprite-sheet1.png");
	m_polygon->SetPivot(KdSquarePolygon::PivotType::Center_Middle);
	m_polygon->SetSplit(5, 1);		// 横5コマ
	m_polygon->SetScale(3.0f);		// サイズ調整
	m_polygon->SetUVRect(0);

	// アニメーション初期値
	m_animeCnt = 0.0f;

	// 生成タイマー初期値
	// 最初はすぐに出現させる（SpawnInterval にすると最初の出現まで10秒待つ）
	m_spawnTimer = SpawnInterval;

	//===================================================================
	// HP・破壊演出の初期化
	//===================================================================
	m_hp = MaxHP;
	m_isBroken = false;
	m_dissolve = 0.0f;
	m_invincibleTimer = 0;

	//===================================================================
	// 当たり判定の登録
	// TypeDamage：プレイヤーの攻撃を受ける判定
	//===================================================================
	m_pCollider = std::make_unique<KdCollider>();
	m_pCollider->RegisterCollisionShape(
		"ToriiDamageCollision",
		Math::Vector3(0.0f, 1.0f, 0.0f),	// 中心（鳥居の中央あたり）
		1.5f,								// 半径（鳥居は大きいので広め）
		KdCollider::TypeDamage
	);

	// 揺れ演出初期化
	m_shakeTimer = 0;

	// 落下関連の初期化
	m_isFalling = false;
	m_fallY = 0.0f;
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 更新
// ・ワームホールアニメーションの更新
// ・一定間隔で敵を生成する
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void Torii::Update()
{
	//===================================================================
	// 演出中（必殺技など）はUpdateをスキップする
	//===================================================================
	if (SceneManager::Instance().IsCutScene())
	{
		return;
	}

	//===================================================================
	// 落下再出現中の処理
	// 空から落ちてきて、着地したら通常動作を開始する
	//===================================================================
	if (m_isFalling)
	{
		// 落下高さを減らす
		m_fallY -= FallSpeed;

		// 着地したら落下終了
		if (m_fallY <= 0.0f)
		{
			m_fallY = 0.0f;
			m_isFalling = false;
		}

		// 落下中は敵生成などをせず、ここで return
		// （描画は DrawLit で m_fallY を反映する）
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
	// 揺れタイマーの更新
	//===================================================================
	if (m_shakeTimer > 0)
	{
		m_shakeTimer--;
	}

	//===================================================================
	// 破壊状態の処理
	// ディゾルブを進めて、完全に消えたら消滅する
	//===================================================================
	if (m_isBroken)
	{
		m_dissolve += DissolveSpeed;

		if (m_dissolve >= 1.0f)
		{
			m_isExpired = true;
		}

		// 破壊中は敵を生成しないのでここでreturn
		return;
	}

	//===================================================================
	// ワームホールアニメーション更新
	//===================================================================
	m_animeCnt += AnimeSpeed;

	if (m_animeCnt >= (float)FrameCount)
	{
		m_animeCnt = 0.0f;
	}

	m_polygon->SetUVRect((int)m_animeCnt);

	//===================================================================
	// 敵の生成
	// 鳥居の破壊数に応じて出現数を増やし・間隔を短くする
	//===================================================================

	// 鳥居の破壊数を取得（強化は最大2まで）
	int breakCount = SceneManager::Instance().GetToriiBreakCount();
	if (breakCount > 2) { breakCount = 2; }	// 2で頭打ち

	// 破壊数に応じて最大出現数を増やす（破壊1つにつき +5体）
	int currentMaxSpawn = m_maxSpawnCount + breakCount * 5;

	// 破壊数に応じて出現間隔を短くする
	int currentInterval = SpawnInterval - breakCount * 40;
	if (currentInterval < 30) { currentInterval = 30; }

	// 現在シーンに存在するEnemyの数を数える
	int enemyCount = 0;
	for (auto& obj : SceneManager::Instance().GetObjList())
	{
		if (std::dynamic_pointer_cast<Enemy>(obj) != nullptr)
		{
			enemyCount++;
		}
	}

	// 最大数に達していなければタイマーを進める
	if (enemyCount < currentMaxSpawn)
	{
		m_spawnTimer++;

		// 生成間隔に達したら敵を生成
		if (m_spawnTimer >= currentInterval)
		{
			m_spawnTimer = 0;
			SpawnEnemy();
		}
	}

	//===================================================================
	// 発光タイマーの更新
	//===================================================================
	if (m_brightTimer > 0)
	{
		m_brightTimer--;
	}
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 描画
// ・鳥居モデルを描画する
// ・ワームホールを鳥居の中央に描画する
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void Torii::DrawLit()
{
	//===================================================================
	// 揺れの計算
	// 揺れタイマーが残っている間は左右に小刻みにずらす
	//===================================================================
	Math::Matrix drawMat = m_mWorld;

	if (m_isFalling)
	{
		Math::Matrix fallMat = Math::Matrix::CreateTranslation(
			Math::Vector3(0.0f, m_fallY, 0.0f)
		);
		drawMat = fallMat * m_mWorld;
	}

	if (m_shakeTimer > 0)
	{
		// タイマーの値で揺れ方向を交互に変える（プラス・マイナス）
		// フレームごとに左右に振れることで揺れて見える
		float shakeX = (m_shakeTimer % 2 == 0) ? ShakeStrength : -ShakeStrength;

		// 描画用の行列を横にずらす
		Math::Matrix shakeMat = Math::Matrix::CreateTranslation(
			Math::Vector3(shakeX, 0.0f, 0.0f)
		);
		drawMat = shakeMat * m_mWorld;
	}

	//===================================================================
	// 破壊中はディゾルブをかけて鳥居を描画する
	//===================================================================
	if (m_isBroken)
	{
		float         range = 0.3f;
		Math::Vector3 color = { 1.0f, 0.3f, 0.3f };
		KdShaderManager::Instance().m_StandardShader.SetDissolve(
			m_dissolve, &range, &color
		);
	}

	// 鳥居モデルの描画（揺れを反映した行列を使う）
	KdShaderManager::Instance().m_StandardShader.DrawModel(
		*m_spModel, drawMat
	);

	//===================================================================
	// ワームホールの描画
	// 落下中・破壊中は表示しない（着地後に表示）
	//===================================================================
	if (!m_isBroken && !m_isFalling)
	{
		// ワームホールは揺らさないので元の m_mWorld 基準で描画する
		Math::Matrix wormholeMat = Math::Matrix::CreateTranslation(
			m_mWorld.Translation() + Math::Vector3(0.0f, 0.8f, 0.0f)
		);
		KdShaderManager::Instance().m_StandardShader.DrawPolygon(
			*m_polygon, wormholeMat
		);
	}
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 発光描画
// 敵が出現するタイミングで鳥居を光らせる
// タイマーが残っている間だけ DrawBright に描画する
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void Torii::DrawBright()
{
	// 発光タイマーが0のときは描画しない
	if (m_brightTimer <= 0) { return; }

	// DrawBright に描画するだけで光って見える
	KdShaderManager::Instance().m_StandardShader.DrawModel(
		*m_spModel, m_mWorld
	);
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 敵を生成してシーンに追加する
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void Torii::SpawnEnemy()
{
	auto enemy = std::make_shared<Enemy>();
	enemy->Init();

	// 鳥居の座標に敵を生成する（Y座標は-0.25に固定）
	Math::Vector3 spawnPos = m_mWorld.Translation();
	spawnPos.y = -0.25f;
	enemy->SetPos(spawnPos);

	SceneManager::Instance().AddObject(enemy);

	// 敵が出現したら発光タイマーをセット
	m_brightTimer = BrightTime;
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// ダメージを受ける関数
// ・HPを減らして0になったら破壊状態にする
// ・無敵時間中は処理しない（連続ヒット防止）
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void Torii::TakeDamage(int _damage)
{
	if (m_invincibleTimer > 0) { return; }
	if (m_isBroken) { return; }

	m_hp -= _damage;
	m_invincibleTimer = InvincibleTime;

	// 被弾時の揺れをセット
	m_shakeTimer = ShakeTime;

	if (m_hp <= 0)
	{
		m_isBroken = true;
		m_pCollider->SetEnableAll(false);

		// 鳥居破壊数を加算（残りの鳥居が強化される）
		SceneManager::Instance().AddToriiBreakCount();
	}
}