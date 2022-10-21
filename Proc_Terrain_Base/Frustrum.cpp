#include "Frustrum.h"

Frustum::Frustum()
{
}

Frustum::Frustum(const Frustum&)
{
}

Frustum::~Frustum()
{
}

void Frustum::ConstructFrustum(XMMATRIX proj, XMMATRIX view)
{
	frustum = BoundingFrustum(proj);
	XMVECTOR Determinant;
	XMMATRIX InvViewMatrixLH = XMMatrixInverse(&Determinant, view);
	frustum.Transform(frustum, InvViewMatrixLH);
}

bool Frustum::CheckPoint(XMFLOAT3 point)
{
	return frustum.Contains(XMLoadFloat3(&point));
	
}

bool Frustum::CheckPoint(float, float, float)
{
	return false;
}

bool Frustum::CheckCube(XMFLOAT3 pos, float sideLength)
{
	auto cube = BoundingOrientedBox(pos, XMFLOAT3(sideLength, sideLength, sideLength), XMFLOAT4(1,1,1,1));
	return frustum.Intersects(cube);
}

bool Frustum::CheckCube(float x, float y, float z, float size)
{
	return false;
}

bool Frustum::CheckSphere(XMFLOAT4)
{
	return false;
}

bool Frustum::CheckSphere(float, float, float, float)
{
	return false;
}

bool Frustum::CheckCuboid(XMFLOAT3 position, XMFLOAT3 dimensions)
{
	return frustum.Intersects(BoundingOrientedBox(position, dimensions, XMFLOAT4(0, 0, 0, 1)));
}

bool Frustum::CheckCuboid(float pos_x, float pos_y, float pos_z, float x_dim, float y_dim, float z_dim)// bad
{
	/*bool result = false;
	// first check if the center is in, to save extra calculations in simple cases
	if (CheckPoint(position))
		return true;

	//for (int i = 0; i < 6; i++) 
	{// start with the 4 left-side corners
		result = CheckPoint(XMFLOAT3(pos_x - dim_x / 2.f, position.y - dimensions.y / 2.f, position.z - dimensions.z / 2.f));
		if (result) 
			return result;
	}
	{
		result = CheckPoint(XMFLOAT3(pos_x - dim_x / 2.f, position.y - dimensions.y / 2.f, position.z + dimensions.z / 2.f));
		if (result)
			return result;
	}
	{
		result = CheckPoint(XMFLOAT3(pos_x - dim_x / 2.f, position.y + dimensions.y / 2.f, position.z - dimensions.z / 2.f));
		if (result)
			return result;
	}
	{
		result = CheckPoint(XMFLOAT3(pos_x - dim_x / 2.f, position.y + dimensions.y / 2.f, position.z + dimensions.z / 2.f));
		if (result)
			return result;
	}
	// then the 4 right-side corners
	{
		result = CheckPoint(XMFLOAT3(pos_x + dim_x / 2.f, position.y - dimensions.y / 2.f, position.z - dimensions.z / 2.f));
		if (result)
			return result;
	}
	{
		result = CheckPoint(XMFLOAT3(pos_x + dim_x / 2.f, position.y - dimensions.y / 2.f, position.z + dimensions.z / 2.f));
		if (result)
			return result;
	}
	{
		result = CheckPoint(XMFLOAT3(pos_x + dim_x / 2.f, position.y + dimensions.y / 2.f, position.z - dimensions.z / 2.f));
		if (result)
			return result;
	}
	{
		result = CheckPoint(XMFLOAT3(pos_x + dim_x / 2.f, position.y + dimensions.y / 2.f, position.z + dimensions.z / 2.f));
		if (result)
			return result;
	}*/
	return false;//result;// 
}
