#pragma once

#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxcolors.h>
#include <DirectXCollision.h>
#include "DDSTextureLoader.h"
#include "resource.h"
#include <iostream>
#include "structures.h"
#include "DrawableGameObject.h"

static class Collision
{
public:
	static bool isColliding(DrawableGameObject* a, DrawableGameObject* b, float deltaT);

private:
	static void applyCollision(DrawableGameObject* a, DrawableGameObject* b, XMVECTOR directionVector, float deltaT);
};

