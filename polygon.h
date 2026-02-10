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
public:
	void Init() override;
};

#endif