#pragma once

class BaseScene;

class SceneManager
{
public:

	// シーン情報
	enum class SceneType
	{
		Title,
		Game,
		Result,
	};

	void PreUpdate();
	void Update();
	void PostUpdate();

	void PreDraw();
	void Draw();
	void DrawSprite();
	void DrawDebug();

	// 次のシーンをセット（フェード遷移が始まる）
	// 遷移中（フェード中）は新しいリクエストを無視する
	void SetNextScene(SceneType _nextScene)
	{
		// 遷移中は受け付けない（連続入力対策）
		if (m_changeState != ChangeState::None) { return; }

		m_nextSceneType = _nextScene;
	}

	// 現在のシーンのオブジェクトリストを取得
	const std::list<std::shared_ptr<KdGameObject>>& GetObjList();

	// 現在のシーンにオブジェクトを追加
	void AddObject(const std::shared_ptr<KdGameObject>& _obj);

	// 演出中フラグの取得・設定
	// 必殺技などの演出中は敵などの更新を止めるために使う
	bool IsCutScene() const { return m_isCutScene; }
	void SetCutScene(bool _flag) { m_isCutScene = _flag; }

	// 敵の速度倍率の取得・設定
	// 1.0：通常　0.5：半速
	// 減速アクション中は Enemy がこの値を参照して速度・アニメ速度を変える
	float GetEnemySpeedRate() const { return m_enemySpeedRate; }
	void  SetEnemySpeedRate(float _rate) { m_enemySpeedRate = _rate; }

	//===================================================================
	// 鳥居の破壊数の管理
	// 鳥居が壊れるたびに加算し、残りの鳥居が強化される
	//===================================================================
	int  GetToriiBreakCount() const { return m_toriiBreakCount; }
	void AddToriiBreakCount() { m_toriiBreakCount++; }
	void ResetToriiBreakCount() { m_toriiBreakCount = 0; }

private:

	// マネージャーの初期化
	// インスタンス生成(アプリ起動)時にコンストラクタで自動実行
	void Init()
	{
		// 開始シーンに切り替え
		ChangeScene(m_currentSceneType);
	}

	// シーン切り替え関数
	void ChangeScene(SceneType _sceneType);

	// 現在のシーンのインスタンスを保持しているポインタ
	std::shared_ptr<BaseScene> m_currentScene = nullptr;

	// 現在のシーンの種類を保持している変数
	SceneType m_currentSceneType = SceneType::Game;

	// 次のシーンの種類を保持している変数
	SceneType m_nextSceneType = m_currentSceneType;

	//===================================================================
	// シーン遷移の進行状態
	// フェード処理を伴うシーン切り替えを管理する
	//===================================================================
	enum class ChangeState
	{
		None,		// 通常（遷移していない）
		FadeOut,	// フェードアウト待ち（ふすまが閉じるのを待つ）
		FadeIn,		// フェードイン待ち（ふすまが開くのを待つ）
	};

	// 現在の遷移状態
	ChangeState m_changeState = ChangeState::None;

	// 演出中フラグ
	// true のとき敵などはUpdateをスキップする
	bool m_isCutScene = false;

	// 敵の速度倍率（1.0：通常）
	float m_enemySpeedRate = 1.0f;

	// 鳥居の破壊数（残りの鳥居の強化に使う）
	int m_toriiBreakCount = 0;

private:

	SceneManager() { Init(); }
	~SceneManager() {}

public:

	// シングルトンパターン
	// 常に存在する && 必ず1つしか存在しない
	// どこからでもアクセスが可能で便利だが
	// 何でもかんでもシングルトンという思考はNG
	static SceneManager& Instance()
	{
		static SceneManager instance;
		return instance;
	}
};