#include "GameScene.h"
#include"../SceneManager.h"
#include "../../Object/BackGround/BackGround.h"
#include "../../Object/Ground/Ground.h"
#include "../../Object/Player/Player.h"
#include "../FadeManager/FadeManager.h"
#include "../../Object/Torii/Torii.h"
#include "../../Object/Bomb/BombManager.h"

void GameScene::Event()
{
	//===================================================================
	// デバッグ情報の表示
	// 毎フレームクリアしてから最新情報を1行で表示する
	//===================================================================
	KdDebugGUI::Instance().ClearLog();

	//===================================================================
	// タイマーの更新
	// 毎フレーム減少し、0になったらゲームクリア（リザルトへ）
	//===================================================================
	if (m_timer > 0)
	{
		m_timer--;

		// 時間切れ＝ゲームクリア
		if (m_timer <= 0)
		{
			m_timer = 0;

			// BGM停止
			if (m_bgm) { m_bgm->Stop(); }

			// 鳥居破壊数リセット
			SceneManager::Instance().ResetToriiBreakCount();

			// リザルトシーンへ遷移
			SceneManager::Instance().SetNextScene(SceneManager::SceneType::Result);
		}
	}

	//===================================================================
	// デバッグ用：エンターキーでタイトルシーンに遷移
	//===================================================================
	if (GetAsyncKeyState(VK_RETURN) & 0x8000)
	{
		// タイトルへ戻る前にBGMを停止
		if (m_bgm)
		{
			m_bgm->Stop();
		}

		// 鳥居破壊数をリセット
		SceneManager::Instance().ResetToriiBreakCount();

		SceneManager::Instance().SetNextScene(SceneManager::SceneType::Result);
	}

	//===================================================================
	// プレイヤーの情報を取得する
	//===================================================================
	Math::Vector3 playerPos = {};
	bool          isZoom = false;	// カメラを寄せるか

	if (m_wpPlayer.expired() == false)
	{
		std::shared_ptr<Player> spPlayer = m_wpPlayer.lock();

		playerPos = spPlayer->GetPos();

		// 必殺技中 または 減速発動アニメ中はカメラを寄せる
		isZoom = spPlayer->IsSpecial() || spPlayer->IsSlow();

		KdDebugGUI::Instance().AddLog(
			"PlayerPos X:%.2f Y:%.2f Z:%.2f",
			playerPos.x,
			playerPos.y,
			playerPos.z
		);
	}

	//===================================================================
	// カメラの寄り具合を補間する
	// 必殺技中は 1.0 に・通常時は 0.0 に徐々に近づける
	//===================================================================
	float targetRate = isZoom ? 1.0f : 0.0f;

	// 線形補間で滑らかに変化させる
	m_camZoomRate += (targetRate - m_camZoomRate) * 0.1f;

	//===================================================================
	// 寄り具合に応じてカメラの距離を変える
	// 通常時のオフセット → 寄り時のオフセット へ補間する
	//===================================================================
	Math::Vector3 normalOffset = Math::Vector3(0.0f, 3.0f, -4.0f);	// 通常
	Math::Vector3 zoomOffset = Math::Vector3(0.0f, 2.0f, -2.0f);	// 寄り

	// 補間したオフセットを計算
	Math::Vector3 camOffset = Math::Vector3::Lerp(
		normalOffset, zoomOffset, m_camZoomRate
	);

	// カメラの座標行列を作成
	Math::Matrix transMat = Math::Matrix::CreateTranslation(camOffset + playerPos);

	// カメラの回転行列を作成（少し下を向かせる）
	Math::Matrix rotMat = Math::Matrix::CreateRotationX(
		DirectX::XMConvertToRadians(35)
	);

	// 行列を合成（回転 * 座標）
	Math::Matrix mat = rotMat * transMat;

	// カメラに行列をセット
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
	// 鳥居を3つ配置する
	//===================================================================
	std::shared_ptr<Torii> torii;

	// 鳥居1
	torii = std::make_shared<Torii>();
	torii->Init();
	torii->SetPos(Math::Vector3(-10.0f, 0.0f, 10.0f));
	torii->SetMaxSpawnCount(20);		// 最大20体
	m_objList.push_back(torii);

	// 鳥居2
	torii = std::make_shared<Torii>();
	torii->Init();
	torii->SetPos(Math::Vector3(0.0f, 0.0f, 10.0f));
	torii->SetMaxSpawnCount(20);
	m_objList.push_back(torii);

	// 鳥居3
	torii = std::make_shared<Torii>();
	torii->Init();
	torii->SetPos(Math::Vector3(10.0f, 0.0f, 10.0f));
	torii->SetMaxSpawnCount(20);
	m_objList.push_back(torii);

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
	// BGM再生（ループ）
	//===================================================================
	m_bgm = KdAudioManager::Instance().Play("Asset/Sounds/Samurai_Strain.wav", true);
	if (m_bgm)
	{
		m_bgm->SetVolume(0.2f);
	}

	//===================================================================
	// タイマー初期化（2分 = 7200フレーム）
	//===================================================================
	m_timer = 7200;

	// 数字画像のロード
	m_spNumberTex = std::make_shared<KdTexture>("Asset/Textures/UI/number.png");

}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 2D描画
// タイマーを画面上中央に数字画像で表示する
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void GameScene::DrawSpriteScene()
{
	if (!m_spNumberTex) { return; }

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