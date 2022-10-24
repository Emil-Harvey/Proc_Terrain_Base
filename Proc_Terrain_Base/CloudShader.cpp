#include "CloudShader.h"



CloudShader::CloudShader(ID3D11Device* device, HWND hwnd) : TextureShader(device, hwnd, true)
{
	//if (textured)
	//	initShader(L"texture_vs.cso", L"texture_ps.cso");
	//else
	initShader(L"shaders/texture_vs.cso",L"shaders/cloud_gs.cso", L"shaders/cloud_ps.cso");// 
}


CloudShader::~CloudShader()
{
	// Release the sampler state.
	if (sampleState)
	{
		sampleState->Release();
		sampleState = 0;
	}

	// Release the matrix constant buffer.
	if (matrixBuffer)
	{
		matrixBuffer->Release();
		matrixBuffer = 0;
	}

	// Release the layout.
	if (layout)
	{
		layout->Release();
		layout = 0;
	}

	//Release base shader components
	BaseShader::~BaseShader();
}


void CloudShader::initShader(const wchar_t* vsFilename, const wchar_t* gsFilename, const wchar_t* psFilename) //: BaseShader::initShader(vsFilename, psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC variableBufferDesc;//
	D3D11_BUFFER_DESC cameraBufferDesc;//
	D3D11_SAMPLER_DESC samplerDesc;

	// Load (+ compile) shader files
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);
	//loadGeometryShader(gsFilename);

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);//SECONDmatrixBuffer

	//
	
	// Setup the description of the VARIABLES buffer .
	variableBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	variableBufferDesc.ByteWidth = sizeof(timeBuffer);///
	variableBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	variableBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	variableBufferDesc.MiscFlags = 0;
	variableBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the ~ shader constant buffer from within this class.
	renderer->CreateBuffer(&variableBufferDesc, NULL, &variableBuffer);

	// Setup the description of the Camera buffer .
	cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraBufferDesc.ByteWidth = sizeof(PositionBufferType);///
	cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cameraBufferDesc.MiscFlags = 0;
	cameraBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the geo shader constant buffer from within this class.
	renderer->CreateBuffer(&cameraBufferDesc, NULL, &cameraBuffer);

	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;///D3D11_FILTER_MIN_MAG_MIP_POINT;///        // anti-aliasing mode.
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	renderer->CreateSamplerState(&samplerDesc, &sampleState);

}


void CloudShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* texture, float t)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	XMMATRIX tworld, tview, tproj;


	// Transpose the matrices to prepare them for the shader.
	tworld = XMMatrixTranspose(worldMatrix);
	tview = XMMatrixTranspose(viewMatrix);
	tproj = XMMatrixTranspose(projectionMatrix);
	
	// Send  matrix data
	result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;//tview;
	dataPtr->projection = tproj;
	deviceContext->Unmap(matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);
	//deviceContext->GSSetConstantBuffers(1, 1, &matrixBuffer);
	

	// Send pos data to geo shader
	PositionBufferType* posPtr;
	deviceContext->Map(cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	posPtr = (PositionBufferType*)mappedResource.pData;
	auto inverse = XMMatrixInverse(&XMMatrixDeterminant(viewMatrix), viewMatrix);
	XMStoreFloat4(&posPtr->position, inverse.r[3]);

	deviceContext->Unmap(cameraBuffer, 0);
	//deviceContext->GSSetConstantBuffers(2, 1, &cameraBuffer);

	// Send variable data
	result = deviceContext->Map(variableBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	timeBuffer* varsPtr = (timeBuffer*)mappedResource.pData;
	varsPtr->time = t - int(t);
	varsPtr->timeOfYear = t;
	varsPtr->day = int(t);
	deviceContext->Unmap(variableBuffer, 0);
	//deviceContext->GSSetConstantBuffers(0, 1, &variableBuffer);
	deviceContext->PSSetConstantBuffers(0, 1, &variableBuffer);


	// Set shader texture and sampler resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &texture);
	deviceContext->PSSetSamplers(0, 1, &sampleState);
}