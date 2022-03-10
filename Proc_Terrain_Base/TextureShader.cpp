#include "TextureShader.h"



TextureShader::TextureShader(ID3D11Device* device, HWND hwnd, bool textured) : BaseShader(device, hwnd)
{
	if (textured)
		initShader(L"texture_vs.cso", L"texture_ps.cso");
	else
		initShader(L"texture_vs.cso", L"sun_ps.cso");// sky / sun mode
}


TextureShader::~TextureShader()
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


void TextureShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC variableBufferDesc;//
	D3D11_SAMPLER_DESC samplerDesc;

	// Load (+ compile) shader files
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);//SECONDmatrixBuffer

	renderer->CreateBuffer(&matrixBufferDesc, NULL, &SECONDmatrixBuffer);

	// Setup the description of the VARIABLES buffer .
	variableBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	variableBufferDesc.ByteWidth = sizeof(memeBuffer);///
	variableBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	variableBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	variableBufferDesc.MiscFlags = 0;
	variableBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the ~ shader constant buffer from within this class.
	renderer->CreateBuffer(&variableBufferDesc, NULL, &variableBuffer);

	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;///MIN_MAG_MIP_POINT;///        // anti-aliasing mode.
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;///NEVER; ?
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	renderer->CreateSamplerState(&samplerDesc, &sampleState);

}


void TextureShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* texture, float t)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	XMMATRIX tworld, tview, tproj;


	// Transpose the matrices to prepare them for the shader.
	tworld = XMMatrixTranspose(worldMatrix);
	tview = XMMatrixTranspose(viewMatrix);
	tproj = XMMatrixTranspose(projectionMatrix);
	;
	// Send (ortho) matrix data
	result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = (t >= 0) ? XMMatrixIdentity() + XMMATRIX(XMVECTOR({ 0 }), XMVECTOR({ 0 }), XMVECTOR({ 0, 0, 0, 10.0 }), XMVECTOR({ 0 })) : tview;//tview;
	dataPtr->projection = tproj;
	deviceContext->Unmap(matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);
	// Send matrix data
	result = deviceContext->Map(SECONDmatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	deviceContext->Unmap(SECONDmatrixBuffer, 0);

	deviceContext->PSSetConstantBuffers(1, 1, &SECONDmatrixBuffer);

	// Send variable data
	result = deviceContext->Map(variableBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memeBuffer* varsPtr = (memeBuffer*)mappedResource.pData;
	varsPtr->time = t - int(t);
	varsPtr->timeOfYear = t;
	varsPtr->day = int(t);
	deviceContext->Unmap(variableBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &variableBuffer);


	// Set shader texture and sampler resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &texture);
	deviceContext->PSSetSamplers(0, 1, &sampleState);
}