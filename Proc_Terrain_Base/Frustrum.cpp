#include "Frustrum.h"

FrustumClass::FrustumClass()
{
}

FrustumClass::FrustumClass(const FrustumClass&)
{
}

FrustumClass::~FrustumClass()
{
}

void FrustumClass::ConstructFrustum(float, XMMATRIX, XMMATRIX)
{
}

bool FrustumClass::CheckPoint(XMFLOAT3 point)
{
	return frustum.Contains(XMLoadFloat3(&point));
	
}

bool FrustumClass::CheckPoint(float, float, float)
{
	return false;
}

bool FrustumClass::CheckCube(XMFLOAT4)
{
	return false;
}

bool FrustumClass::CheckCube(float, float, float, float)
{
	return false;
}

bool FrustumClass::CheckSphere(XMFLOAT4)
{
	return false;
}

bool FrustumClass::CheckSphere(float, float, float, float)
{
	return false;
}

bool FrustumClass::CheckRectangle(XMFLOAT3, XMFLOAT3)
{
	return false;
}

bool FrustumClass::CheckRectangle(float, float, float, float, float, float)
{
	return false;
}
