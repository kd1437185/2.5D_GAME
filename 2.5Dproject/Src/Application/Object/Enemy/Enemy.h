#pragma once

class Player;

class Enemy : public KdGameObject
{
public:

	// 敵同士の当たり判定で private メンバにアクセスするため
	friend class Enemy;

	Enemy() {}
	~Enemy() override {}

	void Update()					 override;
	void PostUpdate()				 override;

	void GenerateDepthMapFromLight() override;
	void DrawLit()					 override;

	void Init()						 override;

	void SetPos(Math::Vector3 _pos) { m_pos = _pos; }

	// 外部から消滅させるための関数
// AttackEffect から呼ばれる
// 将来的には TakeDamage(int damage) に変更する
	void SetExpired() { m_isExpired = true; }

private:

	bool SearchPlayer(Math::Vector3& _outPlayerPos);

	// 板ポリゴン
	std::shared_ptr<KdSquarePolygon> m_polygon;

	// アニメーション用テクスチャリスト
	// 事前に全フレームをロードしておく
	std::vector<std::shared_ptr<KdTexture>> m_animTextures;

	// アニメーション情報
	float m_animeCnt = 0;
	float m_animeSpeed = 0.0f;

	// 座標
	Math::Vector3 m_pos;

	// 方向 (ベクトルの向き)
	Math::Vector3 m_dir;

	// 移動量 (ベクトルの大きさ)
	float m_speed = 0.0f;

	// 画像反転フラグ
	// true のとき画像を左右反転して描画する
	bool m_isFlip = false;

	// 当たり判定の半径
	static constexpr float CollisionRadius = 0.5f;
};