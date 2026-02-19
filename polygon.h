/*
	polygon.h
	20250423 hanaue sho
*/
#ifndef POLYGON_H_
#define POLYGON_H_

#include "GameObject.h"
#include "TransformComponent.h"
#include "MeshFilterComponent.h"
#include "MeshFactory.h"
#include "MaterialComponent.h"
#include "MeshRendererComponent.h"
#include "renderer.h"
#include "texture.h"  // Texture::Load Šù‘¶

#include "SpriteAnimationComponent.h"


class Polygon2D : public GameObject
{
private:
	SpriteClip m_Clip;
	float m_Alpha = 1.0f;
	bool m_IsBlink = false;
	float m_Radian = 0.0f;

public:
	void Init() override;
	void Update(float gameDt, float realDt) override;

	void SetTexture(const char* path);
	void SetSize(float width, float height, bool center = true);
	void SetBlink(bool b) { m_IsBlink = b; }
};

#endif