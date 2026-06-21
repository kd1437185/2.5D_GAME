#pragma once

#include"../BaseScene/BaseScene.h"

// 前方宣言
class Player;
class Torii;

class GameScene : public BaseScene
{
public :

	GameScene()  { Init(); }
	~GameScene() {}

private:

	void Event() override;
	void Init()  override;
	void DrawSpriteScene() override;

	// プレイヤーの情報
	std::weak_ptr<Player> m_wpPlayer;

	// スマートポインタの種類は３つ

	// shared_ptr … 複数のポインタでアドレスを所有できる
	// 　　　　　　　参照カウントが増減する、常にアクセス可能

	// weak_ptr　 … shared_ptrのアドレスを保持できるが、アクセスする権利は無い
	// 　　　　　　　参照カウントが増減しない

	// unique_ptr … １つのポインタでしかアドレスを所有できない

	// カメラの寄り具合（0.0：通常 〜 1.0：寄り）
	// 必殺技中は1.0に近づき、通常時は0.0に近づく
	float m_camZoomRate = 0.0f;

	// BGMのサウンドインスタンス
	// シーンが保持して、シーン終了時に停止する
	std::shared_ptr<KdSoundInstance> m_bgm;

	//===================================================================
	// タイマー関連
	//===================================================================

	// 残り時間（フレーム数）
	// 60fps × 120秒 = 7200フレーム = 2分
	int m_timer = 7200;

	// 数字スプライトシート画像（0123456789: の11コマ）
	std::shared_ptr<KdTexture> m_spNumberTex;


};
