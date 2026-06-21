#pragma once

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 鳥居管理クラス
// ・候補地点に鳥居を初期配置する
// ・鳥居が壊れて数が減ったら、空いている候補地点に落下再出現させる
// ・常に一定数の鳥居を保つ
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
class ToriiManager : public KdGameObject
{
public:

	ToriiManager() {}
	~ToriiManager() override {}

	void Init()   override;
	void Update() override;

private:

	// 指定地点に鳥居を生成する（落下モード）
	void SpawnTorii(const Math::Vector3& _pos, bool _isFall);

	// 候補地点リスト
	// この地点のどれかに鳥居が配置される
	std::vector<Math::Vector3> m_spawnPoints;

	// 保つ鳥居の数
	static constexpr int KeepToriiCount = 6;

	// 各鳥居の最大敵出現数
	static constexpr int ToriiMaxSpawn = 20;
};