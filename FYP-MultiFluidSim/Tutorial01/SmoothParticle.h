#pragma once

#include <d3d11_1.h>
#include <d3dcompiler.h>

#include <directxcolors.h>
#include <DirectXMath.h>

#include "structures.h"
#include "resource.h"
#include "DrawableGameObject.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "ImGui\\imgui.h"
#include "ImGui\\imgui_impl_win32.h"
#include "ImGui\\imgui_impl_dx11.h"

class SmoothParticle
{
public:
	void CalcNavStok(vector<DrawableGameObject*> &allObjects, double deltaT, vector<pair<ImVec4, string>> *debugLog);

private:
	vector<pair<ImVec4, string>> localDebugLog;

	float WPoly(float kernalSize, XMFLOAT3 rsubrj);
	XMFLOAT3 WPress(float kernalSize, XMFLOAT3 rsubrj);
	float WVis(float kernalSize, XMFLOAT3 rsubrj);

	float Density(float kernalSize, float otherMass, XMFLOAT3 rsubrj);
	XMFLOAT3 Pressure(float kernalSize, XMFLOAT3 rsubrj, DrawableGameObject& thisObj, DrawableGameObject& otherObj);
	XMFLOAT3 Viscosity(float kernalSize, XMFLOAT3 rsubrj, DrawableGameObject& thisObj, DrawableGameObject& otherObj);

	XMFLOAT3 WallPressure(float kernalSize, DrawableGameObject& thisObj, double deltaT, vector<pair<ImVec4, string>>*debugLog);
	float WallDensity(float kernalSize, DrawableGameObject& thisObj);
};

