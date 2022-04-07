#include "Grower.h"


Grower::Grower(ID3D11Device* device, HWND hwnd) :
	LightShader(device, hwnd)
{
	initShader(L"tree_vs.cso",L"tree_gs.cso", L"tree_ps.cso");
}
void Grower::initShader(const wchar_t* vsFilename, const wchar_t* gsFilename, const wchar_t* psFilename)
{
	LightShader::initShader(vsFilename, psFilename);

	loadGeometryShader(gsFilename);
}


void Grower::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* extra_tex, ID3D11ShaderResourceView* tex[], Light* light, FPCamera* camera, ShaderVariables* SVars, ID3D11ShaderResourceView* heightmap, bool trees)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;

	XMMATRIX tworld, tview, tproj;


	// Transpose the matrices to prepare them for the shader.
	tworld = XMMatrixTranspose(world);
	tview = XMMatrixTranspose(view);
	tproj = XMMatrixTranspose(projection);
	result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	deviceContext->Unmap(matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);
	deviceContext->GSSetConstantBuffers(0, 1, &matrixBuffer);

	

	// Send pos data to geo shader
	CameraBufferType* posPtr;
	deviceContext->Map(cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	posPtr = (CameraBufferType*)mappedResource.pData;
	auto inverse = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMStoreFloat3(&posPtr->position, inverse.r[3]);// position of camera
	// Tell Vert shader whether rendering grass (dense) or trees
	if (trees)
		posPtr->padding = 15;
	else// grass
		posPtr->padding = 5;

	deviceContext->Unmap(cameraBuffer, 0);
	deviceContext->VSSetConstantBuffers(1, 1, &cameraBuffer);
	deviceContext->GSSetConstantBuffers(2, 1, &cameraBuffer);

	// Send scale data to geo shader
	DataBufferType* infoPtr;
	deviceContext->Map(chunkBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	infoPtr = (DataBufferType*)mappedResource.pData;
	infoPtr->position = SVars->seed;
	infoPtr->scale = SVars->Scale;
	infoPtr->time = SVars->TimeOfYear;
	infoPtr->factor = SVars->TessellationFactor; // resolution;? 
	infoPtr->extra.x = SVars->GlobalPosition.x;
	infoPtr->extra.y = SVars->GlobalPosition.y;
	infoPtr->extra.z = SVars->PlanetDiameter;
	
	deviceContext->Unmap(chunkBuffer, 0);
	deviceContext->GSSetConstantBuffers(1, 1, &chunkBuffer);
	//deviceContext->VSSetConstantBuffers(1, 1, &chunkBuffer);
	deviceContext->PSSetConstantBuffers(1, 1, &chunkBuffer);

	//Additional
	// Send light data to pixel shader
	LightBufferType* lightPtr;
	deviceContext->Map(lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	lightPtr = (LightBufferType*)mappedResource.pData;
	lightPtr->ambient = light->getAmbientColour();
	lightPtr->diffuse = light->getDiffuseColour();
	lightPtr->direction = light->getDirection();
	lightPtr->specular = 0.0f;
	deviceContext->Unmap(lightBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &lightBuffer);

	deviceContext->VSSetShaderResources(0, 1, &heightmap);
	deviceContext->VSSetSamplers(0, 1, &sampleState);

	// Set shader texture resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &extra_tex);
	deviceContext->PSSetShaderResources(1, 4, tex);
	deviceContext->PSSetSamplers(0, 1, &sampleState);
}

