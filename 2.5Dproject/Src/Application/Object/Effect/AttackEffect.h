#pragma once

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 攻撃エフェクトクラス
// ・プレイヤーの攻撃時に生成されるスプライトエフェクト
// ・スフィア判定で敵に当たったら敵を消滅させる
// ・全コマ再生したら自動的に消滅する（m_isExpired = true）
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
class AttackEffect : public KdGameObject
{
public:

	AttackEffect() {}
	~AttackEffect() override {}

	void Init()			override;
	void Update()		override;
	void PostUpdate()	override;
	void DrawBright()		override;

	// 生成時に座標と向きを設定する
	// ・_pos    … 生成座標
	// ・_isFlip … true のとき画像を左右反転（左向き攻撃）
	void Setup(const Math::Vector3& _pos, bool _isFlip)
	{
		m_pos = _pos;
		m_isFlip = _isFlip;
	}

private:

	// 板ポリゴン
	std::shared_ptr<KdSquarePolygon> m_polygon;

	// 座標
	Math::Vector3 m_pos;

	// アニメーションカウント
	float m_animeCnt = 0.0f;

	// アニメーション速度
	float m_animeSpeed = 0.3f;

	// 総コマ数（横5×縦2 - 空き1 = 9コマ）
	static constexpr int FrameCount = 9;

	// 当たり判定の半径
	static constexpr float CollisionRadius = 0.8f;

	// 左右反転フラグ
	bool m_isFlip = false;

	// ヒット済みフラグ
	// 一度敵にヒットしたら以降は当たり判定しない
	bool m_isHit = false;
};