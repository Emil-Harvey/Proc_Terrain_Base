#include "TerrainShader.h"

TerrainShader::TerrainShader(ID3D11Device* device, HWND hwnd) : TessShader(device, hwnd)
{
	initShader(L"shaders/tess_vs.cso", L"shaders/tess_hs.cso", L"shaders/terrain_noise_ds.cso", L"shaders/tree_gs.cso", L"shaders/light_ps.cso");
}
void TerrainShader::initShader(const wchar_t* vsFilename, const wchar_t* hsFilename, const wchar_t* dsFilename, const wchar_t* gsFilename, const wchar_t* psFilename)
{
	// InitShader must be overwritten and it will load both vertex and pixel shaders + setup buffers

	D3D11_BUFFER_DESC matrixBufferDesc, lightBufferDesc, cameraBufferDesc, geoBufferDesc, chunkBufferDesc;
	D3D11_SAMPLER_DESC textureSamplerDesc, heightmapSamplerDesc;

	//		Load (+ compile) shader files
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);//
	//		Load other required shaders.
	loadHullShader(hsFilename);

	if (GeoShaderActive) {//<-- relevant?
		//		Load the geo shader optionally
		loadDomainShader(L"shaders/grass_ds.cso");
		loadGeometryShader(gsFilename);
	}
	else {
		loadDomainShader(dsFilename);
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

	// Create a heightmap sampler state description.
	heightmapSamplerDesc.Filter = D3D11_FILTER_ANISOTROPIC; // heightmap must have good filtering to avoid blocky terrain
	heightmapSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;//D3D11_TEXTURE_ADDRESS_CLAMP;
	heightmapSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;//D3D11_TEXTURE_ADDRESS_CLAMP;
	heightmapSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;//D3D11_TEXTURE_ADDRESS_CLAMP;
	heightmapSamplerDesc.BorderColor[0] = -10.0f;
	heightmapSamplerDesc.BorderColor[1] = -10.0f;
	heightmapSamplerDesc.BorderColor[2] = -10.0f;
	heightmapSamplerDesc.BorderColor[3] = -10.0f;
	heightmapSamplerDesc.MipLODBias = 0.0f;
	heightmapSamplerDesc.MaxAnisotropy = 1;
	heightmapSamplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	heightmapSamplerDesc.MinLOD = 0;
	heightmapSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	renderer->CreateSamplerState(&heightmapSamplerDesc, &heightmapSampleState);

	// Create a texture sampler state description.
	textureSamplerDesc.Filter = D3D11_FILTER_ANISOTROPIC; //D3D11_FILTER_MIN_MAG_MIP_POINT;
	textureSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	textureSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	textureSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	textureSamplerDesc.MipLODBias = 0.0f;
	textureSamplerDesc.MaxAnisotropy = 1;
	textureSamplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	textureSamplerDesc.MinLOD = 0;
	textureSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	renderer->CreateSamplerState(&textureSamplerDesc, &sampleState);

	// tess buffer!
	chunkBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	chunkBufferDesc.ByteWidth = sizeof(TerrainDataBufferType);
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
void TerrainShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView** textures, Light* light, FPCamera* camera, ShaderVariables* SVars, ID3D11ShaderResourceView* heightmap)
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
	//deviceContext->DSSetConstantBuffers(2, 1, &matrixBuffer);
	deviceContext->DSSetConstantBuffers(0, 1, &matrixBuffer);// DS 0

	//Additional
	// 
		// Send data to hull shader
	result = deviceContext->Map(chunkBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	TerrainDataBufferType* tessPtr;
	tessPtr = (TerrainDataBufferType*)mappedResource.pData;
	tessPtr->seed = SVars->seed;//n_Offset;// seed/noise Offset //{ 0.0,0.0 };
	tessPtr->Scale = SVars->Scale;//scale;
	tessPtr->TimeOfYear = SVars->TimeOfYear;//timeOfYear;
	tessPtr->TessellationFactor = SVars->TessellationFactor; //(int)TD.x;// tessellation Factor;
	tessPtr->LODfar = SVars->LODfar;//TD.w;// LOD near threshold (distance)
	tessPtr->LODnear = SVars->LODnear;//TD.z;// LOD far threshold (distance)
	tessPtr->Amplitude = SVars->Amplitude;//TD.y;// amplitude;
	tessPtr->GlobalPosition = SVars->GlobalPosition;
	tessPtr->PlanetDiameter = SVars->PlanetDiameter;

#ifdef CPU_TERRAIN_ENABLED
	tessPtr->padding = 1;
#else
	tessPtr->padding = -1;
#endif // CPU_TERRAIN_ENABLED

	deviceContext->Unmap(chunkBuffer, 0);

	deviceContext->HSSetConstantBuffers(0, 1, &chunkBuffer);// hs
	deviceContext->DSSetConstantBuffers(1, 1, &chunkBuffer);// DS 1
	deviceContext->PSSetConstantBuffers(2, 1, &chunkBuffer);

	// Send light data to pixel shader
	LightBufferType* lightPtr;
	deviceContext->Map(lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	lightPtr = (LightBufferType*)mappedResource.pData;
	lightPtr->ambient = light->getAmbientColour();
	lightPtr->diffuse = light->getDiffuseColour();
	lightPtr->direction = light->getDirection();
	lightPtr->specular = light->getSpecularPower();
	deviceContext->Unmap(lightBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &lightBuffer);

	// Send camera data to pixel shader
	CameraBufferType* camPtr;
	deviceContext->Map(cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	camPtr = (CameraBufferType*)mappedResource.pData;
	camPtr->position = camera->getPosition();
	camPtr->padding = 0.0f;
	deviceContext->Unmap(cameraBuffer, 0);
	deviceContext->HSSetConstantBuffers(1, 1, &cameraBuffer);
	deviceContext->DSSetConstantBuffers(2, 1, &cameraBuffer); 
	deviceContext->PSSetConstantBuffers(1, 1, &cameraBuffer);

	//// Send chunk data to pixel shader
	//DataBufferType* chunkPtr;
	//deviceContext->Map(chunkBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	//chunkPtr = (DataBufferType*)mappedResource.pData;
	////chunkPtr->position = 0;//chunkPos;
	//chunkPtr->scale = scale;
	//chunkPtr->time = timeOfYear;
	//deviceContext->Unmap(chunkBuffer, 0);
	//deviceContext->VSSetConstantBuffers(1, 1, &chunkBuffer);
	//deviceContext->PSSetConstantBuffers(2, 1, &chunkBuffer);

	/// set shader resources for DS
	deviceContext->DSSetShaderResources(0, 1, &heightmap);
	deviceContext->DSSetSamplers(0, 1, &heightmapSampleState);

	/// Set shader texture resource in the pixel shader.
	//for (int i = 1; i < 36; i++) {
		deviceContext->PSSetShaderResources(0, 17, &textures[0]);
		//deviceContext->VSSetShaderResources(0, i, &textures[0]);// ideally just heightmaps
	//}

	deviceContext->PSSetSamplers(0, 1, &sampleState);
}

void TerrainShader::unbind(ID3D11DeviceContext* deviceContext)// I thought this would solve the problem but it doesn't (?)
{
	ID3D11ShaderResourceView* nullSRV[] = { NULL };
	deviceContext->DSSetShaderResources(0, 1, nullSRV);
}


