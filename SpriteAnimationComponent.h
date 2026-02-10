/*
	SpriteAnimationComponent.h
	20260209  hanaue sho
*/
#ifndef SPRITEANIMATIONCOMPONENT_H_
#define SPRITEANIMATIONCOMPONENT_H_
#include <algorithm>
#include "Component.h"
#include "SpriteRendererComponent.h"


struct SpriteClip
{
	int columns; // 横分割数
	int rows;	 // 縦分割数
	int frameCount; // 実際に使うコマの数
	float fps;
	bool loopDefault = false;
	bool flipY = false; // テクスチャの上下反転
	bool flipX = false; // テクスチャの左右反転
};

class SpriteAnimationComponent : public Component
{
private:
	SpriteRendererComponent* m_pSpriteRenderer = nullptr;
	const SpriteClip* m_pClip = nullptr; // 外部から渡す

	// 状態
	bool m_IsPlaying = true;
	bool m_IsLoop = true;
	float m_Speed = 1.0f;
	float m_Timer = 0.0f;
	int m_Frame = 0;
	bool m_Dirty = true;

public:
	void OnAdded() override
	{
		m_pSpriteRenderer = Owner()->GetComponent<SpriteRendererComponent>();
	}
	void FixedUpdate(float fixedDt) override
	{
		if (!m_pSpriteRenderer) return;
		if (!m_pClip) return;

		// クリップ情報
		const int columns = m_pClip->columns;
		const int rows	  = m_pClip->rows;
		const float fps   = m_pClip->fps;

		if (columns <= 0 || rows <= 0 || fps <= 0.0f) return;

		const int sheetFrames = columns * rows;
		if (sheetFrames <= 0) return;

		int frames = m_pClip->frameCount;
		if (frames <= 0 || frames > sheetFrames) frames = sheetFrames;
		if (frames <= 0) return;

		// 再生していない場合でも、外部からSetFrameしたときの反映のためのDirty処理
		if (m_IsPlaying)
		{
			m_Timer += fixedDt * m_Speed;

			// フレーム算出
			int newFrame = (int)(m_Timer * fps);

			if (m_IsLoop)
			{
				newFrame = (frames > 0) ? (newFrame % frames) : 0;
			}
			else
			{
				if (newFrame >= frames)
				{
					newFrame = frames - 1;
					m_IsPlaying = false; // 自動停止
				}
				if (newFrame < 0) newFrame = 0;
			}
			if (newFrame != m_Frame)
			{
				m_Frame = newFrame;
				m_Dirty = true;
			}
		}
		if (m_Frame < 0) m_Frame = 0;
		if (m_Frame >= frames) m_Frame = frames - 1;

		if (!m_Dirty) return;
		m_Dirty = false; // dirty 消費

		// frame → uv rect
		const float sx = 1.0f / (float)columns;
		const float sy = 1.0f / (float)rows;
		const int col = m_Frame % columns;
		const int row = m_Frame / columns;

		Vector2 offset{ col * sx, row * sy };
		Vector2 scale{ sx, sy };

		// flip
		if (m_pClip->flipX)
		{
			offset.x = (col + 1) * sx;
			scale.x = -sx;
		}
		if (m_pClip->flipY)
		{
			offset.y = (row + 1) * sy;
			scale.y = -sy;
		}

		m_pSpriteRenderer->SetUVRect(offset, scale);
	}

	// 外部API
	void SetClip(const SpriteClip* clip, bool resetTimer = true) 
	{ 
		m_pClip = clip; 
		if (!m_pClip) return;
		m_IsLoop = m_pClip->loopDefault;
		if (resetTimer) { m_Timer = 0.0f; m_Frame = 0;}
		m_Dirty = true;
	}
	void Play(bool reset = false)
	{
		m_IsPlaying = true;
		if (reset)
		{
			m_Timer = 0.0f;
			m_Frame = 0;
			m_Dirty = true;
		}
	}
	void Stop()
	{
		m_IsPlaying = false;
		m_Timer = 0.0f;
		m_Frame = 0;
		m_Dirty = true;
	}
	void Pause() { m_IsPlaying = false; }
	void SetLoop(bool loop) { m_IsLoop = loop; }
	void SetSpeed(float speed) { m_Speed = std::max(0.0f, speed); }
	void SetFrame(int frame, bool apply = true) 
	{ 
		m_Frame = frame; 

		if (!m_pClip)
		{
			if (m_Frame < 0) m_Frame = 0;
			if (apply) m_Dirty = true;
			return;
		}

		const int sheetFrames = m_pClip->columns * m_pClip->rows;
		int frames = m_pClip->frameCount;
		if (frames <= 0 || frames > sheetFrames) frames = sheetFrames;
		if (frames <= 0) { m_Frame = 0; if (apply) m_Dirty = true; return; }

		if (m_Frame < 0) m_Frame = 0;
		if (m_Frame >= frames) m_Frame = frames - 1;

		if (apply) m_Dirty = true; 
	}
	bool IsFinished() const
	{
		if (!m_pClip) return false;
		if (m_IsLoop) return false;
		const int total = m_pClip->columns * m_pClip->rows;
		int frames = m_pClip->frameCount;
		if (frames <= 0 || frames > total) frames = total;
		if (frames <= 0) return false;
		return (!m_IsPlaying) && (m_Frame >= frames - 1);
	}

};

#endif