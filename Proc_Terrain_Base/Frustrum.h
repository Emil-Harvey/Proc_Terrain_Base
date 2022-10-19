////////////////////////////////////////////////////////////////////////////////
// Filename: frustumclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _FRUSTUMCLASS_H_
#define _FRUSTUMCLASS_H_


//////////////							from rastertek
// INCLUDES //
//////////////
#include "DXF.h"
#include <DirectXMath.h>
#include <DirectXCollision.h>

////////////////////////////////////////////////////////////////////////////////
// Class name: FrustumClass
////////////////////////////////////////////////////////////////////////////////
class FrustumClass
{
public:
	FrustumClass();
	FrustumClass(const FrustumClass&);
	~FrustumClass();

	void ConstructFrustum(float, XMMATRIX, XMMATRIX);

	bool CheckPoint(XMFLOAT3);
	bool CheckPoint(float, float, float);
	bool CheckCube(XMFLOAT4);
	bool CheckCube(float, float, float, float);
	bool CheckSphere(XMFLOAT4);
	bool CheckSphere(float, float, float, float);
	bool CheckRectangle(XMFLOAT3, XMFLOAT3);
	bool CheckRectangle(float, float, float, float, float, float);
private:
	///D3DXPLANE m_planes[6];/// what is the DX 11 version
	BoundingFrustum frustum;
};

#endif