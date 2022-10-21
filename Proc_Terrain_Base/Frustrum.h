////////////////////////////////////////////////////////////////////////////////
// Filename: frustumclass.h
////////////////////////////////////////////////////////////////////////////////
#pragma once


//////////////							from rastertek
// INCLUDES //
//////////////
#include "DXF.h"
#include <DirectXMath.h>
#include <DirectXCollision.h>

////////////////////////////////////////////////////////////////////////////////
// Class name: Frustum
////////////////////////////////////////////////////////////////////////////////
class Frustum
{
public:
	Frustum();
	Frustum(const Frustum&);
	~Frustum();

	void ConstructFrustum(XMMATRIX projection, XMMATRIX view);

	bool CheckPoint(XMFLOAT3);
	bool CheckPoint(float, float, float);
	bool CheckCube(XMFLOAT3, float);
	bool CheckCube(float, float, float, float);
	bool CheckSphere(XMFLOAT4);
	bool CheckSphere(float, float, float, float);
	bool CheckCuboid(XMFLOAT3, XMFLOAT3);
	bool CheckCuboid(float, float, float, float, float, float);
private:
	///D3DXPLANE m_planes[6];/// what is the DX 11 version
	BoundingFrustum frustum;
};
