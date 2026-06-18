#pragma once

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 必殺技エフェクトクラス
// ・画面全体に斬撃エフェクトを描画する
// ・斬撃の後にフラッシュ演出を行う
// ・演出が終わったら消滅する
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
class SpecialEffect : public KdGameObject
{
public:

	SpecialEffect() {}
	~SpecialEffect() override {}

	void Init()       override;
	void Update()     override;
	void DrawBright() override;

private:

	// 斬撃エフェクトのテクスチャ
	std::shared_ptr<KdTexture> m_spTexSlash;

	// 演出の経過フレーム
	int m_timer = 0;

	// 斬撃アニメーションカウント
	float m_slashAnimeCnt = 0.0f;

	// 斬撃の総コマ数（横5コマ）
	static constexpr int SlashFrameCount = 5;

	// 斬撃アニメーション速度
	static constexpr float SlashAnimeSpeed = 0.3f;

	// フラッシュの強さ（0.0〜1.0）
	float m_flashAlpha = 0.0f;

	// 画面サイズ
	static constexpr int ScreenW = 1280;
	static constexpr int ScreenH = 720;

	// 演出の各タイミング（フレーム数）
	static constexpr int SlashEndFrame = 30;	// 斬撃終了
	static constexpr int FlashFrame = 35;	// フラッシュ開始
	static constexpr int EffectEndFrame = 60;	// 演出終了

	// ダメージを与えたかフラグ（1回だけダメージを与える）
	bool m_isDamageGiven = false;

	// 必殺技のダメージ量（大ダメージ）
	static constexpr int SpecialDamage = 10;
};