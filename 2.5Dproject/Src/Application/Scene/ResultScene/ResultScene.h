#pragma once

#include "../BaseScene/BaseScene.h"

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// リザルトシーン
// ・ゲームクリア後に表示される
// ・エンターキーでタイトルへ戻る
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
class ResultScene : public BaseScene
{
public:

	ResultScene() { Init(); }
	~ResultScene() override {}

private:

	void Event()      override;
	void Init()       override;
	void DrawSpriteScene() override;

	// リザルト背景画像
	std::shared_ptr<KdTexture> m_spResultBg;
};