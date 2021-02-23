#include "SmoothParticle.h"

void SmoothParticle::CalcNavStok(vector<DrawableGameObject*> &allObjects, double deltaT, bool doSim)
{
	vector<DrawableGameObject*> closePoints;

	if(doSim)
	{
		debugLog.clear();
	}

	for (int obj = 0; obj < allObjects.size(); obj++)
	{
		float density = 0.0f;
		float pressure = kb * (density - 1000.0f);
	
		DrawableGameObject* rObj = allObjects[obj];
		XMFLOAT3 rPos = rObj->getPosition();
		float rRad = rObj->getRadius();

		XMVECTOR vecRPos = XMLoadFloat3(&rPos);

		for (int i = 0; i < allObjects.size(); i++)
		{
			DrawableGameObject* otherObj = allObjects[i];
			if (allObjects[obj] == otherObj)
			{
				continue;
			}

			XMFLOAT3 otherPos = otherObj->getPosition();
			XMVECTOR vecRjPos = XMLoadFloat3(&otherPos);

			float dist;
			XMStoreFloat(&dist, XMVector3Length(XMVectorSubtract(vecRPos, vecRjPos)));

			if (dist < rRad)
			{
				closePoints.push_back(otherObj);
			}
		}

		for (int i = 0; i < closePoints.size(); i++)
		{
			DrawableGameObject* otherObj = closePoints[i];
			XMFLOAT3 otherPos = otherObj->getPosition();
			XMVECTOR vecRjPos = XMLoadFloat3(&otherPos);

			float dist;
			XMStoreFloat(&dist, XMVector3Length(XMVectorSubtract(vecRPos, vecRjPos)));

			XMFLOAT3 rsubrj;
			XMStoreFloat3(&rsubrj, XMVectorSubtract(vecRPos, vecRjPos));

			density += Density(rRad, otherObj->getMass(), rsubrj);
		}

		allObjects[obj]->setDensity(density);
		allObjects[obj]->setPressure(pressure);

		closePoints.clear();
	}

	for (int obj = 0; obj < allObjects.size(); obj++)
	{
		float kinVis = 1.787f;
		XMFLOAT3 viscosity = XMFLOAT3(0.0f, 0.0f, 0.0f);
		XMFLOAT3 pressure = XMFLOAT3(0.0f, 0.0f, 0.0f);

		DrawableGameObject* rObj = allObjects[obj];
		XMFLOAT3 rPos = rObj->getPosition();
		float rRad = rObj->getRadius();

		XMVECTOR vecRPos = XMLoadFloat3(&rPos);

		for (int i = 0; i < allObjects.size(); i++)
		{
			DrawableGameObject* otherObj = allObjects[i];
			if (allObjects[obj] == otherObj)
			{
				continue;
			}

			XMFLOAT3 otherPos = otherObj->getPosition();
			XMVECTOR vecRjPos = XMLoadFloat3(&otherPos);

			float dist;
			XMStoreFloat(&dist, XMVector3Length(XMVectorSubtract(vecRPos, vecRjPos)));

			if (dist < rRad)
			{
				closePoints.push_back(otherObj);
			}
		}

		for (int i = 0; i < closePoints.size(); i++)
		{
			DrawableGameObject* otherObj = closePoints[i];
			XMFLOAT3 otherPos = otherObj->getPosition();
			XMVECTOR vecRjPos = XMLoadFloat3(&otherPos);

			float dist;
			XMStoreFloat(&dist, XMVector3Length(XMVectorSubtract(vecRPos, vecRjPos)));

			XMFLOAT3 rsubrj;
			XMStoreFloat3(&rsubrj, XMVectorSubtract(vecRPos, vecRjPos));

			XMFLOAT3 tempViscosity = Viscosity(rRad, rsubrj, *allObjects[obj], *closePoints[i]);

			viscosity.x += tempViscosity.x;
			viscosity.y += tempViscosity.y;
			viscosity.z += tempViscosity.z;

			XMFLOAT3 tempPressure = Pressure(rRad, rsubrj, *allObjects[obj], *closePoints[i]);

			pressure.x += tempPressure.x;
			pressure.y += tempPressure.y;
			pressure.z += tempPressure.z;
		}

		XMFLOAT3 tempVisco;
		tempVisco.x = kinVis * viscosity.x;
		tempVisco.y = kinVis * viscosity.y;
		tempVisco.z = kinVis * viscosity.z;

		XMFLOAT3 tempPress;
		tempPress.x = -1.0f * pressure.x;
		tempPress.y = -1.0f * pressure.y;
		tempPress.z = -1.0f * pressure.z;

		XMFLOAT3 wallPress = WallPressure(rRad, *allObjects[obj], deltaT);

		XMFLOAT3 grav = XMFLOAT3(0.0f, -gravityConst * deltaT * allObjects[obj]->getMass(), 0.0f);

		allObjects[obj]->addForce(tempVisco);
		allObjects[obj]->addForce(tempPress);
		allObjects[obj]->addForce(wallPress);
		allObjects[obj]->addForce(grav);

		if (doSim && (closePoints.size() > 0 || wallPress.x > 0 || wallPress.y > 0 || wallPress.z > 0))
		{
			ostringstream log;

			log << "Forces on point :" << rPos.x << "," << rPos.y << "," << rPos.z << "\n Visco::" << tempVisco.x << "," << tempVisco.y << "," << tempVisco.z <<
				"\n Press:: " << tempPress.x << "," << tempPress.y << ", " << tempPress.z << "\n Wall::" <<
				wallPress.x << "," << wallPress.y << "," << wallPress.z << "\n Grav::" << grav.y << "\n" << "Number of close:: " << closePoints.size() << "\n";

			string strLog = log.str();

			debugLog.push_back(strLog);
		}

		closePoints.clear();
	}

	ImGui::Text("DeltaTime = %f", deltaT);

	ImGui::Text("Log size = %i", debugLog.size());

	for (int i = 0; i < debugLog.size(); i++)
	{
		ImGui::Text(debugLog[i].c_str());
	}
}

float SmoothParticle::WPoly(float kernalSize, XMFLOAT3 rsubrj)
{
	float scale = 315.0f / (64.0f * (XM_PI * pow(kernalSize, 9.0f)));

	float absRSubRj = sqrt(pow(rsubrj.x, 2.0f) + pow(rsubrj.y, 2.0f) + pow(rsubrj.z, 2.0f));

	float WPoly = scale * pow(pow(kernalSize, 2.0f) - pow(absRSubRj, 2.0f), 3.0f);

	return WPoly;
}

XMFLOAT3 SmoothParticle::WPress(float kernalSize, XMFLOAT3 rsubrj)
{
	float scale = 45.0f / (XM_PI * pow(kernalSize, 6.0f));

	float absRSubRj = sqrt(pow(rsubrj.x, 2.0f) + pow(rsubrj.y, 2.0f) + pow(rsubrj.z, 2.0f));

	XMFLOAT3 WPress;
	WPress.x = scale * pow((kernalSize - absRSubRj), 3.0f) * (rsubrj.x / absRSubRj);
	WPress.y = scale * pow((kernalSize - absRSubRj), 3.0f) * (rsubrj.y / absRSubRj);
	WPress.z = scale * pow((kernalSize - absRSubRj), 3.0f) * (rsubrj.z / absRSubRj);

	return WPress;
}

float SmoothParticle::WVis(float kernalSize, XMFLOAT3 rsubrj)
{
	float scale = 45 / (XM_PI * pow(kernalSize, 6.0f));

	float absRSubRj = sqrt(pow(rsubrj.x, 2.0f) + pow(rsubrj.y, 2.0f) + pow(rsubrj.z, 2.0f));

	float WVis = scale * (kernalSize - absRSubRj);

	return WVis;
}

XMFLOAT3 SmoothParticle::Pressure(float kernalSize, XMFLOAT3 rsubrj, DrawableGameObject& thisObj, DrawableGameObject& otherObj)
{
	float density = otherObj.getDensity();

	float scale = otherObj.getMass() * ((thisObj.getPressure() + otherObj.getPressure()) / 2 * otherObj.getDensity());
	XMFLOAT3 WPressVal = WPress(kernalSize, rsubrj);

	XMFLOAT3 tempPress;
	tempPress.x = scale * WPressVal.x;
	tempPress.y = scale * WPressVal.y;
	tempPress.z = scale * WPressVal.z;

	return tempPress;
}

XMFLOAT3 SmoothParticle::WallPressure(float kernalSize, DrawableGameObject& thisObj, double deltaT)
{
	XMFLOAT3 pos = thisObj.getPosition();

	XMFLOAT3 wallNorm = XMFLOAT3(0.0f, 0.0f, 0.0f);
	float wallDist = 0.0f;


	if (pos.x <= -15.0f + kernalSize)
	{
		wallNorm.x = 1.0f;
		wallDist = pos.x - -15.0f;
	}
	else if (pos.x >= 15.0f - kernalSize)
	{
		wallNorm.x = -1.0f;
		wallDist = pos.x - 15.0f;
	}

	if (pos.y <= 5.0f + kernalSize)
	{
		wallNorm.y = 1.0f;
		wallDist = pos.y - 5.0f;
	}
	else if (pos.y >= 15.0f - kernalSize)
	{
		wallNorm.y = -1.0f;
		wallDist = pos.y - 15.0f;
	}
	
	if (pos.z <= -15.0f + kernalSize)
	{
		wallNorm.z = 1.0f;
		wallDist = pos.z - -15.0f;
	}
	else if (pos.z >= 15.0f - kernalSize)
	{
		wallNorm.z = -1.0f;
		wallDist = pos.z - 15.0f;
	}

	wallDist = abs(wallDist);

	XMFLOAT3 pressForce = XMFLOAT3(0.0f, 0.0f, 0.0f);

	double distTimeSqr = kernalSize * deltaT;

	//XMVECTOR wallDistMag = XMVector3Length(XMLoadFloat3(&wallDist));
	//float wallDistMagF;

	//XMStoreFloat(&wallDistMagF, wallDistMag);

	float ObjMass = thisObj.getMass();
	
	ostringstream log;

	log << "Wall Force Y = " << ObjMass << "* (((" << kernalSize << " - " << wallDist << ") * " << wallNorm.y << ") / " << distTimeSqr << ")" << "\n";

	string strLog = log.str();

	debugLog.push_back(strLog);

	if (wallDist != 0.0f)
	{
		pressForce.x = ObjMass * (((kernalSize - wallDist) * wallNorm.x) / distTimeSqr);
		pressForce.y = ObjMass * (((kernalSize - wallDist) * wallNorm.y) / distTimeSqr);
		pressForce.z = ObjMass * (((kernalSize - wallDist) * wallNorm.z) / distTimeSqr);
	}

	return pressForce;
}

XMFLOAT3 SmoothParticle::Viscosity(float kernalSize, XMFLOAT3 rsubrj, DrawableGameObject & thisObj, DrawableGameObject& otherObj)
{
	float DynVis = 1.793f * pow(10, -3);
	float density = otherObj.getDensity();

	XMFLOAT3 otherObjVelo = otherObj.getVelocity();
	XMFLOAT3 thisObjVelo = thisObj.getVelocity();

	XMFLOAT3 deltaVelocity;
	deltaVelocity.x = otherObjVelo.x - thisObjVelo.x;
	deltaVelocity.y = otherObjVelo.y - thisObjVelo.y;
	deltaVelocity.z = otherObjVelo.z - thisObjVelo.z;

	XMFLOAT3 dvbydensity;

	dvbydensity.x = deltaVelocity.x / density;
	dvbydensity.y = deltaVelocity.y / density;
	dvbydensity.z = deltaVelocity.z / density;

	XMFLOAT3 viscosity;
	viscosity.x = otherObj.getMass() * dvbydensity.x * WVis(kernalSize, rsubrj);
	viscosity.y = otherObj.getMass() * dvbydensity.y * WVis(kernalSize, rsubrj);
	viscosity.z = otherObj.getMass() * dvbydensity.z * WVis(kernalSize, rsubrj);

	return viscosity;
}

float SmoothParticle::Density(float kernalSize, float otherMass, XMFLOAT3 rsubrj)
{
	float Wpoly = WPoly(kernalSize, rsubrj);
	float density = otherMass * Wpoly;

	return density;
}