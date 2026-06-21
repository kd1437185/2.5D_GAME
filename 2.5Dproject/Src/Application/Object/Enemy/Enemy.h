#pragma once

// 前方宣言
class Player;

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 敵クラス
// ・プレイヤーを追跡してホーミングする
// ・HPが0になったら消滅する
// ・攻撃を受けたらノックバックする
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
class Enemy : public KdGameObject
{
public:

	Enemy() {}
	~Enemy() override {}

	void Update()					 override;
	void PostUpdate()				 override;

	void GenerateDepthMapFromLight() override;
	void DrawLit()					 override;

	void DrawSprite() override;

	void Init()						 override;

	void SetPos(Math::Vector3 _pos) { m_pos = _pos; }

	// 外部から消滅させるための関数
	void SetExpired() { m_isExpired = true; }

	// ダメージを受ける関数
	// ・_damage    … 受けるダメージ量
	// ・_knockBack … ノックバックの方向と強さ
	void TakeDamage(int _damage, const Math::Vector3& _knockBack);

	// 移動方向の取得（AttackEffect のノックバック計算に使用）
	Math::Vector3 GetDir() const { return m_dir; }

	// 敵同士の当たり判定で private メンバにアクセスするため
	friend class Enemy;

private:

	bool SearchPlayer(Math::Vector3& _outPlayerPos);

	// 板ポリゴン
	std::shared_ptr<KdSquarePolygon> m_polygon;

	//===================================================================
	// テクスチャ（全Enemyで共有）
	// static にすることで敵を生成するたびにロードしなくて済む
	// 最初の1体目だけロードし、以降は使い回す
	//===================================================================
	static std::shared_ptr<KdTexture> m_spTexWalk;	// 歩行
	static std::shared_ptr<KdTexture> m_spTexHurt;	// 被弾
	static std::shared_ptr<KdTexture> m_spTexAttack;	// 攻撃

	// 総コマ数（横9コマ）
	static constexpr int FrameCount = 9;

	// アニメーション情報
	float m_animeCnt = 0;
	float m_animeSpeed = 0.0f;

	// 座標
	Math::Vector3 m_pos;

	// 方向（ベクトルの向き）
	Math::Vector3 m_dir;

	// 移動量（ベクトルの大きさ）
	float m_speed = 0.0f;

	// 左右反転フラグ
	bool m_isFlip = false;

	//===================================================================
	// HP関連
	//===================================================================

	// 現在のHP
	int m_hp = 3;

	// 最大HP
	static constexpr int MaxHP = 3;

	//===================================================================
	// ノックバック関連
	//===================================================================

	// ノックバック中のベクトル
	Math::Vector3 m_knockBackVec = Math::Vector3::Zero;

	// ノックバックの減衰率
	static constexpr float KnockBackDecay = 0.8f;

	// ノックバックが終了したとみなす速度の閾値
	static constexpr float KnockBackThreshold = 0.01f;

	//===================================================================
	// 無敵時間関連
	//===================================================================

	// 無敵タイマー（フレーム数）
	int m_invincibleTimer = 0;

	// 無敵時間（フレーム数）
	static constexpr int InvincibleTime = 30;

	//===================================================================
	// 被弾アニメーション関連
	//===================================================================

	// 被弾中フラグ
	bool m_isHurt = false;

	// 被弾アニメーションのカウント
	float m_hurtAnimeCnt = 0.0f;

	// 被弾アニメーションの総コマ数（横2コマ）
	static constexpr int HurtFrameCount = 2;

	// 被弾アニメーションの速度
	static constexpr float HurtAnimeSpeed = 0.15f;

	//===================================================================
	// 死亡演出関連
	//===================================================================

	// 死亡演出の状態
	enum class DeathState
	{
		Alive,		// 生存中
		KnockBack,	// 死亡ノックバック中
		Dissolve,	// ディゾルブ中
	};

	// 現在の死亡演出の状態
	DeathState m_deathState = DeathState::Alive;

	// ディゾルブの進行度（0.0〜1.0）
	float m_dissolve = 0.0f;

	// ディゾルブの速度
	static constexpr float DissolveSpeed = 0.02f;

	//===================================================================
	// 攻撃関連
	//===================================================================

	// 攻撃状態フラグ
	bool m_isAttacking = false;

	// 攻撃アニメーションのカウント
	float m_attackAnimeCnt = 0.0f;

	// 攻撃アニメーションの速度
	static constexpr float AttackAnimeSpeed = 0.15f;

	// 攻撃アニメーションの総コマ数（横7コマ）
	static constexpr int AttackFrameCount = 7;

	// ダメージを与えるコマ番号（4コマ目 = index3）
	static constexpr int AttackDamageFrame = 3;

	// 攻撃開始距離
	static constexpr float AttackRange = 0.5f;

	// プレイヤーへのダメージ量
	static constexpr int AttackDamage = 1;

	// ダメージを与えたかフラグ（1回の攻撃で1回だけ）
	bool m_isDamageGiven = false;

	// 攻撃クールタイムタイマー（フレーム数）
	int m_attackCoolTimer = 0;

	// 攻撃クールタイム（フレーム数）
	static constexpr int AttackCoolTime = 120;

	//===================================================================
	// HPバー画像（全Enemyで共有）
	//===================================================================
	static std::shared_ptr<KdTexture> m_spHpBarBg;	// 枠
	static std::shared_ptr<KdTexture> m_spHpBarFill;	// 中身
};