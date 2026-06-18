#include "SpecialEffect.h"

#include "../../Scene/SceneManager.h"
#include "../Enemy/Enemy.h"

void SpecialEffect::Init()
{
	// 斬撃エフェクトのテクスチャをロード
	m_spTexSlash = std::make_shared<KdTexture>("Asset/Textures/Effect/sprite-sheet2.png");

	m_timer = 0;
	m_slashAnimeCnt = 0.0f;
	m_flashAlpha = 0.0f;
	m_isDamageGiven = false;
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 更新
// ・斬撃アニメーション → フラッシュ → 終了 の流れで進める
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void SpecialEffect::Update()
{
	m_timer++;

	//===================================================================
	// 斬撃アニメーションを進める
	//===================================================================
	if (m_timer < SlashEndFrame)
	{
		m_slashAnimeCnt += SlashAnimeSpeed;
		if (m_slashAnimeCnt >= (float)SlashFrameCount)
		{
			m_slashAnimeCnt = SlashFrameCount - 1;	// 最後のコマで止める
		}
	}

	//===================================================================
	// フラッシュ演出
	// FlashFrame で一気に白くなり徐々に消えていく
	// このタイミングで全敵にダメージを与える
	//===================================================================
	if (m_timer >= FlashFrame)
	{
		if (m_timer == FlashFrame)
		{
			m_flashAlpha = 1.0f;	// 一気に最大

			//===================================================================
			// 全敵にダメージを与える（1回だけ）
			//===================================================================
			if (!m_isDamageGiven)
			{
				m_isDamageGiven = true;

				for (auto& obj : SceneManager::Instance().GetObjList())
				{
					std::shared_ptr<Enemy> enemy = std::dynamic_pointer_cast<Enemy>(obj);
					if (enemy == nullptr) { continue; }

					// 大ダメージを与える（ノックバックなし）
					enemy->TakeDamage(SpecialDamage, Math::Vector3::Zero);
				}
			}
		}
		else
		{
			m_flashAlpha -= 0.05f;
			if (m_flashAlpha < 0.0f) { m_flashAlpha = 0.0f; }
		}
	}

	//===================================================================
	// 演出終了
	//===================================================================
	if (m_timer >= EffectEndFrame)
	{
		m_isExpired = true;
	}
}

// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
// 2D描画
// ・画面全体に斬撃エフェクトを描画
// ・フラッシュを白い矩形で描画
// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// ///// /////
void SpecialEffect::DrawBright()
{
	//===================================================================
	// 斬撃エフェクトを画面全体に描画
	//===================================================================
	if (m_timer < SlashEndFrame)
	{
		// 現在のコマのUV範囲を計算
		int   x = (int)m_slashAnimeCnt % SlashFrameCount;
		float w = 1.0f / SlashFrameCount;

		Math::Rectangle srcRect;
		srcRect.x = (long)(x * w * m_spTexSlash->GetWidth());
		srcRect.y = 0;
		srcRect.width = (long)(w * m_spTexSlash->GetWidth());
		srcRect.height = (long)m_spTexSlash->GetHeight();

		// 画面中央に画面サイズで描画
		KdShaderManager::Instance().m_spriteShader.DrawTex(
			m_spTexSlash.get(),
			0, 0,				// 画面中央（座標系は中央原点）
			ScreenW, ScreenH,	// 画面全体サイズ
			&srcRect,
			&kWhiteColor,
			Math::Vector2(0.5f, 0.5f)
		);
	}

	//===================================================================
	// フラッシュ描画（白い矩形を全画面に重ねる）
	//===================================================================
	if (m_flashAlpha > 0.0f)
	{
		Math::Color flashColor = { 1.0f, 1.0f, 1.0f, m_flashAlpha };

		KdShaderManager::Instance().m_spriteShader.DrawBox(
			0, 0,				// 画面中央
			ScreenW / 2, ScreenH / 2,	// ハーフサイズ
			&flashColor,
			true				// 塗りつぶし
		);
	}
}