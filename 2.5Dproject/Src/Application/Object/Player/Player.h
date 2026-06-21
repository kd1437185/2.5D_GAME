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
	void DrawSprite()					override;

	void Init()							override;

	// m_pos を直接返すことで常に最新座標を取得できる
	Math::Vector3 GetPos() const override { return m_pos; }

	// 敵からダメージを受ける関数
	void TakeDamage(int _damage);

	// 必殺技中かどうか取得（カメラ演出用）
	bool IsSpecial() const { return m_isSpecial; }

	// 減速発動アニメ中かどうか取得（カメラ演出用）
	bool IsSlow() const { return m_isSlow; }

	// HP取得（HPバー表示用）
	int GetHp()    const { return m_hp; }
	int GetMaxHp() const { return MaxHP; }

	// SP取得（SPバー表示用）
	int GetSp()    const { return m_sp; }
	int GetMaxSp() const { return MaxSP; }

	// SPを増やす関数
	// 攻撃が敵にヒットしたときに呼ばれる
	void AddSp(int _value)
	{
		m_sp += _value;

		// 最大値を超えないように制限
		if (m_sp > MaxSP) { m_sp = MaxSP; }
	}

	// SPを消費する関数
	// SPが足りていれば消費して true、足りなければ false を返す
	bool UseSp(int _cost)
	{
		// SPが足りなければ消費しない
		if (m_sp < _cost) { return false; }

		// SPを消費
		m_sp -= _cost;
		return true;
	}

	// 死亡しているかどうか取得
	bool IsDead() const { return m_isDead; }

	// 死亡演出が完了したかどうか取得
	// 死亡アニメが最後まで再生されたら true
	bool IsDeathAnimeEnd() const { return m_isDeathAnimeEnd; }

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

	//===================================================================
	// 回避関連
	//===================================================================

	// 回避中フラグ
	bool m_isDashing = false;

	// 回避無敵フラグ
	// true のとき敵の攻撃を受けない
	bool m_isDashInvincible = false;

	// 回避アニメーションカウント
	float m_dashAnimeCnt = 0.0f;

	// 回避アニメーションの総コマ数（9コマ）
	static constexpr int DashFrameCount = 9;

	// 回避アニメーションの速度
	static constexpr float DashAnimeSpeed = 0.5f;

	// 回避速度
	static constexpr float DashSpeed = 0.2f;

	// 回避クールタイムタイマー（フレーム数）
	int m_dashCoolTimer = 0;

	// 回避クールタイム（フレーム数）
	// 後で変更しやすいように定数で管理
	// 60fps × 1秒 = 60フレーム
	static constexpr int DashCoolTime = 60;

	// 回避テクスチャリスト（左向き）
	std::vector<std::shared_ptr<KdTexture>> m_animTexturesDashL;

	// 回避テクスチャリスト（右向き）
	std::vector<std::shared_ptr<KdTexture>> m_animTexturesDashR;

	//===================================================================
	// 必殺技関連
	//===================================================================

	// 必殺技中フラグ
	bool m_isSpecial = false;

	// 必殺技アニメーションカウント
	float m_specialAnimeCnt = 0.0f;

	// 必殺技アニメーションの総コマ数（9コマ）
	static constexpr int SpecialFrameCount = 9;

	// 必殺技アニメーションの速度
	static constexpr float SpecialAnimeSpeed = 0.2f;

	// 必殺技クールタイムタイマー（フレーム数）
	int m_specialCoolTimer = 0;

	// 必殺技クールタイム（フレーム数）
	// 後で調整しやすいように定数で管理
	static constexpr int SpecialCoolTime = 300;	// 5秒

	// 必殺技テクスチャリスト（正面・左右共通）
	std::vector<std::shared_ptr<KdTexture>> m_animTexturesSpecial;

	//===================================================================
	// 減速アクション関連
	//===================================================================

	// 減速アクション中フラグ
	bool m_isSlow = false;

	// 減速発動アニメーションカウント
	float m_slowAnimeCnt = 0.0f;

	// 減速発動アニメーションの総コマ数（9コマ）
	static constexpr int SlowFrameCount = 9;

	// 減速発動アニメーションの速度
	static constexpr float SlowAnimeSpeed = 0.2f;

	// 減速効果の残り時間（フレーム数）
	int m_slowEffectTimer = 0;

	// 減速効果時間（フレーム数）
	// 後で調整しやすいように定数で管理
	static constexpr int SlowEffectTime = 600;	// 5秒

	// 減速クールタイムタイマー
	int m_slowCoolTimer = 0;

	// 減速クールタイム（フレーム数）
	static constexpr int SlowCoolTime = 600;	// 10秒

	// 減速発動アニメーション用テクスチャ（正面・共通）
	std::vector<std::shared_ptr<KdTexture>> m_animTexturesSlow;

	//===================================================================
	// 残像関連
	//===================================================================

	// 残像1個分の情報
	struct AfterImage
	{
		Math::Vector3                m_pos;		// 座標
		std::shared_ptr<KdTexture>   m_spTex;	// そのときのテクスチャ
		bool                         m_isFlip;	// 左右反転
		float                        m_alpha;	// 透明度
	};

	// 残像のリスト
	std::list<AfterImage> m_afterImages;

	// 残像を出す間隔のカウンタ
	int m_afterImageTimer = 0;

	// 残像描画用の板ポリゴン（毎フレーム生成を避けるためメンバ化）
	KdSquarePolygon m_afterImagePolygon;

	//===================================================================
	// HP関連
	//===================================================================

	// 現在のHP
	int m_hp = 10;

	// 最大HP（後で調整可能）
	static constexpr int MaxHP = 10;

	// HPバーの画像
	std::shared_ptr<KdTexture> m_spHpBarBg;	// 枠
	std::shared_ptr<KdTexture> m_spHpBarFill;	// 中身

	//===================================================================
	// SP関連
	//===================================================================

	// 現在のSP
	int m_sp = 0;

	// 最大SP
	static constexpr int MaxSP = 100;

	// SPバーの中身画像
	std::shared_ptr<KdTexture> m_spSpBarFill;	// 中身（青）

	//===================================================================
	// 被弾無敵関連
	//===================================================================

	// 被弾無敵タイマー（フレーム数）
	// 0より大きい間は無敵＆点滅する
	int m_damageInvincibleTimer = 0;

	// 被弾無敵時間（フレーム数）
	// 後で調整しやすいように定数で管理
	static constexpr int DamageInvincibleTime = 90;	// 約1.5秒

	// 点滅の間隔（フレーム数）
	// この間隔で表示・非表示を切り替える
	static constexpr int BlinkInterval = 5;

	// 必殺技のSP消費量
	static constexpr int SpecialCostSP = 50;

	// 減速アクションのSP消費量
	static constexpr int SlowCostSP = 30;

	//===================================================================
	// 死亡演出関連
	//===================================================================

	// 死亡中フラグ
	bool m_isDead = false;

	// 死亡アニメーションカウント
	float m_deathAnimeCnt = 0.0f;

	// 死亡アニメーションの総コマ数（9コマ）
	static constexpr int DeathFrameCount = 9;

	// 死亡アニメーションの速度
	static constexpr float DeathAnimeSpeed = 0.2f;

	// 死亡アニメーション用テクスチャ（左向き）
	std::vector<std::shared_ptr<KdTexture>> m_animTexturesDeathL;

	// 死亡アニメーション用テクスチャ（右向き）
	std::vector<std::shared_ptr<KdTexture>> m_animTexturesDeathR;

	// 死亡演出完了フラグ
	bool m_isDeathAnimeEnd = false;
};