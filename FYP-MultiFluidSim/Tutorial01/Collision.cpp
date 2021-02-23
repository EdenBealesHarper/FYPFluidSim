#include "Collision.h"

bool Collision::isColliding(DrawableGameObject* a, DrawableGameObject* b, float deltaT)
{
	//Load postion vector
	XMFLOAT3 f_aPos = a->getPosition();
	XMFLOAT3 f_bPos = b->getPosition();

	XMVECTOR aPos = XMLoadFloat3(&f_aPos);
	XMVECTOR bPos = XMLoadFloat3(&f_bPos);

	//distance between points
	XMVECTOR dirVec = bPos - aPos;
	float distance = 0.0f;
	XMStoreFloat(&distance, XMVector3Length(dirVec));

	//Are the circles colliding
	if (distance < (a->getRadius() + b->getRadius()))
	{
		applyCollision(a, b, dirVec, deltaT);
		return true;
	}

	return false;
}

void Collision::applyCollision(DrawableGameObject* a, DrawableGameObject* b, XMVECTOR directionVector, float deltaT)
{
	float distance = 0.0f;
	XMStoreFloat(&distance, XMVector3Length(directionVector));

	float overlapPerc = ((distance - (a->getRadius() + b->getRadius())) / distance);
	float overlapPercA = overlapPerc * (a->getRadius() / (a->getRadius() + b->getRadius()));
	float overlapPercB = overlapPerc * (b->getRadius() / (a->getRadius() + b->getRadius()));

	XMVECTOR DirVecNormalised = XMVector3Normalize(directionVector);

	XMVECTOR outputForceA = XMVectorScale(-DirVecNormalised, a->getRadius() * overlapPercA * a->getRadiusForceScale() * deltaT);
	XMVECTOR outputForceB = XMVectorScale(DirVecNormalised, b->getRadius() * overlapPercB * b->getRadiusForceScale() * deltaT);

	XMFLOAT3 appliedForceAFloat;
	XMFLOAT3 appliedForceBFloat;

	XMStoreFloat3(&appliedForceAFloat, outputForceB);
	XMStoreFloat3(&appliedForceBFloat, outputForceA);

	a->addForce(appliedForceAFloat);
	b->addForce(appliedForceBFloat);
}