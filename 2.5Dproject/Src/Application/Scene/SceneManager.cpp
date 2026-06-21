#include "SceneManager.h"

#include "BaseScene/BaseScene.h"
#include "TitleScene/TitleScene.h"
#include "GameScene/GameScene.h"
#include "FadeManager/FadeManager.h"
#include "ResultScene/ResultScene.h"

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 更新の前処理
// ・フェードの更新
// ・フェードを伴うシーン遷移の進行管理
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void SceneManager::PreUpdate()
{
	//===================================================================
	// フェードの更新
	// シーン遷移判定より前に更新することで
	// IsFadeOutEnd() / IsFadeInEnd() を正しいタイミングで取得できる
	//===================================================================
	FadeManager::Instance().Update();

	//===================================================================
	// シーン遷移の進行管理
	//
	// 流れ：
	//   None    … 遷移リクエストが来たらフェードアウト開始 → FadeOut へ
	//   FadeOut … フェードアウト完了でシーン切替＆フェードイン開始 → FadeIn へ
	//   FadeIn  … フェードイン完了で通常状態に戻る → None へ
	//===================================================================
	switch (m_changeState)
	{
		//--------------------------------------
		// 通常状態
		// 遷移リクエスト（cur != next）が来たらフェードアウト開始
		//--------------------------------------
	case ChangeState::None:
	{
		if (m_currentSceneType != m_nextSceneType)
		{
			FadeManager::Instance().FadeOut();
			m_changeState = ChangeState::FadeOut;
		}
		break;
	}

	//--------------------------------------
	// フェードアウト待ち
	// ふすまが閉じ切ったらシーンを切り替えてフェードイン開始
	//--------------------------------------
	case ChangeState::FadeOut:
	{
		if (FadeManager::Instance().IsFadeOutEnd())
		{
			ChangeScene(m_nextSceneType);
			FadeManager::Instance().FadeIn();
			m_changeState = ChangeState::FadeIn;
		}
		break;
	}

	//--------------------------------------
	// フェードイン待ち
	// ふすまが開き切ったら通常状態に戻る
	//--------------------------------------
	case ChangeState::FadeIn:
	{
		if (FadeManager::Instance().IsFadeInEnd())
		{
			m_changeState = ChangeState::None;
		}
		break;
	}
	}

	m_currentScene->PreUpdate();
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 更新
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void SceneManager::Update()
{
	m_currentScene->Update();
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 更新の後処理
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void SceneManager::PostUpdate()
{
	m_currentScene->PostUpdate();
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 描画の前処理
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void SceneManager::PreDraw()
{
	m_currentScene->PreDraw();
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 描画
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void SceneManager::Draw()
{
	m_currentScene->Draw();
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 2Dスプライト描画
// ・シーンの2D描画の後にフェードを描画する（最前面に表示するため）
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void SceneManager::DrawSprite()
{
	// シーンの2D描画
	m_currentScene->DrawSprite();

	// フェード描画はシーンより後（最前面に表示するため）
	FadeManager::Instance().DrawSprite();
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// デバッグ描画
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void SceneManager::DrawDebug()
{
	m_currentScene->DrawDebug();
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 現在のシーンのオブジェクトリストを取得
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
const std::list<std::shared_ptr<KdGameObject>>& SceneManager::GetObjList()
{
	return m_currentScene->GetObjList();
}

KdCamera* SceneManager::GetCamera()
{
	return m_currentScene->GetCamera();
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 現在のシーンにオブジェクトを追加
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void SceneManager::AddObject(const std::shared_ptr<KdGameObject>& _obj)
{
	m_currentScene->AddObject(_obj);
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// シーン切り替え
// ・次のシーンを作成して現在のシーンにする
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void SceneManager::ChangeScene(SceneType _sceneType)
{
	// 次のシーンを作成し、現在のシーンにする
	switch (_sceneType)
	{
	case SceneType::Title:
		m_currentScene = std::make_shared<TitleScene>();
		break;
	case SceneType::Game:
		m_currentScene = std::make_shared<GameScene>();
		break;
	case SceneType::Result:
		m_currentScene = std::make_shared<ResultScene>();
		break;
	}

	// 現在のシーン情報を更新
	m_currentSceneType = _sceneType;
}