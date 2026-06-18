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
	// 現在シーンに存在するEnemyの数を数えて
	// 最大数に達していない場合のみ生成タイマーを進める
	//===================================================================

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
	if (enemyCount < m_maxSpawnCount)
	{
		m_spawnTimer++;

		// 生成間隔に達したら敵を生成
		if (m_spawnTimer >= SpawnInterval)
		{
			m_spawnTimer = 0;
			SpawnEnemy();
		}
	}

	// 発光タイマーの更新
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
	// 鳥居モデルの描画
	KdShaderManager::Instance().m_StandardShader.DrawModel(
		*m_spModel, m_mWorld
	);

	// ワームホールの描画
	// 鳥居の中央に配置するため Y 軸にオフセットをかける
	Math::Matrix wormholeMat = Math::Matrix::CreateTranslation(
		m_mWorld.Translation() + Math::Vector3(0.0f, 0.8f, 0.0f)
	);
	KdShaderManager::Instance().m_StandardShader.DrawPolygon(
		*m_polygon, wormholeMat
	);
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