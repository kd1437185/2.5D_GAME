#pragma once

#include "../BaseScene/BaseScene.h"

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// リザルトシーン
// ・討伐数 → 鳥居破壊数 → 結果スコア の順に表示する
// ・各表示時に効果音を鳴らす
// ・結果スコア＝討伐数 × 100 × 鳥居破壊数
// ・エンターキーでタイトルへ戻る
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
class ResultScene : public BaseScene
{
public:

	ResultScene() { Init(); }
	~ResultScene() override {}

private:

	void Event()          override;
	void Update();
	void Init()           override;
	void DrawSpriteScene() override;

	// 数字を描画するヘルパー関数
	// ・_value … 表示する数値
	// ・_x, _y … 中心座標
	// ・_scale … 拡大率
	void DrawNumber(int _value, int _x, int _y, float _scale);

	//===================================================================
	// 画像
	//===================================================================
	std::shared_ptr<KdTexture> m_spNumberTex;	// 数字（0123456789:）
	std::shared_ptr<KdTexture> m_spLabelKill;	// 討伐ラベル
	std::shared_ptr<KdTexture> m_spLabelTorii;	// 鳥居ラベル
	std::shared_ptr<KdTexture> m_spLabelResult;	// 結果ラベル
	std::shared_ptr<KdTexture> m_spResultBg;	// リザルト背景

	//===================================================================
	// スコアデータ
	//===================================================================
	int m_killCount = 0;	// 討伐数
	int m_toriiCount = 0;	// 鳥居破壊数
	int m_resultScore = 0;	// 結果スコア

	//===================================================================
	// 段階表示の管理
	//===================================================================

	// 表示の進行段階
	// 0：何も出ていない
	// 1：討伐数を表示
	// 2：鳥居破壊数を表示
	// 3：結果スコアを表示
	int m_phase = 0;

	// 段階を進めるタイマー（フレーム数）
	int m_phaseTimer = 0;

	// 段階を進める間隔（フレーム数）
	static constexpr int PhaseInterval = 60;	// 約1秒
};