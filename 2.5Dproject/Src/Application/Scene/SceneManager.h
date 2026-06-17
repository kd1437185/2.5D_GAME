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