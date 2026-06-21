#pragma once

class BaseScene
{
public :

	BaseScene()			 { Init(); }
	virtual ~BaseScene() {}

	void PreUpdate();
	void Update();
	void PostUpdate();

	void PreDraw();
	void Draw();
	void DrawSprite();
	void DrawDebug();

	// オブジェクトリストを取得
	const std::list<std::shared_ptr<KdGameObject>>& GetObjList()
	{
		return m_objList;
	}
	
	// オブジェクトリストに追加
	void AddObject(const std::shared_ptr<KdGameObject>& _obj)
	{
		m_objList.push_back(_obj);
	}

	// カメラのポインタを取得（HPバーの座標変換などに使う）
	// 所有権は渡さず参照だけ提供する
	KdCamera* GetCamera() const { return m_camera.get(); }

protected :

	// 継承先シーンで必要ならオーバーライドする
	virtual void Event();
	virtual void Init();

	// シーン独自の2D描画（継承先でオーバーライド）
	// オブジェクトの2D描画の後に呼ばれる
	virtual void DrawSpriteScene() {}

	std::unique_ptr<KdCamera> m_camera = nullptr;

	// 全オブジェクトのアドレスをリストで管理
	std::list<std::shared_ptr<KdGameObject>> m_objList;
};
