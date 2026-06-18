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
	// ログが横に伸びないように ClearLog() を毎フレーム呼ぶ
	//===================================================================
	KdDebugGUI::Instance().ClearLog();

	if (m_wpPlayer.expired() == false)
	{
		Math::Vector3 playerPos = m_wpPlayer.lock()->GetPos();

		KdDebugGUI::Instance().AddLog(
			"PlayerPos X:%.2f Y:%.2f Z:%.2f",
			playerPos.x,
			playerPos.y,
			playerPos.z
		);
	}

	//===================================================================
	// デバッグ用：エンターキーでタイトルシーンに遷移
	//===================================================================
	if (GetAsyncKeyState(VK_RETURN) & 0x8000)
	{
		SceneManager::Instance().SetNextScene(SceneManager::SceneType::Title);
	}

	// プレイヤーの最新の座標を取得する
	Math::Vector3 playerPos = {};

	// weak_ptrで所持している m_player が有効かどうかを調べる
	// expired() … 既に無効なアドレスならtrueを返す
	if (m_wpPlayer.expired() == false)
	{
		// weak_ptrを lock() で shared_ptr として取得
		std::shared_ptr<Player> spPlayer = m_wpPlayer.lock();

		// shared_ptrならアドレスの先にアクセス可能
		playerPos = spPlayer->GetPos();
	}

	// カメラの座標行列を作成
	Math::Matrix transMat;
	transMat = Math::Matrix::CreateTranslation
	(Math::Vector3(0.0f, 4.0f, -5.0f) + playerPos);

	// カメラの回転行列を作成
	// 少し下を向かせる
	Math::Matrix rotMat;
	rotMat = Math::Matrix::CreateRotationX
	(DirectX::XMConvertToRadians(30));

	// 行列を合成 (拡縮 * 回転 * 座標)
	Math::Matrix mat = rotMat * transMat;

	// カメラに行列をセット
	// この時点では画面には反映されない
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
	torii->SetMaxSpawnCount(5);		// 最大5体
	m_objList.push_back(torii);

	// 鳥居2
	torii = std::make_shared<Torii>();
	torii->Init();
	torii->SetPos(Math::Vector3(0.0f, 0.0f, 10.0f));
	torii->SetMaxSpawnCount(5);
	m_objList.push_back(torii);

	// 鳥居3
	torii = std::make_shared<Torii>();
	torii->Init();
	torii->SetPos(Math::Vector3(10.0f, 0.0f, 10.0f));
	torii->SetMaxSpawnCount(5);
	m_objList.push_back(torii);

}
