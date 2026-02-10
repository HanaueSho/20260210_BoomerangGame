/*
	enemy.h
	20250514 hanaue sho
*/
#pragma once

#include "gameObject.h"

#include "ColliderComponent.h"
#include "TransformComponent.h"
#include "MeshFilterComponent.h"
#include "MeshFactory.h"
#include "MaterialComponent.h"
#include "MeshRendererComponent.h"
#include "RigidbodyComponent.h"
#include "renderer.h"
#include "texture.h"  // Texture::Load Šù‘¶


class ModelRenderer;

class Enemy : public GameObject
{
private:
	
public:
	void Init() override;
	void Update(float dt) override;
};
