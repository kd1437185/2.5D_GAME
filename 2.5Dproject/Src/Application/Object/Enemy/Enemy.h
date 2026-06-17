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

	void Init()						 override;

	void SetPos(Math::Vector3 _pos) { m_pos = _pos; }

	// 外部から消滅させるための関数
	void SetExpired() { m_isExpired = true; }

	// ダメージを受ける関数
	// ・_damage    … 受けるダメージ量
	// ・_knockBack … ノックバックの方向と強さ
	void TakeDamage(int _damage, const Math::Vector3& _knockBack);

	// 敵同士の当たり判定で private メンバにアクセスするため
	friend class Enemy;

	// 移動方向の取得（AttackEffect のノックバック計算に使用）
	Math::Vector3 GetDir() const { return m_dir; }

	//===================================================================
	// 死亡演出関連
	//===================================================================

	// 死亡演出の状態
	enum class DeathState
	{
		Alive,		// 生存中
		KnockBack,	// 死亡ノックバック中（HPが0になった後）
		Dissolve,	// ディゾルブ中（ノックバック終了後）
	};


private:

	bool SearchPlayer(Math::Vector3& _outPlayerPos);

	// 板ポリゴン
	std::shared_ptr<KdSquarePolygon> m_polygon;

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
	// 毎フレーム減衰させて自然に止まるようにする
	Math::Vector3 m_knockBackVec = Math::Vector3::Zero;

	// ノックバックの減衰率（1フレームあたり何割残るか）
	static constexpr float KnockBackDecay = 0.8f;

	// ノックバックが終了したとみなす速度の閾値
	static constexpr float KnockBackThreshold = 0.01f;

	//===================================================================
	// 無敵時間関連
	// 連続ヒットを防ぐために攻撃を受けた後一定時間無敵にする
	//===================================================================

	// 無敵タイマー（フレーム数）
	int m_invincibleTimer = 0;

	// 無敵時間（フレーム数）
	static constexpr int InvincibleTime = 30;

	//===================================================================
	// 被弾アニメーション関連
	//===================================================================

	// 被弾中フラグ
	// true のとき被弾アニメーションを再生中
	bool m_isHurt = false;

	// 被弾アニメーションのカウント
	float m_hurtAnimeCnt = 0.0f;

	// 被弾アニメーションの総コマ数（横2コマ）
	static constexpr int HurtFrameCount = 2;

	// 被弾アニメーションの速度
	static constexpr float HurtAnimeSpeed = 0.15f;

	// 通常テクスチャ（WALK）
	std::shared_ptr<KdTexture> m_spTexWalk;

	// 被弾テクスチャ（HURT）
	std::shared_ptr<KdTexture> m_spTexHurt;

	// 現在の死亡演出の状態
	DeathState m_deathState = DeathState::Alive;

	// ディゾルブの進行度（0.0〜1.0）
	// 0.0 = 通常・1.0 = 完全に消える
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

	// 攻撃テクスチャ
	std::shared_ptr<KdTexture> m_spTexAttack;

	// プレイヤーへのダメージ量
	static constexpr int AttackDamage = 1;

	// ダメージを与えたかフラグ（1回の攻撃で1回だけダメージを与える）
	bool m_isDamageGiven = false;

	//===================================================================
	// 攻撃クールタイム関連
	//===================================================================

	// 攻撃クールタイムタイマー（フレーム数）
	int m_attackCoolTimer = 0;

	// 攻撃クールタイム（フレーム数）
	// 60fps なら 60 = 1秒
	static constexpr int AttackCoolTime = 120;	// 2秒
};