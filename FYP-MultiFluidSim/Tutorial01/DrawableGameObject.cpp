#include "DrawableGameObject.h"

using namespace std;
using namespace DirectX;

#define NUM_VERTICES 36

DrawableGameObject::DrawableGameObject()
{
	m_pVertexBuffer = nullptr;
	m_pIndexBuffer = nullptr;
	m_pTextureResourceView = nullptr;
	m_pSamplerLinear = nullptr;
	m_position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);

	// Initialize the world matrix
	XMStoreFloat4x4(&m_World, XMMatrixIdentity());
}


DrawableGameObject::~DrawableGameObject()
{
	if( m_pVertexBuffer ) 
		m_pVertexBuffer->Release();
	
	if( m_pIndexBuffer )
		m_pIndexBuffer->Release();

	if (m_pTextureResourceView)
		m_pTextureResourceView->Release();

	if (m_pSamplerLinear)
		m_pSamplerLinear->Release();
}

HRESULT DrawableGameObject::initMesh(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pContext)
{
	// Create vertex buffer
	SimpleVertex *vertices = new SimpleVertex[24]
	{
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },

		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },

		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },

		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },

		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) , XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },

		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
	};

	// Create index buffer
	WORD indices[] =
	{
		3,1,0,
		2,1,3,

		6,4,5,
		7,4,6,

		11,9,8,
		10,9,11,

		14,12,13,
		15,12,14,

		19,17,16,
		18,17,19,

		22,20,21,
		23,20,22
	};

	CalculateModelVectors(vertices, 36, indices);
	
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * 24;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData = {};
	InitData.pSysMem = vertices;
	HRESULT hr = pd3dDevice->CreateBuffer(&bd, &InitData, &m_pVertexBuffer);
	if (FAILED(hr))
		return hr;


	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * NUM_VERTICES;        // 36 vertices needed for 12 triangles in a triangle list
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	InitData.pSysMem = indices;
	hr = pd3dDevice->CreateBuffer(&bd, &InitData, &m_pIndexBuffer);
	if (FAILED(hr))
		return hr;

	setCubeVB(pContext);
	
	// Set primitive topology
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = pd3dDevice->CreateSamplerState(&sampDesc, &m_pSamplerLinear);

	m_material.Material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_material.Material.Specular = XMFLOAT4(1.0f, 0.2f, 0.2f, 1.0f);
	m_material.Material.SpecularPower = 32.0f;
	m_material.Material.UseTexture = true;

	return hr;
}

void DrawableGameObject::setPosition(XMFLOAT3 position)
{
	m_position = position;
}

void DrawableGameObject::update(float t, vector<pair<ImVec4, string>>* debugLog)
{
	if (!isStatic)
	{
		calcForces(t, debugLog);
	}

	forces.clear();

	if (doesRotate)
	{
		m_rotation.y += t * spinSpeed;
		if (m_rotation.y >= XMConvertToRadians(360.0f))
		{
			m_rotation.y = 0.0f;
		}
	}

	XMMATRIX mSpin = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);
	XMMATRIX mTranslate = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
	XMMATRIX mScale = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	XMMATRIX world = mTranslate * mSpin * mScale;
	XMStoreFloat4x4(&m_World, world);
}

void DrawableGameObject::draw(ID3D11DeviceContext* pContext)
{
	setCubeVB(pContext);

	pContext->DrawIndexed(NUM_VERTICES, 0, 0);
}

// NOTE!! - this assumes each face is using its own vertices (no shared vertices)
// so the index file would look like 0,1,2,3,4 and so on
// it won't work with shared vertices as the tangent / binormal for a vertex is related to a specific face
// REFERENCE this has largely been modified from "Mathematics for 3D Game Programmming and Computer Graphics" by Eric Lengyel
void DrawableGameObject::CalculateModelVectors(SimpleVertex* vertices, int indexCount, WORD* indexList)
{
	int faceCount, i, index;
	SimpleVertex vertex1, vertex2, vertex3;
	XMFLOAT3 tangent, binormal, normal;


	// Calculate the number of faces in the model.
	faceCount = indexCount / 3;

	// Initialize the index to the model data.
	index = 0;

	// Go through all the faces and calculate the the tangent, binormal, and normal vectors.
	for (i = 0; i < faceCount; i++)
	{
		if (i == 8)
		{
			int x = 1;
		}
		// Get the three vertices for this face from the model.
		vertex1.Pos.x = vertices[indexList[index]].Pos.x;
		vertex1.Pos.y = vertices[indexList[index]].Pos.y;
		vertex1.Pos.z = vertices[indexList[index]].Pos.z;
		vertex1.TexCoord.x = vertices[indexList[index]].TexCoord.x;
		vertex1.TexCoord.y = vertices[indexList[index]].TexCoord.y;
		vertex1.Normal.x = vertices[indexList[index]].Normal.x;
		vertex1.Normal.y = vertices[indexList[index]].Normal.y;
		vertex1.Normal.z = vertices[indexList[index]].Normal.z;
		index++;

		vertex2.Pos.x = vertices[indexList[index]].Pos.x;
		vertex2.Pos.y = vertices[indexList[index]].Pos.y;
		vertex2.Pos.z = vertices[indexList[index]].Pos.z;
		vertex2.TexCoord.x = vertices[indexList[index]].TexCoord.x;
		vertex2.TexCoord.y = vertices[indexList[index]].TexCoord.y;
		vertex2.Normal.x = vertices[indexList[index]].Normal.x;
		vertex2.Normal.y = vertices[indexList[index]].Normal.y;
		vertex2.Normal.z = vertices[indexList[index]].Normal.z;
		index++;

		vertex3.Pos.x = vertices[indexList[index]].Pos.x;
		vertex3.Pos.y = vertices[indexList[index]].Pos.y;
		vertex3.Pos.z = vertices[indexList[index]].Pos.z;
		vertex3.TexCoord.x = vertices[indexList[index]].TexCoord.x;
		vertex3.TexCoord.y = vertices[indexList[index]].TexCoord.y;
		vertex3.Normal.x = vertices[indexList[index]].Normal.x;
		vertex3.Normal.y = vertices[indexList[index]].Normal.y;
		vertex3.Normal.z = vertices[indexList[index]].Normal.z;
		index++;

		// Calculate the tangent and binormal of that face.
		CalculateTangentBinormal2(vertex1, vertex2, vertex3, normal, tangent, binormal);

		// Store the normal, tangent, and binormal for this face back in the model structure.
		vertices[indexList[index - 1]].Normal.x = normal.x;
		vertices[indexList[index - 1]].Normal.y = normal.y;
		vertices[indexList[index - 1]].Normal.z = normal.z;
		vertices[indexList[index - 1]].tangent.x = tangent.x;
		vertices[indexList[index - 1]].tangent.y = tangent.y;
		vertices[indexList[index - 1]].tangent.z = tangent.z;
		vertices[indexList[index - 1]].biTangent.x = binormal.x;
		vertices[indexList[index - 1]].biTangent.y = binormal.y;
		vertices[indexList[index - 1]].biTangent.z = binormal.z;

		vertices[indexList[index - 2]].Normal.x = normal.x;
		vertices[indexList[index - 2]].Normal.y = normal.y;
		vertices[indexList[index - 2]].Normal.z = normal.z;
		vertices[indexList[index - 2]].tangent.x = tangent.x;
		vertices[indexList[index - 2]].tangent.y = tangent.y;
		vertices[indexList[index - 2]].tangent.z = tangent.z;
		vertices[indexList[index - 2]].biTangent.x = binormal.x;
		vertices[indexList[index - 2]].biTangent.y = binormal.y;
		vertices[indexList[index - 2]].biTangent.z = binormal.z;

		vertices[indexList[index - 3]].Normal.x = normal.x;
		vertices[indexList[index - 3]].Normal.y = normal.y;
		vertices[indexList[index - 3]].Normal.z = normal.z;
		vertices[indexList[index - 3]].tangent.x = tangent.x;
		vertices[indexList[index - 3]].tangent.y = tangent.y;
		vertices[indexList[index - 3]].tangent.z = tangent.z;
		vertices[indexList[index - 3]].biTangent.x = binormal.x;
		vertices[indexList[index - 3]].biTangent.y = binormal.y;
		vertices[indexList[index - 3]].biTangent.z = binormal.z;
	}

}


void DrawableGameObject::CalculateTangentBinormal2(SimpleVertex v0, SimpleVertex v1, SimpleVertex v2, XMFLOAT3& normal, XMFLOAT3& tangent, XMFLOAT3& binormal)
{
	// http://softimage.wiki.softimage.com/xsidocs/tex_tangents_binormals_AboutTangentsandBinormals.html

	// 1. CALCULATE THE NORMAL
	XMVECTOR vv0 = XMLoadFloat3(&v0.Pos);
	XMVECTOR vv1 = XMLoadFloat3(&v1.Pos);
	XMVECTOR vv2 = XMLoadFloat3(&v2.Pos);

	XMVECTOR P = vv1 - vv0;
	XMVECTOR Q = vv2 - vv0;

	XMVECTOR e01cross = XMVector3Cross(P, Q);
	XMFLOAT3 normalOut;
	XMStoreFloat3(&normalOut, e01cross);
	normal = normalOut;

	// 2. CALCULATE THE TANGENT from texture space

	float s1 = v1.TexCoord.x - v0.TexCoord.x;
	float t1 = v1.TexCoord.y - v0.TexCoord.y;
	float s2 = v2.TexCoord.x - v0.TexCoord.x;
	float t2 = v2.TexCoord.y - v0.TexCoord.y;


	float tmp = 0.0f;
	if (fabsf(s1 * t2 - s2 * t1) <= 0.0001f)
	{
		tmp = 1.0f;
	}
	else
	{
		tmp = 1.0f / (s1 * t2 - s2 * t1);
	}

	XMFLOAT3 PF3, QF3;
	XMStoreFloat3(&PF3, P);
	XMStoreFloat3(&QF3, Q);

	tangent.x = (t2 * PF3.x - t1 * QF3.x);
	tangent.y = (t2 * PF3.y - t1 * QF3.y);
	tangent.z = (t2 * PF3.z - t1 * QF3.z);

	tangent.x = tangent.x * tmp;
	tangent.y = tangent.y * tmp;
	tangent.z = tangent.z * tmp;

	XMVECTOR vn = XMLoadFloat3(&normal);
	XMVECTOR vt = XMLoadFloat3(&tangent);

	// 3. CALCULATE THE BINORMAL
	// left hand system b = t cross n (rh would be b = n cross t)
	XMVECTOR vb = XMVector3Cross(vt, vn);

	vn = XMVector3Normalize(vn);
	vt = XMVector3Normalize(vt);
	vb = XMVector3Normalize(vb);

	XMStoreFloat3(&normal, vn);
	XMStoreFloat3(&tangent, vt);
	XMStoreFloat3(&binormal, vb);

	return;
}

void DrawableGameObject::setCubeVB(ID3D11DeviceContext* pContext)
{
	// Set vertex buffer
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;

	pContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
	pContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void DrawableGameObject::applyGrav(double deltaT)
{
	forces.push_back(XMFLOAT3(0.0f, -gravityConst * deltaT, 0.0f));
}

void DrawableGameObject::checkLimits(double deltaT)
{
	XMFLOAT3 pos = getPosition();
	XMFLOAT3 newPos = pos;
	XMFLOAT3 totalForce = XMFLOAT3(0.0f, 0.0f, 0.0f);

	if (pos.z < -15.0f || pos.z > 15.0f)
	{
		m_Velocity.z = -m_Velocity.z * 0.9f;
	}

	if (pos.x < -15.0f || pos.x > 15.0f)
	{
		m_Velocity.x = -m_Velocity.x * 0.9f;
	}
	
	if (pos.y < 0.0f)
	{
		float force = (m_mass * -m_Velocity.y);

		newPos.y = 0.0f;
		totalForce.y += force;
	}
	else if (pos.y > 15.0f)
	{
		float force = (m_mass * -m_Velocity.y);

		newPos.y = 15.0f;
		totalForce.y += force;
	}

	setPosition(newPos);
	addForce(totalForce);
}

void DrawableGameObject::calcForces(double deltaT, vector<pair<ImVec4, string>>*debugLog)
{
	ImVec4 colour = ImVec4(1.0f, 1.0f, 1.0f, 0.5f);
	m_Acceleration = XMFLOAT3(0.0f, 0.0f, 0.0f);

	XMFLOAT3 totalForce = XMFLOAT3(0.0f, 0.0f, 0.0f);

	XMVECTOR forceVec = XMLoadFloat3(&totalForce);
	float forceMag;
	XMStoreFloat(&forceMag, XMVector3Length(forceVec));

	for (int i = 0; i < forces.size(); i++)
	{
		forceVec = XMLoadFloat3(&forces[i]);
		XMStoreFloat(&forceMag, XMVector3Length(forceVec));

		if (forceMag > 0.0000000001f)
		{
			totalForce.x += forces[i].x;
			totalForce.y += forces[i].y;
			totalForce.z += forces[i].z;
		}
	}

	XMFLOAT3 wallNorm = XMFLOAT3(0.0f, 0.0f, 0.0f);
	float wallDist = 100000000000000.0f;
	XMFLOAT3 tempPos = m_position;

	if (m_position.x < -15.0f + m_wallradius)
	{
		wallNorm.x = 1.0f;
		wallDist = m_position.x - -15.0f;

		tempPos.x = -15.0f + m_wallradius;
	}
	else if (m_position.x > 15.0f - m_wallradius)
	{
		wallNorm.x = -1.0f;
		wallDist = 15.0f - m_position.x;

		tempPos.x = 15.0f - m_wallradius;
	}

	if (m_position.y < -15.0f + m_wallradius)
	{
		wallNorm.y = 1.0f;
		wallDist = m_position.y - -15.0f;

		tempPos.y = -15.0f + m_wallradius;
	}
	else if (m_position.y > 15.0f - m_wallradius)
	{
		wallNorm.y = -1.0f;
		wallDist = 15.0f - m_position.y;

		tempPos.y = 15.0f - m_wallradius;
	}

	if (m_position.z < -15.0f + m_wallradius)
	{
		wallNorm.z = 1.0f;
		wallDist = m_position.z - -15.0f;

		tempPos.z = -15.0f + m_wallradius;
	}
	else if (m_position.z > 15.0f - m_wallradius)
	{
		wallNorm.z = -1.0f;
		wallDist = 15.0f - m_position.z;

		tempPos.z = 15.0f - m_wallradius;
	}

	//TODO:: Fix this -> What portion of velo is opposite to wall normal, what portion of accel is opposite to wall normal (get this from total force)
	//-= velo portion from velocity
	//-= accel portion from acceleration
	//Move particle by wall normal + overlap distance (Set pos)
	//Apply forces

	//If total force < certain amount don't do accel changes, if velo is < value don't change velo, if wall dist < value don't move particle

	XMFLOAT3 tempAcc = XMFLOAT3(0.0f,0.0f,0.0f);
	XMFLOAT3 tempVel = XMFLOAT3(0.0f,0.0f,0.0f);
	
	if (wallDist <= m_wallradius)
	{
		tempAcc.x = totalForce.x / m_mass;
		tempAcc.y = totalForce.y / m_mass;
		tempAcc.z = totalForce.z / m_mass;

		tempVel.x = m_Velocity.x + (m_Acceleration.x * deltaT);
		tempVel.y = m_Velocity.y + (m_Acceleration.y * deltaT);
		tempVel.z = m_Velocity.z + (m_Acceleration.z * deltaT);

		if (m_Velocity.x == INFINITY || m_Velocity.y == INFINITY || m_Velocity.z == INFINITY)
		{
			m_Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
		}

		if (abs(m_Velocity.x) > 0.001f || abs(m_Velocity.y) > 0.001f || abs(m_Velocity.z) > 0.001f)
		{
			m_Velocity.x -= tempVel.x * wallNorm.x;
			m_Velocity.y -= tempVel.y * wallNorm.y;
			m_Velocity.z -= tempVel.z * wallNorm.z;
			colour.x = 1.0f;
		}

		if (abs(totalForce.x) > 0.001f || abs(totalForce.y) > 0.001f || abs(totalForce.z > 0.001f))
		{
			m_Acceleration.x -= tempAcc.x * wallNorm.x;
			m_Acceleration.y -= tempAcc.y * wallNorm.y;
			m_Acceleration.z -= tempAcc.z * wallNorm.z;
			colour.y = 1.0f;
		}

		if (wallDist < m_wallradius)
		{
			m_position = tempPos;
			colour.z = 1.0f;
		}

		colour.w = 1.0f;
	}

	m_Acceleration.x += totalForce.x / m_mass;
	m_Acceleration.y += totalForce.y / m_mass;
	m_Acceleration.z += totalForce.z / m_mass;

	m_Velocity.x += m_Acceleration.x * deltaT;
	m_Velocity.y += m_Acceleration.y * deltaT;
	m_Velocity.z += m_Acceleration.z * deltaT;

	m_position.x += m_Velocity.x;
	m_position.y += m_Velocity.y;
	m_position.z += m_Velocity.z;

	bool outside = false;

	if (m_position.x == INFINITY)
	{
		outside = true;
	}

	if (m_position.y == INFINITY)
	{
		outside = true;
	}

	if (m_position.z == INFINITY)
	{
		outside = true;
	}

	if (outside)
	{
		colour = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	}

	ostringstream log;

	log << "Cube at point :" << m_position.x << "," << m_position.y << "," << m_position.z <<
		"\n Acceleration::" << m_Acceleration.x << "," << m_Acceleration.y << "," << m_Acceleration.z <<
		"\n Velocity:: " << m_Velocity.x << "," << m_Velocity.y << ", " << m_Velocity.z <<
		"\n Total Force:: " << totalForce.x << "," << totalForce.y << ", " << totalForce.z << "\n\n";

	std::string strLog = log.str();

	pair<ImVec4, string> temp;

	temp = make_pair(colour, strLog);

	debugLog->push_back(temp);

	/*m_Velocity.x *= 0.9f;
	m_Velocity.y *= 0.9f;
	m_Velocity.z *= 0.9f;*/
}