#include "TessShader.h"


TessShader::TessShader(ID3D11Device* device, HWND hwnd, bool includeGeoShader) : LightShader(device, hwnd)
{
	GeoShaderActive = includeGeoShader;
	initShader(L"tess_vs.cso", L"tess_hs.cso", L"water_ds.cso", L"tree_gs.cso", L"water_ps.cso");

}

void TessShader::initShader(const wchar_t* vsFilename, const wchar_t* hsFilename, const wchar_t* dsFilename, const wchar_t* gsFilename, const wchar_t* psFilename)
{
	// InitShader must be overwritten and it will load both vertex and pixel shaders + setup buffers

	D3D11_BUFFER_DESC matrixBufferDesc, lightBufferDesc, cameraBufferDesc, geoBufferDesc, chunkBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;

	//		Load (+ compile) shader files
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);//
	//		Load other required shaders.
	loadHullShader(hsFilename);

	if (!GeoShaderActive) {
		loadDomainShader(dsFilename);
	}
	else {//		Load the geo shader optionally
		loadDomainShader(L"grass_ds.cso");
		loadGeometryShader(gsFilename);
	}


	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);

	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	renderer->CreateSamplerState(&samplerDesc, &sampleState);


	// tess buffer!
	chunkBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	chunkBufferDesc.ByteWidth = sizeof(DataBufferType);
	chunkBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	chunkBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	chunkBufferDesc.MiscFlags = 0;
	chunkBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&chunkBufferDesc, NULL, &chunkBuffer);

	// Setup light buffer
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&lightBufferDesc, NULL, &lightBuffer);

	//setup camera buffer
	cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraBufferDesc.ByteWidth = sizeof(CameraBufferType);
	cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cameraBufferDesc.MiscFlags = 0;
	cameraBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&cameraBufferDesc, NULL, &cameraBuffer);


}

constexpr XMFLOAT3 directions[6] = { {1,0,0},{-1,0,0},// right and left
									 {0,1,0},{0,-1,0},// up and down
									 {0,0,1},{0,0,-1} };//fwd & bwd

void TessShader::setShaderParameters(ID3D11DeviceContext* dc, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* texture, Light* light, Camera* camera, XMFLOAT4 TD , float time)// this shader has tessellation, texture heightmapping and geometry generation (grass)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	XMMATRIX tworld, tview, tproj;


	// Transpose the matrices to prepare them for the shader.
	tworld = XMMatrixTranspose(worldMatrix);
	tview = XMMatrixTranspose(viewMatrix);
	tproj = XMMatrixTranspose(projectionMatrix);
	

	// Send data to hull shader
	result = dc->Map(chunkBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	DataBufferType* tessPtr;
	tessPtr = (DataBufferType*)mappedResource.pData;
	tessPtr->factor = (int)TD.x;// tessellation Factor;
	tessPtr->extra.x = TD.y;// amplitude;
	tessPtr->extra.y = TD.w;// LOD near threshold (distance)
	tessPtr->extra.z = TD.z;// LOD far threshold (distance)
	tessPtr->position = { 0.0,0.0 };// offset?
	tessPtr->time = time;
	dc->Unmap(chunkBuffer, 0);
	dc->HSSetConstantBuffers(0, 1, &chunkBuffer);// hs
	dc->DSSetConstantBuffers(1, 1, &chunkBuffer);

	// Lock the constant buffer so it can be written to.
	result = dc->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	MatrixBufferType* dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	dc->Unmap(matrixBuffer, 0);
	dc->VSSetConstantBuffers(0, 1, &matrixBuffer);
	dc->DSSetConstantBuffers(0, 1, &matrixBuffer);
	if (GeoShaderActive) dc->GSSetConstantBuffers(0, 1, &matrixBuffer);//


	// Send camera data to pixel shader
	CameraBufferType* camPtr;
	dc->Map(cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	camPtr = (CameraBufferType*)mappedResource.pData;
	camPtr->position = camera->getPosition();
	camPtr->padding = time;
	dc->Unmap(cameraBuffer, 0);
	dc->HSSetConstantBuffers(1, 1, &cameraBuffer);// needed for LOD
	dc->DSSetConstantBuffers(2, 1, &cameraBuffer);// needed for LOD
	dc->PSSetConstantBuffers(1, 1, &cameraBuffer);// for specular

	///*
	// Send light data to pixel shader
	LightBufferType* lightPtr;
	dc->Map(lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	lightPtr = (LightBufferType*)mappedResource.pData;
	lightPtr->ambient = light->getAmbientColour();
	lightPtr->diffuse = light->getDiffuseColour();
	lightPtr->direction = light->getDirection();
	lightPtr->specular = light->getSpecularPower();
	dc->Unmap(lightBuffer, 0);
	dc->PSSetConstantBuffers(0, 1, &lightBuffer);

	// Set shader texture resource in the pixel shader.
	dc->PSSetShaderResources(0, 1, &texture);

	dc->PSSetSamplers(0, 1, &sampleState);
	//*/

}