#pragma once

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// フェードマネージャークラス
// ・ふすま画像を左右からスライドさせてシーン遷移を演出する
// ・進行度 m_rate（0.0〜1.0）でフェードを管理する
//   0.0 = 完全に開いている（ふすま画面外）
//   1.0 = 完全に閉じている（ふすま画面中央）
//
// 【フェードの流れ】
//   FadeOut() で閉じる → 閉じ切ったら1秒待機 → IsFadeOutEnd() が true
//   FadeIn() で開く → 開き切ったら IsFadeInEnd() が true
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
class FadeManager
{
public:

	// フェードの状態
	enum class FadeState
	{
		None,		// フェードなし（通常状態）
		FadeOut,	// 閉じていく（rate: 0→1）
		Wait,		// 閉じた状態で待機
		FadeIn,		// 開いていく（rate: 1→0）
	};

	void Update();
	void DrawSprite();

	// フェードアウト開始
	void FadeOut()
	{
		m_fadeState = FadeState::FadeOut;
		m_isFadeOutEnd = false;
		m_waitTimer = 0.0f;

		// ふすまが閉じる音
		auto se = KdAudioManager::Instance().Play("Asset/Sounds/和太鼓.wav", false);
		if (se) { se->SetVolume(1.0f); }
	}

	// フェードイン開始
	void FadeIn()
	{
		m_fadeState = FadeState::FadeIn;
		m_isFadeInEnd = false;

		
	}

	// フェードアウト（待機含む）が完了したか
	bool IsFadeOutEnd() const { return m_isFadeOutEnd; }

	// フェードインが完了したか
	bool IsFadeInEnd()  const { return m_isFadeInEnd; }

	// フェード中かどうか
	bool IsFading() const { return m_fadeState != FadeState::None; }

	// デバッグ用
	float GetRate() const { return m_rate; }

private:

	void Init();

	// フェードの現在の状態
	FadeState m_fadeState = FadeState::None;

	// 画面サイズ
	static constexpr int ScreenW = 1280;
	static constexpr int ScreenH = 720;
	static constexpr int FusumaW = 640;	// 画面半分
	static constexpr int FusumaH = 720;

	// フェードの進行度（0.0〜1.0）
	float m_rate = 0.0f;

	// フェード速度（1フレームあたりの進行度）
	static constexpr float FadeSpeed = 0.03f;

	// 閉じ切った後の待機フレーム数（60fpsなら60で1秒）
	static constexpr float WaitTime = 30.0f;

	// 待機用カウンタ（フレーム数を加算していく）
	float m_waitTimer = 0.0f;

	bool m_isFadeOutEnd = false;
	bool m_isFadeInEnd = false;

	std::shared_ptr<KdTexture> m_spFusumaL;
	std::shared_ptr<KdTexture> m_spFusumaR;

	//=====================================================
	// シングルトンパターン
	//=====================================================
private:
	FadeManager() { Init(); }

public:
	static FadeManager& Instance()
	{
		static FadeManager instance;
		return instance;
	}
};