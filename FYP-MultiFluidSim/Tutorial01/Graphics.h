#pragma once

#define _XM_NO_INTRINSICS_

using namespace std;

#include <windows.h>
#include <windowsx.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include <DirectXCollision.h>
#include "DDSTextureLoader.h"
#include "resource.h"
#include <iostream>

#include "DrawableGameObject.h"
#include "structures.h"
#include "Collision.h"
#include "SmoothParticle.h"

#include <vector>

#include "ImGui\\imgui.h"
#include "ImGui\\imgui_impl_win32.h"
#include "ImGui\\imgui_impl_dx11.h"

#define WKEY 0x57
#define AKEY 0x41
#define SKEY 0x53
#define DKEY 0x44
#define QKEY 0x51
#define EKEY 0x45
#define RKEY 0x52
#define FKEY 0x46

#define NUMTEXTURE 3
#define NUMCUBES1 300
#define NUMCUBES2 0
#define NUMCUBES3 0
#define NUMCUBES4 0

class Graphics
{
	typedef vector<DrawableGameObject*> vecDrawables;
	
	public:
		HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
		HRESULT InitDevice();
		HRESULT InitMesh();
		HRESULT InitWorld(int width, int height);
		void CleanupDevice();
		static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
		void Render(vector<pair<ImVec4, string>>* debugLog);
		void Update(double dt, vector<pair<ImVec4, string>>*debugLog);

		void UpdateCamera();
		void ReadKeyboard(float deltaT);

	private:
		HINSTANCE g_hInst = nullptr;
		HWND g_hWnd = nullptr;
		D3D_DRIVER_TYPE g_driverType = D3D_DRIVER_TYPE_NULL;
		D3D_FEATURE_LEVEL g_featureLevel = D3D_FEATURE_LEVEL_11_0;
		ID3D11Device* g_pd3dDevice = nullptr;
		ID3D11Device1* g_pd3dDevice1 = nullptr;
		ID3D11DeviceContext* g_pImmediateContext = nullptr;
		ID3D11DeviceContext1* g_pImmediateContext1 = nullptr;
		IDXGISwapChain* g_pSwapChain = nullptr;
		IDXGISwapChain1* g_pSwapChain1 = nullptr;
		ID3D11RenderTargetView* g_pRenderTargetView = nullptr;
		ID3D11Texture2D* g_pDepthStencil = nullptr;
		ID3D11DepthStencilView* g_pDepthStencilView = nullptr;
		ID3D11VertexShader* g_pVertexShader = nullptr;

		ID3D11PixelShader* g_pPixelShader = nullptr;
		ID3D11PixelShader* g_pPixelShaderSolid = nullptr;

		ID3D11InputLayout* g_pVertexLayout = nullptr;
		ID3D11Buffer* g_pVertexBuffer = nullptr;
		ID3D11Buffer* g_pIndexBuffer = nullptr;

		ID3D11Buffer* g_pConstantBuffer = nullptr;
		ID3D11Buffer* g_pConstantBufferPP = nullptr;
		ID3D11Buffer* g_pMaterialConstantBuffer = nullptr;
		ID3D11Buffer* g_pLightConstantBuffer = nullptr;

		ID3D11ShaderResourceView* g_pTextureRV = nullptr;

		ID3D11SamplerState* g_pSamplerLinear = nullptr;
		ID3D11SamplerState* g_pSamplerNormal = nullptr;

		int g_viewWidth;
		int g_viewHeight;

		float camSpeed = 5.0f / 1000.0f;
		float camRotateSpeed = 1.0f / 1000.0f;
		float moveLeftRight = 0.0f;
		float moveForwardBack = 0.0f;
		float moveUpDown = 0.0f;
		float camYaw = 0.0f;
		float camPitch = 0.0f;
		POINT prevPos;

		vecDrawables g_GameObjectArr;

		XMFLOAT4X4* g_World1 = new XMFLOAT4X4();
		XMFLOAT4X4* g_View = new XMFLOAT4X4();
		XMFLOAT4X4* g_Projection = new XMFLOAT4X4();

		XMFLOAT4 g_DefaultAt = XMFLOAT4(-1.0f, 0.0f, 0.0f, 0.0f);
		XMFLOAT4 g_DefaultUp = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);
		XMFLOAT4 g_DefaultRight = XMFLOAT4(0.0f, 0.0f, -1.0f, 0.0f);

		XMFLOAT4 g_At = g_DefaultAt;
		XMFLOAT4 g_Up = g_DefaultUp;
		XMFLOAT4 g_Right = g_DefaultRight;

		XMFLOAT4 g_EyePosition = XMFLOAT4(4.0f, 0, 0, 1.0f);
		XMFLOAT4 g_LightPosition = XMFLOAT4(0.0f, 0, -3, 1.0f);

		ID3D11Buffer* g_pScreenQuadVB = nullptr;
		ID3D11Buffer* g_pScreenQuadIB = nullptr;

		SCREEN_VERTEX svQuad[4];

		SmoothParticle* sph;

		void UpdateViewMatrix();

		void buildFSQuad();

		HRESULT LoadShader(WCHAR* shaderName);

		void RenderDefault();
};