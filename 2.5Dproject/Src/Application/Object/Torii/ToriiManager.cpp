#include "ToriiManager.h"

#include "../../Scene/SceneManager.h"
#include "Torii.h"

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 初期化
// ・候補地点を登録して、初期配置する
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void ToriiManager::Init()
{
	//===================================================================
	// 候補地点を登録する
	// ここに挙げた地点のどれかに鳥居が配置される
	// 初期配置数より多めに用意することで再出現の空き地点を確保する
	//===================================================================
	m_spawnPoints.clear();
	m_spawnPoints.push_back(Math::Vector3(-10.0f, 0.0f, 8.0f));
	m_spawnPoints.push_back(Math::Vector3(-6.0f, 0.0f, 10.0f));
	m_spawnPoints.push_back(Math::Vector3(-3.0f, 0.0f, 10.0f));
	m_spawnPoints.push_back(Math::Vector3(3.0f, 0.0f, 10.0f));
	m_spawnPoints.push_back(Math::Vector3(6.0f, 0.0f,8.0f));
	m_spawnPoints.push_back(Math::Vector3(10.0f, 0.0f, 10.0f));
	// 再出現用に追加の候補地点（少し奥）
	m_spawnPoints.push_back(Math::Vector3(-1.0f, 0.0f, 5.0f));
	m_spawnPoints.push_back(Math::Vector3(0.0f, 0.0f, 15.0f));
	m_spawnPoints.push_back(Math::Vector3(4.0f, 0.0f, 15.0f));

	//===================================================================
	// 初期配置（最初は落下なしで配置）
	// 候補地点の先頭から KeepToriiCount 個に鳥居を置く
	//===================================================================
	for (int i = 0; i < KeepToriiCount && i < (int)m_spawnPoints.size(); ++i)
	{
		SpawnTorii(m_spawnPoints[i], false);	// 初期配置は落下なし
	}
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 更新
// ・現在の鳥居の数を数えて、足りなければ空き地点に落下再出現させる
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void ToriiManager::Update()
{
	//===================================================================
	// 演出中はスキップ
	//===================================================================
	if (SceneManager::Instance().IsCutScene())
	{
		return;
	}

	//===================================================================
	// 現在シーンに存在する鳥居の座標を集める
	//===================================================================
	std::vector<Math::Vector3> activeToriiPos;

	for (auto& obj : SceneManager::Instance().GetObjList())
	{
		std::shared_ptr<Torii> torii = std::dynamic_pointer_cast<Torii>(obj);
		if (torii != nullptr)
		{
			activeToriiPos.push_back(torii->GetPos());
		}
	}

	//===================================================================
	// 鳥居の数が足りていれば何もしない
	//===================================================================
	int currentCount = (int)activeToriiPos.size();
	if (currentCount >= KeepToriiCount) { return; }

	//===================================================================
	// 空いている候補地点を探す
	// 既に鳥居がある地点は除外する
	//===================================================================
	std::vector<Math::Vector3> emptyPoints;

	for (auto& point : m_spawnPoints)
	{
		// この候補地点に鳥居がいるか確認
		bool occupied = false;
		for (auto& toriiPos : activeToriiPos)
		{
			// 座標が近ければ「使用中」とみなす
			Math::Vector3 diff = point - toriiPos;
			if (diff.Length() < 1.0f)
			{
				occupied = true;
				break;
			}
		}

		// 空いていればリストに追加
		if (!occupied)
		{
			emptyPoints.push_back(point);
		}
	}

	//===================================================================
	// 空き地点があればランダムに選んで落下再出現
	//===================================================================
	if (!emptyPoints.empty())
	{
		// ランダムに1つ選ぶ
		int index = rand() % (int)emptyPoints.size();

		// 落下モードで生成
		SpawnTorii(emptyPoints[index], true);
	}
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 鳥居を生成する
// ・_pos    … 配置座標
// ・_isFall … true なら落下モードで出現
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void ToriiManager::SpawnTorii(const Math::Vector3& _pos, bool _isFall)
{
	auto torii = std::make_shared<Torii>();
	torii->Init();
	torii->SetPos(_pos);
	torii->SetMaxSpawnCount(ToriiMaxSpawn);

	// 落下モードなら落下開始
	if (_isFall)
	{
		torii->SetFallMode();
	}

	SceneManager::Instance().AddObject(torii);
}