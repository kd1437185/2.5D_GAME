#pragma once

class Player : public KdGameObject
{
public:

	// 方向種類
	enum DirType
	{
		Up = 1 << 0,	// 上　0000 0001
		Down = 1 << 1,	// 下　0000 0010
		Left = 1 << 2,	// 左　0000 0100
		Right = 1 << 3,	// 右　0000 1000
	};

	// アニメーション情報
	struct AnimationInfo
	{
		int   start;	// 開始コマ
		int   end;		// 終了コマ
		float count;	// 現在のカウント数
		float speed;	// アニメーションの速度
	};

	Player() {}
	~Player() override {}

	void Update()						override;
	void PostUpdate()					override;

	void GenerateDepthMapFromLight()	override;
	void DrawLit()						override;

	void Init()							override;

	// m_pos を直接返すことで常に最新座標を取得できる
	Math::Vector3 GetPos() const override { return m_pos; }

	// 敵からダメージを受ける関数
	void TakeDamage(int _damage);

private:

	void ChangeAnimation();

	// 板ポリゴン
	KdSquarePolygon m_polygon;

	// 歩行・左向きアニメーション用テクスチャリスト
	std::vector<std::shared_ptr<KdTexture>> m_animTexturesL;

	// 歩行・右向きアニメーション用テクスチャリスト
	std::vector<std::shared_ptr<KdTexture>> m_animTexturesR;

	// 待機・左向きアニメーション用テクスチャリスト
	std::vector<std::shared_ptr<KdTexture>> m_animTexturesIdleL;

	// 待機・右向きアニメーション用テクスチャリスト
	std::vector<std::shared_ptr<KdTexture>> m_animTexturesIdleR;

	// 攻撃・左向きアニメーション用テクスチャリスト
	std::vector<std::shared_ptr<KdTexture>> m_animTexturesAttackL;

	// 攻撃・右向きアニメーション用テクスチャリスト
	std::vector<std::shared_ptr<KdTexture>> m_animTexturesAttackR;

	// 現在使用中のテクスチャリストへのポインタ
	std::vector<std::shared_ptr<KdTexture>>* m_pCurrentTextures = nullptr;

	// アニメーション情報
	AnimationInfo m_animeInfo = {};

	// キャラが向いている方向種類 ・・・ ビットで管理
	UINT m_dirType = 0;

	// 最後に向いていた方向
	UINT m_lastDirType = DirType::Left;

	// 攻撃中フラグ
	// true のとき攻撃アニメーションを再生中
	bool m_isAttacking = false;

	// 座標
	Math::Vector3 m_pos;

	// 方向 (ベクトルの向き)
	Math::Vector3 m_dir;

	// 移動量
	float m_speed = 0.0f;

	float m_anime = 0.0f;

	// 重力
	float m_gravity = 0.0f;

	// 押しっぱ防止
	bool m_keyFlg = false;
};