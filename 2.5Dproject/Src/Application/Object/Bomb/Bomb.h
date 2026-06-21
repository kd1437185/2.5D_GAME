#pragma once

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 爆弾クラス
// ・着弾予告（赤い丸）を表示
// ・空から球体が落下
// ・着地で爆発エフェクト＆プレイヤーにダメージ
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
class Bomb : public KdGameObject
{
public:

	Bomb() {}
	~Bomb() override {}

	void Init()       override;
	void Update()     override;
	void DrawLit()    override;
	void DrawEffect() override;

	// 着弾地点を設定する（プレイヤーの位置）
	void SetTargetPos(const Math::Vector3& _pos) { m_targetPos = _pos; }

private:

	//===================================================================
	// 爆弾の状態
	//===================================================================
	enum class State
	{
		Warning,	// 着弾予告中（赤い丸表示・爆弾が落下）
		Explode,	// 爆発中（爆発エフェクト再生）
	};

	State m_state = State::Warning;

	//===================================================================
	// 爆弾モデル（球体）
	//===================================================================
	std::shared_ptr<KdModelData> m_spModel;

	//===================================================================
	// 着弾予告の赤い丸
	//===================================================================
	std::shared_ptr<KdSquarePolygon> m_warningPolygon;

	//===================================================================
	// 爆発エフェクト
	//===================================================================
	std::shared_ptr<KdSquarePolygon> m_explodePolygon;

	// 爆発アニメーションカウント
	float m_explodeAnimeCnt = 0.0f;

	// 爆発アニメーションの総コマ数（横7コマ）
	static constexpr int ExplodeFrameCount = 7;

	// 爆発アニメーションの速度
	static constexpr float ExplodeAnimeSpeed = 0.2f;

	//===================================================================
	// 座標関連
	//===================================================================

	// 着弾地点（地面上の目標座標）
	Math::Vector3 m_targetPos;

	// 爆弾の現在座標
	Math::Vector3 m_pos;

	// 落下開始の高さ
	static constexpr float StartHeight = 18.0f;

	// 落下速度
	static constexpr float FallSpeed = 0.2f;

	//===================================================================
	// ダメージ関連
	//===================================================================

	// 爆発の範囲（半径）
	static constexpr float ExplodeRadius = 1.0f;

	// 爆発のダメージ量
	static constexpr int ExplodeDamage = 1;

	// ダメージを与えたかフラグ（1回だけ）
	bool m_isDamageGiven = false;
};