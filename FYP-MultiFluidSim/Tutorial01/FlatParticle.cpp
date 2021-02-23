#include "FlatParticle.h"

FlatParticle::FlatParticle(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pContext)
{
	m_spriteBatch = make_unique<SpriteBatch>(pContext);
}

void FlatParticle::Draw(ID3D11ShaderResourceView* m_pTextureResourceView, XMFLOAT3 position)
{
	m_spriteBatch->Begin();
	m_spriteBatch->Draw(m_pTextureResourceView, XMFLOAT2(position.x, position.y));
	m_spriteBatch->End();
}

void FlatParticle::Update()
{

}