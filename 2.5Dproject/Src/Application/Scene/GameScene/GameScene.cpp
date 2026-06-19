#include "GameScene.h"
#include"../SceneManager.h"
#include "../../Object/BackGround/BackGround.h"
#include "../../Object/Ground/Ground.h"
#include "../../Object/Player/Player.h"
#include "../FadeManager/FadeManager.h"
#include "../../Object/Torii/Torii.h"

void GameScene::Event()
{
	//===================================================================
	// デバッグ情報の表示
	// 毎フレームクリアしてから最新情報を1行で表示する
	//===================================================================
	KdDebugGUI::Instance().ClearLog();

	//===================================================================
	// デバッグ用：エンターキーでタイトルシーンに遷移
	//===================================================================
	if (GetAsyncKeyState(VK_RETURN) & 0x8000)
	{
		SceneManager::Instance().SetNextScene(SceneManager::SceneType::Title);
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
	Math::Vector3 normalOffset = Math::Vector3(0.0f, 4.0f, -5.0f);	// 通常
	Math::Vector3 zoomOffset = Math::Vector3(0.0f, 2.0f, -3.0f);	// 寄り

	// 補間したオフセットを計算
	Math::Vector3 camOffset = Math::Vector3::Lerp(
		normalOffset, zoomOffset, m_camZoomRate
	);

	// カメラの座標行列を作成
	Math::Matrix transMat = Math::Matrix::CreateTranslation(camOffset + playerPos);

	// カメラの回転行列を作成（少し下を向かせる）
	Math::Matrix rotMat = Math::Matrix::CreateRotationX(
		DirectX::XMConvertToRadians(30)
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

}
