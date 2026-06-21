#include "ResultScene.h"

#include "../SceneManager.h"

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 初期化
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void ResultScene::Init()
{
	// カメラ生成
	m_camera = std::make_unique<KdCamera>();
	m_camera->SetProjectionMatrix(60);

	//===================================================================
	// 画像のロード
	//===================================================================
	m_spNumberTex = std::make_shared<KdTexture>("Asset/Textures/UI/number.png");
	m_spLabelKill = std::make_shared<KdTexture>("Asset/Textures/UI/討伐.png");
	m_spLabelTorii = std::make_shared<KdTexture>("Asset/Textures/UI/鳥居.png");
	m_spLabelResult = std::make_shared<KdTexture>("Asset/Textures/UI/結果.png");
	m_spResultBg = std::make_shared<KdTexture>("Asset/Textures/UI/result.png");

	//===================================================================
	// スコアデータを SceneManager から取得して計算する
	//===================================================================
	m_killCount = SceneManager::Instance().GetKillCount();
	m_toriiCount = SceneManager::Instance().GetToriiBreakCount();

	// 結果スコア＝討伐数 × 100 × 鳥居破壊数
	m_resultScore = m_killCount * 100 * m_toriiCount;

	//===================================================================
	// 段階表示の初期化
	//===================================================================
	m_phase = 0;
	m_phaseTimer = 0;

	// 演出フラグを解除しておく（次のゲームに持ち越さないため）
	SceneManager::Instance().SetCutScene(false);
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// イベント（入力処理）
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void ResultScene::Event()
{
	// 段階表示の更新
	Update();

	//===================================================================
	// エンターキーでタイトルへ戻る
	// 全部表示し終えてから受け付ける
	//===================================================================
	if (m_phase >= 3 && (GetAsyncKeyState(VK_RETURN) & 0x8000))
	{
		SceneManager::Instance().SetNextScene(SceneManager::SceneType::Title);
	}
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 更新
// ・一定間隔で段階を進めて、進むたびに効果音を鳴らす
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void ResultScene::Update()
{
	// 全段階表示済みなら何もしない
	if (m_phase >= 3) { return; }

	// タイマーを進める
	m_phaseTimer++;

	//===================================================================
	// 間隔に達したら次の段階へ進めて効果音を鳴らす
	//===================================================================
	if (m_phaseTimer >= PhaseInterval)
	{
		m_phaseTimer = 0;
		m_phase++;

		// 効果音を鳴らす
		KdAudioManager::Instance().Play("Asset/Sounds/太鼓と鈴を用いた決定音.wav", false);
	}
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 2D描画
// ・段階に応じて討伐・鳥居・結果を上から順に描画する
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void ResultScene::DrawSpriteScene()
{
	//===================================================================
	// 背景を画面全体に描画（スコアより先に描く）
	//===================================================================
	if (m_spResultBg)
	{
		KdShaderManager::Instance().m_spriteShader.DrawTex(
			m_spResultBg.get(),
			0, 0,			// 画面中央
			1280, 720,		// 画面全体サイズ
			nullptr,
			&kWhiteColor,
			Math::Vector2(0.5f, 0.5f)
		);
	}

	//===================================================================
	// ラベル画像の表示サイズ
	//===================================================================
	const int labelW = 200;
	const int labelH = 64;

	// ラベルのX座標（画面左寄り）
	const int labelX = -200;

	// 数字のX座標（ラベルの右）
	const int numX = 150;

	//===================================================================
	// 段階1以上：討伐数を表示（一番上）
	//===================================================================
	if (m_phase >= 1)
	{
		int posY = 150;

		// 討伐ラベル
		KdShaderManager::Instance().m_spriteShader.DrawTex(
			m_spLabelKill.get(),
			labelX, posY,
			labelW, labelH,
			nullptr, &kWhiteColor,
			Math::Vector2(0.5f, 0.5f)
		);

		// 討伐数
		DrawNumber(m_killCount, numX, posY, 1.0f);
	}

	//===================================================================
	// 段階2以上：鳥居破壊数を表示（真ん中）
	//===================================================================
	if (m_phase >= 2)
	{
		int posY = 0;

		// 鳥居ラベル
		KdShaderManager::Instance().m_spriteShader.DrawTex(
			m_spLabelTorii.get(),
			labelX, posY,
			labelW, labelH,
			nullptr, &kWhiteColor,
			Math::Vector2(0.5f, 0.5f)
		);

		// 鳥居破壊数
		DrawNumber(m_toriiCount, numX, posY, 1.0f);
	}

	//===================================================================
	// 段階3：結果スコアを表示（一番下・大きく）
	//===================================================================
	if (m_phase >= 3)
	{
		int posY = -180;

		// 結果ラベル
		KdShaderManager::Instance().m_spriteShader.DrawTex(
			m_spLabelResult.get(),
			labelX, posY,
			labelW, labelH,
			nullptr, &kWhiteColor,
			Math::Vector2(0.5f, 0.5f)
		);

		// 結果スコア（他より大きく表示）
		DrawNumber(m_resultScore, numX, posY, 1.8f);
	}
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 数字を描画するヘルパー関数
// ・数字スプライトシート（0123456789: の11コマ）から桁ごとに描画する
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void ResultScene::DrawNumber(int _value, int _x, int _y, float _scale)
{
	if (!m_spNumberTex) { return; }

	//===================================================================
	// 数値を桁ごとに分解する
	//===================================================================

	// 0の場合は "0" の1桁
	// それ以外は各桁を取り出す
	std::vector<int> digits;

	if (_value <= 0)
	{
		digits.push_back(0);
	}
	else
	{
		int temp = _value;
		while (temp > 0)
		{
			digits.push_back(temp % 10);	// 一の位から取り出す
			temp /= 10;
		}
		// 取り出した順は逆なので反転する
		std::reverse(digits.begin(), digits.end());
	}

	//===================================================================
	// 1コマのサイズ（スプライトシートの幅÷11）
	//===================================================================
	int numW = m_spNumberTex->GetWidth() / 11;
	int numH = m_spNumberTex->GetHeight();

	int drawW = (int)(numW * _scale);
	int drawH = (int)(numH * _scale);

	//===================================================================
	// 桁数分の合計幅を計算して中央寄せの開始位置を求める
	//===================================================================
	int totalW = drawW * (int)digits.size();
	int startX = _x - totalW / 2;

	//===================================================================
	// 各桁を順に描画する
	//===================================================================
	for (int i = 0; i < (int)digits.size(); ++i)
	{
		// このコマのUV範囲を切り取る
		Math::Rectangle srcRect;
		srcRect.x = digits[i] * numW;
		srcRect.y = 0;
		srcRect.width = numW;
		srcRect.height = numH;

		// X座標（左から順に並べる）
		int posX = startX + drawW * i + drawW / 2;

		KdShaderManager::Instance().m_spriteShader.DrawTex(
			m_spNumberTex.get(),
			posX, _y,
			drawW, drawH,
			&srcRect, &kWhiteColor,
			Math::Vector2(0.5f, 0.5f)
		);
	}
}