#include "TitleScene.h"
#include "../SceneManager.h"

void TitleScene::Event()
{
	// 押した瞬間だけ反応する
	// & 0x8000 … 現在押されているか
	// & 0x0001 … 前回押されていなかったか（押した瞬間）
	if (GetAsyncKeyState(VK_RETURN) & 0x8000)
	{
		SceneManager::Instance().SetNextScene(SceneManager::SceneType::Game);
	}
}

void TitleScene::Init()
{
}
