#pragma once

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 鳥居クラス
// ・鳥居モデルを描画する
// ・ワームホールアニメーションを表示する
// ・一定間隔で敵を生成する
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
class Torii : public KdGameObject
{
public:

	Torii() {}
	~Torii() override {}

	void Init()			override;
	void Update()		override;
	void DrawLit()		override;
	void DrawBright()	override;

	//===================================================================
	// 最大出現数を設定する
	// GameScene::Init() から呼んで調整できるようにする
	//===================================================================
	void SetMaxSpawnCount(int _max) { m_maxSpawnCount = _max; }

	// ダメージを受ける関数
	// AttackEffect から呼ばれる
	void TakeDamage(int _damage);

private:

	// 敵を生成してシーンに追加する
	void SpawnEnemy();

	//===================================================================
	// 鳥居モデル
	//===================================================================
	std::shared_ptr<KdModelData> m_spModel;

	//===================================================================
	// ワームホールアニメーション
	//===================================================================

	// 板ポリゴン
	std::shared_ptr<KdSquarePolygon> m_polygon;

	// アニメーションカウント
	float m_animeCnt = 0.0f;

	// アニメーション速度
	static constexpr float AnimeSpeed = 0.1f;

	// 総コマ数（横5コマ）
	static constexpr int FrameCount = 5;

	//===================================================================
	// 敵の生成関連
	//===================================================================

	// 生成タイマー（フレーム数）
	int m_spawnTimer = 0;

	// 生成間隔（フレーム数）
	// 60fps × 10秒 = 600フレーム
	static constexpr int SpawnInterval = 120;

	// 最大生成数（外部から変更可能）
	int m_maxSpawnCount = 20;

	//===================================================================
	// 発光関連
	// 敵が出現するタイミングで鳥居を光らせる
	//===================================================================

	// 発光タイマー（フレーム数）
	int m_brightTimer = 0;

	// 発光時間（フレーム数）
	static constexpr int BrightTime = 60;

	// 発光の強さ
	static constexpr float BrightIntensity = 3.0f;

	//===================================================================
	// HP・破壊演出関連
	//===================================================================

	// 現在のHP
	int m_hp = 5;

	// 最大HP（後で調整可能）
	static constexpr int MaxHP = 5;

	// 破壊状態フラグ
	// true になったら敵を出さなくなりディゾルブで消える
	bool m_isBroken = false;

	// ディゾルブの進行度（0.0〜1.0）
	float m_dissolve = 0.0f;

	// ディゾルブの速度
	static constexpr float DissolveSpeed = 0.02f;

	// 無敵タイマー（連続ヒット防止）
	int m_invincibleTimer = 0;

	// 無敵時間（フレーム数）
	static constexpr int InvincibleTime = 10;

	//===================================================================
	// 被弾時の揺れ演出関連
	//===================================================================

	// 揺れタイマー（フレーム数）
	// 0より大きい間は揺れる
	int m_shakeTimer = 0;

	// 揺れ時間（フレーム数）
	static constexpr int ShakeTime = 15;

	// 揺れの強さ（横方向のずれ幅）
	static constexpr float ShakeStrength = 0.15f;
};