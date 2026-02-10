/*
	AnimationClip.h
	20251214  hanaue sho
*/
#ifndef ANIMATIONCLIP_H_
#define ANIMATIONCLIP_H_
#include <vector>
#include <string>
#include "Vector3.h"
#include "Quaternion.h"

struct TranslationKey
{
	float time;
	Vector3 value;
};

struct RotationKey
{
	float time;
	Quaternion value;
};

struct ScaleKey
{
	float time;
	Vector3 value;
};

// 補間等に使うが今は未使用でOK
struct KeyFrame
{
	float time;
	Vector3 translation;
	Quaternion rotation;
	Vector3 scale;
};

struct BoneTrack
{
	int boneIndex; // Skelton 内のボーン番号
	std::vector<TranslationKey> translationKeys;
	std::vector<RotationKey>	rotationKeys;
	std::vector<ScaleKey>		scaleKeys;
};

struct AnimationClip
{
	std::string			   name;
	float				   duration; // アニメーション全体の時間
	float				   ticksPerSecond;
	std::vector<BoneTrack> tracks;
};

#endif