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
#include <fstream>
#include <sstream>
#include <string>

#include "ImGui\\imgui.h"
#include "ImGui\\imgui_impl_win32.h"
#include "ImGui\\imgui_impl_dx11.h"

using namespace DirectX;

struct SimpleVertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 TexCoord;
	XMFLOAT3 tangent;
	XMFLOAT3 biTangent;
};

class DrawableGameObject
{
public:
	DrawableGameObject();
	~DrawableGameObject();

	HRESULT								initMesh(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pContext);
	void								update(float t, vector<pair<ImVec4, string>>*debugLog);
	void								draw(ID3D11DeviceContext* pContext);
	ID3D11Buffer*						getVertexBuffer() { return m_pVertexBuffer; }
	ID3D11Buffer*						getIndexBuffer() { return m_pIndexBuffer; }
	ID3D11ShaderResourceView**			getTextureResourceView() { return &m_pTextureResourceView; 	}
	XMFLOAT4X4*							getTransform() { return &m_World; }
	ID3D11SamplerState**				getTextureSamplerState() { return &m_pSamplerLinear; }
	MaterialPropertiesConstantBuffer	getMaterial() { return m_material;}
	void								setPosition(XMFLOAT3 position);
	XMFLOAT3 getPosition() { return m_position; }
	void								CalculateTangentBinormal2(SimpleVertex v0, SimpleVertex v1, SimpleVertex v2, XMFLOAT3& normal, XMFLOAT3& tangent, XMFLOAT3& binormal);
	void								CalculateModelVectors(SimpleVertex* vertices, int indexCount, WORD* indexList);

	void setStatic(bool isStaticNew) { isStatic = isStaticNew; }
	bool getStatic() { return isStatic; }

	void setSpinSpeed(float speed) { spinSpeed = speed; }
	float getSpinSpeed() { return spinSpeed; }

	void setRadius(float radius) { m_radius = radius; }
	float getRadius() { return m_radius; }

	void setMass(float mass) { m_mass = mass; }
	float getMass() { return m_mass; }

	void setPressure(float Pressure) { m_pressure = Pressure; }
	float getPressure() { return m_pressure; }
	void setDensity(float Density) { m_density = Density; }
	float getDensity() { return m_density; }
	void setViscosity(float Viscosity) { m_viscosity = Viscosity; }
	float getViscosity() { return m_viscosity; }

	void setDynVis(float DynVis) { m_DynVis = DynVis; }
	float getDynVis() { return m_DynVis; }
	void setKinVis(float KinVis) { m_KinVis = KinVis; }
	float getKinVis() { return m_KinVis; }
	float setRefDensity(float refDens) { m_RefDensity = refDens; }
	float getRefDensity() { return m_RefDensity; }

	XMFLOAT3 getVelocity() { return m_Velocity; }
	void setVelocity(XMFLOAT3 velocity) { m_Velocity = velocity; }

	XMFLOAT3 getAcceleration() { return m_Acceleration; }
	void setAcceleration(XMFLOAT3 acceleration) { m_Acceleration = acceleration; }

	std::vector<DrawableGameObject*> getFriends() { return m_Friends; }
	void setFriends(std::vector<DrawableGameObject*> newFriends) { m_Friends = newFriends; }

	void setCubeVB(ID3D11DeviceContext* pContext);

	void addForce(XMFLOAT3 forceVec) { forces.push_back(forceVec); }

private:

	void checkLimits(double deltaT);

	void applyGrav(double deltaT);
	void calcForces(double deltaT, vector<pair<ImVec4, string>>* debugLog);

	XMFLOAT4X4							m_World;

	ID3D11Buffer*						m_pVertexBuffer;
	ID3D11Buffer*						m_pIndexBuffer;
	ID3D11ShaderResourceView*			m_pTextureResourceView;
	ID3D11SamplerState *				m_pSamplerLinear;
	MaterialPropertiesConstantBuffer	m_material;

	XMFLOAT3							m_position;
	XMFLOAT3							m_rotation;

	std::vector<XMFLOAT3> forces;

	XMFLOAT3 m_Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 m_Acceleration = XMFLOAT3(0.0f, 0.0f, 0.0f);

	std::vector<DrawableGameObject*> m_Friends;

	float m_mass = 1.0f;
	float m_radius = 2.0f;
	float m_KinVis = 0.000001f;
	float m_DynVis = 0.001f;
	float m_RefDensity = 1000.0f; //Density of 1000kg/m^3 at sea level at 20deg
	float m_wallradius = m_radius / 5.0f;

	float m_pressure = 0.0f;
	float m_density = 0.0f;
	float m_viscosity = 0.0f;

	bool doesRotate = false;
	bool isStatic = false;
	float spinSpeed = 1.0f;
};

