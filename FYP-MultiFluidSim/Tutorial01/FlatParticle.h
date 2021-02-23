#pragma once

#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxcolors.h>
#include <DirectXCollision.h>
#include "DDSTextureLoader.h"
#include "resource.h"
#include <iostream>
#include "structures.h"
#include <vector>

#include <SpriteBatch.h>

#include "ImGui\\imgui.h"
#include "ImGui\\imgui_impl_win32.h"
#include "ImGui\\imgui_impl_dx11.h"

class FlatParticle
{
public:
	FlatParticle(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pContext);
	void Draw(ID3D11ShaderResourceView* m_pTextureResourceView, XMFLOAT3 position);
	void Update();

private:
	unique_ptr<SpriteBatch> m_spriteBatch;

	XMFLOAT3 m_velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 m_Acceleration = XMFLOAT3(0.0f, 0.0f, 0.0f);

	float m_mass = 0.001f;
	float m_pressure = 0.0f;
	float m_density = 0.0f;
	float m_viscosity = 0.0f;

	float m_radius = 1.0f;
};

