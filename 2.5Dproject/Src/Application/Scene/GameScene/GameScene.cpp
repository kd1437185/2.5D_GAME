#include "GameScene.h"
#include"../SceneManager.h"
#include "../../Object/BackGround/BackGround.h"
#include "../../Object/Ground/Ground.h"
#include "../../Object/Player/Player.h"
#include "../FadeManager/FadeManager.h"
#include "../../Object/Torii/Torii.h"
#include "../../Object/Torii/ToriiManager.h"
#include "../../Object/Bomb/BombManager.h"

void GameScene::Event()
{
	KdDebugGUI::Instance().ClearLog();

	//===================================================================
	// ゲーム状態ごとの処理
	//===================================================================
	switch (m_gameState)
	{
		//--------------------------------------
		// 開始演出中
		//--------------------------------------
	case GameState::Start:
	{
		m_stateTimer++;

		// 表示時間が経過したらプレイ中へ移行
		if (m_stateTimer >= StateDispTime)
		{
			m_gameState = GameState::Playing;
			m_stateTimer = 0;

			// 演出終了 → 敵などの動きを再開
			SceneManager::Instance().SetCutScene(false);

			// BGM再生開始（ループ）
			m_bgm = KdAudioManager::Instance().Play("Asset/Sounds/Samurai_Strain.wav", true);
			if (m_bgm) { m_bgm->SetVolume(0.2f); }
		}

		// 開始演出中はカメラだけ更新して以降の処理はスキップ
		// （ここでreturnせずカメラ処理へ進む）
		break;
	}

	//--------------------------------------
	// プレイ中
	//--------------------------------------
	case GameState::Playing:
	{
		//===================================================================
		// タイマーの更新
		//===================================================================
		if (m_timer > 0)
		{
			m_timer--;

			// 時間切れ＝ゲームクリア → 決着演出へ
			if (m_timer <= 0)
			{
				m_timer = 0;
				m_gameState = GameState::Finish;
				m_stateTimer = 0;

				// BGM停止
				if (m_bgm) { m_bgm->Stop(); }

				// 決着演出中は敵などを止める
				SceneManager::Instance().SetCutScene(true);
			}
		}

		//===================================================================
		// プレイヤー死亡チェック
		//===================================================================
		if (m_wpPlayer.expired() == false)
		{
			std::shared_ptr<Player> spPlayer = m_wpPlayer.lock();

			// 死亡演出が完了したら決着演出へ
			if (spPlayer->IsDeathAnimeEnd())
			{
				m_gameState = GameState::Finish;
				m_stateTimer = 0;

				if (m_bgm) { m_bgm->Stop(); }
				SceneManager::Instance().SetCutScene(true);
			}
		}
		break;
	}

	//--------------------------------------
	// 決着演出中
	//--------------------------------------
	case GameState::Finish:
	{
		m_stateTimer++;

		if (m_stateTimer >= StateDispTime)
		{
			// ※SetCutScene(false) は呼ばない
			//   フェード中も敵・プレイヤーを止めたままにして
			//   一瞬動くのを防ぐ

			SceneManager::Instance().SetNextScene(SceneManager::SceneType::Result);
		}
		break;
	}
	}

	//===================================================================
	// プレイヤーの情報を取得する（カメラ追従用）
	//===================================================================
	Math::Vector3 playerPos = {};
	bool          isZoom = false;

	if (m_wpPlayer.expired() == false)
	{
		std::shared_ptr<Player> spPlayer = m_wpPlayer.lock();
		playerPos = spPlayer->GetPos();
		isZoom = spPlayer->IsSpecial() || spPlayer->IsSlow();

		// プレイヤー座標をImGuiに表示
		KdDebugGUI::Instance().AddLog(
			"PlayerPos X:%.2f Y:%.2f Z:%.2f",
			playerPos.x,
			playerPos.y,
			playerPos.z
		);
	}

	//===================================================================
	// カメラの寄り具合を補間する
	//===================================================================
	float targetRate = isZoom ? 1.0f : 0.0f;
	m_camZoomRate += (targetRate - m_camZoomRate) * 0.1f;

	Math::Vector3 normalOffset = Math::Vector3(0.0f, 3.0f, -4.0f);
	Math::Vector3 zoomOffset = Math::Vector3(0.0f, 2.0f, -2.0f);

	Math::Vector3 camOffset = Math::Vector3::Lerp(
		normalOffset, zoomOffset, m_camZoomRate
	);

	Math::Matrix transMat = Math::Matrix::CreateTranslation(camOffset + playerPos);
	Math::Matrix rotMat = Math::Matrix::CreateRotationX(
		DirectX::XMConvertToRadians(35)
	);
	Math::Matrix mat = rotMat * transMat;
	m_camera->SetCameraMatrix(mat);
}

void GameScene::Init()
{

	// カメラ 生成＆視野角設定
	m_camera = std::make_unique<KdCamera>();	// 1 メモリ確保
	m_camera->SetProjectionMatrix(60);			// 2 視野角設定

	// 背景
	std::shared_ptr<BackGround> backGround;
	backGround = std::make_shared<BackGround>();	// 1 メモリ確保
	backGround->Init();								// 2 初期化
	m_objList.push_back(backGround);				// 3 リストへ追加

	// 地面
	std::shared_ptr<Ground> ground;
	ground = std::make_shared<Ground>();		// 1 メモリ確保
	ground->Init();								// 2 初期化
	m_objList.push_back(ground);				// 3 リストへ追加

	// プレイヤー
	std::shared_ptr<Player> player;
	player = std::make_shared<Player>();		// 1 メモリ確保
	player->Init();								// 2 初期化
	m_objList.push_back(player);				// 3 リストへ追加

	// プレイヤーの情報をシーン側が保持しておく
	m_wpPlayer = player;

	//===================================================================
	// 鳥居管理を配置（鳥居の初期配置・再出現を管理）
	//===================================================================
	std::shared_ptr<ToriiManager> toriiManager;
	toriiManager = std::make_shared<ToriiManager>();
	toriiManager->Init();
	m_objList.push_back(toriiManager);

	//===================================================================
	// 爆弾管理を配置（一定間隔で爆弾を降らせる）
	//===================================================================
	std::shared_ptr<BombManager> bombManager;
	bombManager = std::make_shared<BombManager>();
	bombManager->Init();
	m_objList.push_back(bombManager);

	//===================================================================
	// 環境光の設定
	// 画面全体の明るさを調整する（デフォルトは0.3）
	// 値を上げると暗い部分が明るくなる
	//===================================================================
	KdShaderManager::Instance().WriteCBAmbientLight(
		Math::Vector4(0.6f, 0.6f, 0.6f, 1.0f)
	);

	//===================================================================
	// 平行光の設定
	// 光の方向と色（明るさ）を設定する
	//===================================================================
	KdShaderManager::Instance().WriteCBDirectionalLight(
		Math::Vector3(1.0f, -1.0f, 1.0f),	// 光の方向
		Math::Vector3(3.0f, 3.0f, 3.0f)		// 光の色（明るさ・デフォルトは2.25）
	);

	//===================================================================
	// タイマー初期化（2分 = 7200フレーム）
	//===================================================================
	m_timer = 7200;

	// 数字画像のロード
	m_spNumberTex = std::make_shared<KdTexture>("Asset/Textures/UI/number.png");

	//===================================================================
	// ゲーム状態の初期化
	// 開始演出からスタート
	//===================================================================
	m_gameState = GameState::Start;
	m_stateTimer = 0;

	// 開始・決着画像のロード
	m_spStartImg = std::make_shared<KdTexture>("Asset/Textures/UI/kaishi.png");
	m_spFinishImg = std::make_shared<KdTexture>("Asset/Textures/UI/kettyaku.png");

	// 開始演出中は敵などを止める
	SceneManager::Instance().SetCutScene(true);

}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 2D描画
// タイマーを画面上中央に数字画像で表示する
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void GameScene::DrawSpriteScene()
{
	//===================================================================
	// タイマー表示（プレイ中のみ）
	//===================================================================
	if (m_gameState == GameState::Playing && m_spNumberTex)
	{
		//===================================================================
		// 残りフレーム数を分:秒に変換する
		//===================================================================
		int totalSeconds = m_timer / 60;	// フレーム→秒
		int minutes = totalSeconds / 60;	// 分
		int seconds = totalSeconds % 60;	// 秒

		//===================================================================
		// 表示する数字を並べる
		// 形式：M:SS（例 2:00, 1:59, 0:05）
		// スプライトシートのコマ番号：0〜9はそのまま、コロンは10
		//===================================================================

		// 表示するコマ番号の配列を作る
		// 分1桁・コロン・秒2桁 の4文字
		int digits[4];
		digits[0] = minutes;			// 分（1桁）
		digits[1] = 10;					// コロン（コマ10）
		digits[2] = seconds / 10;		// 秒の十の位
		digits[3] = seconds % 10;		// 秒の一の位

		//===================================================================
		// 数字を1コマずつ描画する
		//===================================================================

		// 1コマのサイズ（スプライトシートの幅÷11）
		int numW = m_spNumberTex->GetWidth() / 11;
		int numH = m_spNumberTex->GetHeight();

		// 表示サイズ（拡大率）
		float scale = 1.0f;
		int   drawW = (int)(numW * scale);
		int   drawH = (int)(numH * scale);

		// 4文字分の合計幅
		int totalW = drawW * 4;

		// 画面上中央に配置
		// 中央原点なので、左端 = -合計幅/2、上 = 300あたり
		int startX = -totalW / 2;
		int posY = 300;

		// 1文字ずつ描画
		for (int i = 0; i < 4; ++i)
		{
			// このコマのUV範囲を切り取る
			Math::Rectangle srcRect;
			srcRect.x = digits[i] * numW;
			srcRect.y = 0;
			srcRect.width = numW;
			srcRect.height = numH;

			// X座標（左から順に並べる・中心基準なので半分ずらす）
			int posX = startX + drawW * i + drawW / 2;

			KdShaderManager::Instance().m_spriteShader.DrawTex(
				m_spNumberTex.get(),
				posX,
				posY,
				drawW,
				drawH,
				&srcRect,
				&kWhiteColor,
				Math::Vector2(0.5f, 0.5f)
			);
		}
	}

	//===================================================================
	// 開始演出中：「開始」画像を中央に表示
	//===================================================================
	if (m_gameState == GameState::Start && m_spStartImg)
	{
		KdShaderManager::Instance().m_spriteShader.DrawTex(
			m_spStartImg.get(),
			0, 0,
			m_spStartImg->GetWidth() * 2,	// 2倍
			m_spStartImg->GetHeight() * 2,	// 2倍
			nullptr, &kWhiteColor,
			Math::Vector2(0.5f, 0.5f)
		);
	}

	//===================================================================
	// 決着演出中：「決着」画像を中央に表示
	//===================================================================
	if (m_gameState == GameState::Finish && m_spFinishImg)
	{
		KdShaderManager::Instance().m_spriteShader.DrawTex(
			m_spFinishImg.get(),
			0, 0,
			m_spFinishImg->GetWidth() * 2,	// 2倍
			m_spFinishImg->GetHeight() * 2,	// 2倍
			nullptr, &kWhiteColor,
			Math::Vector2(0.5f, 0.5f)
		);
	}
}