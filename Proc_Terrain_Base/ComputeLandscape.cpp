#include "ComputeLandscape.h"

ComputeLandscape::ComputeLandscape(ID3D11Device* device, HWND hwnd, int w, int h) : BaseShader(device, hwnd)
{
	sWidth = w;
	sHeight = h;
	initShader(L"shaders/landscape_cs.cso", NULL);
}

ComputeLandscape::~ComputeLandscape()
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

void ComputeLandscape::initShader(const wchar_t* cfile, const wchar_t* blank)
{
	loadComputeShader(cfile);
	createOutputUAV();
}

void ComputeLandscape::createOutputUAV()
{
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = sWidth;
	textureDesc.Height = sHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;//	 DXGI_FORMAT_R32G32B32_FLOAT; ?
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	m_tex = 0;
	renderer->CreateTexture2D(&textureDesc, 0, &m_tex);

	D3D11_UNORDERED_ACCESS_VIEW_DESC descUAV;
	ZeroMemory(&descUAV, sizeof(descUAV));
	descUAV.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; ;// DXGI_FORMAT_UNKNOWN;
	descUAV.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	descUAV.Texture2D.MipSlice = 0;
	renderer->CreateUnorderedAccessView(m_tex, &descUAV, &m_uavAccess);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	renderer->CreateShaderResourceView(m_tex, &srvDesc, &m_srvTexOutput);

	///	variables buffer
	D3D11_BUFFER_DESC chunkBufferDesc;
	chunkBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	chunkBufferDesc.ByteWidth = sizeof(TerrainDataBufferType);
	chunkBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	chunkBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	chunkBufferDesc.MiscFlags = 0;
	chunkBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&chunkBufferDesc, NULL, &chunkBuffer);
}

void ComputeLandscape::setShaderParameters(ID3D11DeviceContext* dc, ID3D11ShaderResourceView* texture1, ShaderVariables* SVars)
{
	dc->CSSetShaderResources(0, 1, &texture1);

	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	result = dc->Map(chunkBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	TerrainDataBufferType* tessPtr;
	tessPtr = (TerrainDataBufferType*)mappedResource.pData;
	tessPtr->TessellationFactor = SVars->TessellationFactor; // tessellation Factor;
	tessPtr->Amplitude = SVars->Amplitude;// amplitude;
	tessPtr->LODfar = SVars->LODfar;// LOD near threshold (distance)
	tessPtr->LODnear = SVars->LODnear;// LOD far threshold (distance)
	tessPtr->seed = SVars->seed;// seed/noise Offset
	tessPtr->TimeOfYear = SVars->TimeOfYear;
	tessPtr->Scale = SVars->Scale;
	tessPtr->GlobalPosition = SVars->GlobalPosition;
	tessPtr->padding.x = SVars->PlanetDiameter;
	//tessPtr->padding.y = 
	dc->Unmap(chunkBuffer, 0);

	dc->CSSetConstantBuffers(0, 1, &chunkBuffer);// 

	dc->CSSetUnorderedAccessViews(0, 1, &m_uavAccess, 0);
}

void ComputeLandscape::unbind(ID3D11DeviceContext* dc)
{
	ID3D11ShaderResourceView* nullSRV[] = { NULL };
	dc->CSSetShaderResources(0, 1, nullSRV);

	// Unbind output from compute shader
	ID3D11UnorderedAccessView* nullUAV[] = { NULL };
	dc->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);

	// Disable Compute Shader
	dc->CSSetShader(nullptr, nullptr, 0);
}