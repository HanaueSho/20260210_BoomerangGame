/*
	field.h
	20250820 hanaue sho
*/
#ifndef FIELD_H_
#define FIELD_H_
#include "GameObject.h"
#include "TransformComponent.h"
#include "MeshFilterComponent.h"
#include "MeshFactory.h"
#include "MaterialComponent.h"
#include "MeshRendererComponent.h"
#include "renderer.h"
#include "texture.h"  // Texture::Load Šù‘¶

class Field : public GameObject
{
private:
	float m_Height = 2.0f;
public:
	void Init() override;

	void SetHeight(float h) { m_Height = h; }
};

#endif