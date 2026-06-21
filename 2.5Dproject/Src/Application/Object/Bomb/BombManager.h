#pragma once

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 爆弾管理クラス
// ・一定間隔で爆弾をプレイヤーの位置に降らせる
// ・鳥居の破壊数に応じて間隔が短くなる
// ・見た目は持たない（生成役に徹する）
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
class BombManager : public KdGameObject
{
public:

	BombManager() {}
	~BombManager() override {}

	void Init()   override;
	void Update() override;

private:

	// 爆弾を生成する
	void SpawnBomb();

	// 生成タイマー（フレーム数）
	int m_spawnTimer = 0;

	// 基本の生成間隔（フレーム数）
	// 全鳥居が生きている間はこの間隔
	// 60fps × 8秒 = 480フレーム
	static constexpr int BaseInterval = 480;
};