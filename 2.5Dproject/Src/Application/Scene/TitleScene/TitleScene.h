#pragma once

#include "../BaseScene/BaseScene.h"

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// タイトルシーン
// ・背景・タイトルロゴを表示
// ・矢印キー上下で「はじめる」「おわる」を選択（選択中は点滅）
// ・Zキーで決定（はじめる→ゲーム、おわる→終了）
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
class TitleScene : public BaseScene
{
public:

	TitleScene() { Init(); }
	~TitleScene() override {}

private:

	void Event()           override;
	void Init()            override;
	void DrawSpriteScene() override;

	//===================================================================
	// 画像
	//===================================================================
	std::shared_ptr<KdTexture> m_spBg;			// 背景
	std::shared_ptr<KdTexture> m_spTitleName;	// タイトルロゴ
	std::shared_ptr<KdTexture> m_spStart;		// はじめる
	std::shared_ptr<KdTexture> m_spExit;		// おわる

	//===================================================================
	// 選択管理
	//===================================================================

	// 選択中の項目
	// 0：はじめる
	// 1：おわる
	int m_select = 0;

	// 点滅用のカウンタ
	int m_blinkTimer = 0;

	// キー連続入力防止フラグ
	bool m_keyFlg = false;

	//===================================================================
	// サウンド関連
	//===================================================================

	// タイトルBGMのインスタンス（停止用に保持）
	std::shared_ptr<KdSoundInstance> m_bgm;

	// 決定後の待機タイマー
	// 決定音を鳴らしてから少し待ってフェードへ
	int m_decideTimer = 0;

	// 決定待機時間（フレーム数）
	static constexpr int DecideWaitTime = 40;

	// 決定したかフラグ（待機中の多重入力防止）
	bool m_isDecided = false;

	// 決定した項目（0：はじめる、1：おわる）
	int m_decideSelect = 0;

	//===================================================================
	// 音量設定（後で調整しやすいように定数化）
	//===================================================================
	static constexpr float BgmVolume = 0.2f;	// BGM音量
	static constexpr float SelectVolume = 1.0f;	// 選択切替SE音量
	static constexpr float DecideVolume = 1.0f;	// 決定音音量
};