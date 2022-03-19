#include "TnDepthShader.h"

TnDepthShader::TnDepthShader(ID3D11Device* device, HWND hwnd) : DepthShader(device, hwnd)
{
	initShader(L"tess_vs.cso", L"tess_hs.cso", L"tessDepth_ds.cso", L"depth_ps.cso");
}

void TnDepthShader::initShader(const wchar_t* vsFilename, const wchar_t* hsFilename, const wchar_t* dsFilename, const wchar_t* psFilename)
{
	// InitShader must be overwritten and it will load both vertex and pixel shaders + setup buffers
	DepthShader::initShader(vsFilename, psFilename);
	//		Load other required shaders.
	loadHullShader(hsFilename);
	loadDomainShader(dsFilename);

	D3D11_BUFFER_DESC cameraBufferDesc;
	//setup camera buffer
	cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraBufferDesc.ByteWidth = sizeof(CameraBufferType);
	cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cameraBufferDesc.MiscFlags = 0;
	cameraBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&cameraBufferDesc, NULL, &cameraBuffer);

}

void TnDepthShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* displacementMap, XMFLOAT4 TessellationDetails, Camera* camera)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;

	// Transpose the matrices to prepare them for the shader.
	XMMATRIX tworld = XMMatrixTranspose(worldMatrix);
	XMMATRIX tview = XMMatrixTranspose(viewMatrix);
	XMMATRIX tproj = XMMatrixTranspose(projectionMatrix);

	// Send camera data to hull shader
	CameraBufferType* camPtr;
	deviceContext->Map(cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	camPtr = (CameraBufferType*)mappedResource.pData;
	camPtr->position = camera->getPosition();
	camPtr->padding = 0.0f;
	deviceContext->Unmap(cameraBuffer, 0);
	deviceContext->HSSetConstantBuffers(1, 1, &cameraBuffer);// needed for LOD


	deviceContext->Map(manBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	ManipulationBufferType* manPtr;
	manPtr = (ManipulationBufferType*)mappedResource.pData;
	manPtr->tessellationFactor = (int)TessellationDetails.x;
	manPtr->extra.x = TessellationDetails.y;//	amplitude
	manPtr->extra.y = TessellationDetails.z;//	LOD near
	manPtr->extra.z = TessellationDetails.w;//  LOD far
	deviceContext->Unmap(manBuffer, 0);
	deviceContext->HSSetConstantBuffers(0, 1, &manBuffer);//
	//deviceContext->DSSetConstantBuffers(1, 1, &manBuffer);//

	deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	deviceContext->Unmap(matrixBuffer, 0);

	deviceContext->DSSetConstantBuffers(0, 1, &matrixBuffer);



	// for heightmapping
	deviceContext->DSSetShaderResources(0, 1, &displacementMap);
	deviceContext->DSSetSamplers(0, 1, &sampleState);


}