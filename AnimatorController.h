/*
	AnimatorController.h
	20260120  hanaue sho
*/
#ifndef ANIMATORCONTROLLER_H_
#define ANIMATORCONTROLLER_H_
#include <string>

struct AnimationClip;

struct Blend1DNode
{
	// ２点ブレンド
	AnimationClip* clipA = nullptr;
	AnimationClip* clipB = nullptr;

	// パラメータの対応値
	float parameterA = 0.0f;
	float parameterB = 1.0f;
	bool loopA = true;
	bool loopB = true;

	// 再生速度を持たせてもよい

	// 無効チェック
	bool IsValid() const noexcept
	{
		return clipA && clipB && (parameterA - parameterB > 1e-8f);
	}
};

struct Blend1DResult
{
	AnimationClip* clipA = nullptr;
	AnimationClip* clipB = nullptr;
	bool loopA = true;
	bool loopB = true;

	// 再生速度を持たせてもよい

	float alpha = 0.0f;
	bool IsValid() const noexcept { return clipA && clipB; }
};

class AnimatorController
{
public:
	Blend1DNode m_Locomotion;

	Blend1DResult EvaluateLocomotion(float speed) const
	{
		Blend1DResult out{};

		out.clipA = m_Locomotion.clipA;
		out.clipB = m_Locomotion.clipB;
		out.loopA = m_Locomotion.loopA;
		out.loopB = m_Locomotion.loopB;

		if (!out.clipA || !out.clipB)
		{
			out.alpha = 0.0f;
			return out;
		}

		// paramA == paramB の事故回避
		const float denom = (m_Locomotion.parameterB - m_Locomotion.parameterA);
		if (denom == 0.0f)
		{
			out.alpha = 0.0f;
			return out;
		}

		float t = (speed - m_Locomotion.parameterA) / denom;
		out.alpha = Clamp01(t);
		return out;
	}
private:
	static float Clamp01(float x) noexcept
	{
		if (x < 0.0f) return 0.0f;
		if (x > 1.0f) return 1.0f;
		return x;
	}
};

#endif