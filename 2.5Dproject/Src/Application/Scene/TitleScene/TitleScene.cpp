#include "TitleScene.h"

#include "../SceneManager.h"
#include "../../main.h"

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 初期化
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void TitleScene::Init()
{
	// カメラ生成
	m_camera = std::make_unique<KdCamera>();
	m_camera->SetProjectionMatrix(60);

	//===================================================================
	// 画像のロード
	//===================================================================
	m_spBg = std::make_shared<KdTexture>("Asset/Textures/UI/title.png");
	m_spTitleName = std::make_shared<KdTexture>("Asset/Textures/UI/titlename.png");
	m_spStart = std::make_shared<KdTexture>("Asset/Textures/UI/hajimeru.png");
	m_spExit = std::make_shared<KdTexture>("Asset/Textures/UI/owaru.png");

	//===================================================================
	// 選択の初期化
	//===================================================================
	m_select = 0;	// 最初は「はじめる」を選択
	m_blinkTimer = 0;
	m_keyFlg = false;
	// 決定関連の初期化
	m_decideTimer = 0;
	m_isDecided = false;
	m_decideSelect = 0;

	//===================================================================
	// タイトルBGM再生（ループ）
	//===================================================================
	m_bgm = KdAudioManager::Instance().Play("Asset/Sounds/隠れた社.wav", true);
	if (m_bgm) { m_bgm->SetVolume(BgmVolume); }
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// イベント（入力処理）
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void TitleScene::Event()
{
	//===================================================================
	// 点滅用カウンタを進める
	//===================================================================
	m_blinkTimer++;

	//===================================================================
	// 決定後の待機処理
	// 決定音を鳴らしてから一定時間待ってフェード（シーン遷移）へ
	//===================================================================
	if (m_isDecided)
	{
		m_decideTimer++;

		// 待機時間が経過したらシーン遷移
		if (m_decideTimer >= DecideWaitTime)
		{
			if (m_decideSelect == 0)
			{
				// はじめる → スコアリセットしてゲームへ
				SceneManager::Instance().ResetKillCount();
				SceneManager::Instance().ResetToriiBreakCount();

				// BGM停止
				if (m_bgm) { m_bgm->Stop(); }

				SceneManager::Instance().SetNextScene(SceneManager::SceneType::Game);
			}
			else
			{
				// おわる → ゲーム終了
				Application::Instance().End();
			}
		}

		// 決定後は他の入力を受け付けない
		return;
	}

	//===================================================================
	// 矢印キー上下で選択を切り替える
	//===================================================================
	bool upKey = (GetAsyncKeyState(VK_UP) & 0x8000) != 0;
	bool downKey = (GetAsyncKeyState(VK_DOWN) & 0x8000) != 0;

	if (upKey || downKey)
	{
		if (!m_keyFlg)
		{
			m_keyFlg = true;

			// 選択を切り替え
			int oldSelect = m_select;
			if (upKey) { m_select = 0; }
			if (downKey) { m_select = 1; }

			// 選択が変わったら切替SEを鳴らす
			if (m_select != oldSelect)
			{
				auto se = KdAudioManager::Instance().Play("Asset/Sounds/Hyoshigi03-1.wav", false);
				if (se) { se->SetVolume(SelectVolume); }
			}
		}
	}
	else
	{
		m_keyFlg = false;
	}

	//===================================================================
	// Zキーで決定
	//===================================================================
	if (GetAsyncKeyState('Z') & 0x8000)
	{
		// 決定音を鳴らす
		auto se = KdAudioManager::Instance().Play("Asset/Sounds/太鼓と鈴を用いた決定音.wav", false);
		if (se) { se->SetVolume(DecideVolume); }

		// 決定状態に移行（待機後にシーン遷移）
		m_isDecided = true;
		m_decideTimer = 0;
		m_decideSelect = m_select;
	}
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 2D描画
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void TitleScene::DrawSpriteScene()
{
	//===================================================================
	// 背景を画面全体に描画
	//===================================================================
	if (m_spBg)
	{
		KdShaderManager::Instance().m_spriteShader.DrawTex(
			m_spBg.get(),
			0, 0,
			1280, 720,
			nullptr, &kWhiteColor,
			Math::Vector2(0.5f, 0.5f)
		);
	}

	//===================================================================
	// タイトルロゴを画面上部に描画
	//===================================================================
	if (m_spTitleName)
	{
		// 拡大率（1.5倍）
		float scale = 1.5f;

		KdShaderManager::Instance().m_spriteShader.DrawTex(
			m_spTitleName.get(),
			0, 180,
			(int)(m_spTitleName->GetWidth() * scale),
			(int)(m_spTitleName->GetHeight() * scale),
			nullptr, &kWhiteColor,
			Math::Vector2(0.5f, 0.5f)
		);
	}

	//===================================================================
	// 点滅の判定
	// 選択中の項目だけ点滅させる
	// 15フレームごとに表示・非表示を切り替える
	//===================================================================
	bool blinkVisible = (m_blinkTimer / 15) % 2 == 0;

	//===================================================================
	// 「はじめる」を描画
	// 選択中（m_select==0）なら点滅、非選択なら常時表示
	//===================================================================
	if (m_spStart)
	{
		// 選択中で点滅の非表示タイミングなら描画しない
		bool draw = true;
		if (m_select == 0 && !blinkVisible) { draw = false; }

		if (draw)
		{
			KdShaderManager::Instance().m_spriteShader.DrawTex(
				m_spStart.get(),
				0, -80,	// 中央やや下
				m_spStart->GetWidth(),
				m_spStart->GetHeight(),
				nullptr, &kWhiteColor,
				Math::Vector2(0.5f, 0.5f)
			);
		}
	}

	//===================================================================
	// 「おわる」を描画
	// 選択中（m_select==1）なら点滅、非選択なら常時表示
	//===================================================================
	if (m_spExit)
	{
		bool draw = true;
		if (m_select == 1 && !blinkVisible) { draw = false; }

		if (draw)
		{
			KdShaderManager::Instance().m_spriteShader.DrawTex(
				m_spExit.get(),
				0, -200,	// はじめるの下
				m_spExit->GetWidth(),
				m_spExit->GetHeight(),
				nullptr, &kWhiteColor,
				Math::Vector2(0.5f, 0.5f)
			);
		}
	}
}