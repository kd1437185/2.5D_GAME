#include "FadeManager.h"

#include "../../main.h"

void FadeManager::Init()
{
	m_spFusumaL = std::make_shared<KdTexture>("Asset/Textures/Fade/fusumaL.png");
	m_spFusumaR = std::make_shared<KdTexture>("Asset/Textures/Fade/fusumaR.png");

	// 初期は開いている状態
	m_rate = 0.0f;
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 更新
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void FadeManager::Update()
{
	switch (m_fadeState)
	{
		//--------------------------------------
		// フェードアウト（閉じていく）
		// rate: 0.0 → 1.0
		//--------------------------------------
	case FadeState::FadeOut:
	{
		m_rate += FadeSpeed;

		// 完全に閉じたら待機状態へ移行
		if (m_rate >= 1.0f)
		{
			m_rate = 1.0f;
			m_waitTimer = 0.0f;
			m_fadeState = FadeState::Wait;
		}
		break;
	}

	//--------------------------------------
	// 待機（閉じた状態で一定時間止まる）
	//--------------------------------------
	case FadeState::Wait:
	{
		// フレーム数を加算（1フレームにつき1）
		m_waitTimer += 1.0f;

		// 待機フレーム数が経過したらフェードアウト完了
		if (m_waitTimer >= WaitTime)
		{
			m_isFadeOutEnd = true;
			m_fadeState = FadeState::None;
		}
		break;
	}

	//--------------------------------------
	// フェードイン（開いていく）
	// rate: 1.0 → 0.0
	//--------------------------------------
	case FadeState::FadeIn:
	{
		m_rate -= FadeSpeed;

		// 完全に開いたら完了
		if (m_rate <= 0.0f)
		{
			m_rate = 0.0f;
			m_isFadeInEnd = true;
			m_fadeState = FadeState::None;
		}
		break;
	}

	case FadeState::None:
	default:
		break;
	}
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 描画
// 進行度 m_rate に応じてふすまの位置を計算する
// 座標系：画面中央が(0,0)・右が+X
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void FadeManager::DrawSprite()
{
	// 完全に開いている（rate=0）なら描画しない
	if (m_rate <= 0.0f) { return; }

	// 描画幅を少し広げて中央の隙間を埋める
	const int drawW = 660;

	//===================================================================
	// fusumaL（左のふすま）
	// rate=1.0（閉）：中心X = -320
	// rate=0.0（開）：中心X = -960
	//===================================================================
	float fusumaLCenterX = -320.0f - (640.0f * (1.0f - m_rate));

	KdShaderManager::Instance().m_spriteShader.DrawTex(
		m_spFusumaL.get(),
		(int)fusumaLCenterX,
		0,
		drawW,
		FusumaH,
		nullptr,
		&kWhiteColor,
		Math::Vector2(0.5f, 0.5f)
	);

	//===================================================================
	// fusumaR（右のふすま）
	// rate=1.0（閉）：中心X = +320
	// rate=0.0（開）：中心X = +960
	//===================================================================
	float fusumaRCenterX = 320.0f + (640.0f * (1.0f - m_rate));

	KdShaderManager::Instance().m_spriteShader.DrawTex(
		m_spFusumaR.get(),
		(int)fusumaRCenterX,
		0,
		drawW,
		FusumaH,
		nullptr,
		&kWhiteColor,
		Math::Vector2(0.5f, 0.5f)
	);
}