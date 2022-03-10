#pragma once
#include "BaseShader.h"
#include "ShaderVariables.h"

class ComputeLandscape :
    public BaseShader
{
	struct TerrainDataBufferType {
		XMFLOAT2 seed;

		float Scale;
		float TimeOfYear;
		int TessellationFactor;
		float LODnear;
		float LODfar;
		float Amplitude;

		XMFLOAT2 GlobalPosition;
		XMFLOAT2 padding = { 0,0 }; /// maybe just use ShaderVariables??
	};

public:
	ComputeLandscape(ID3D11Device* device, HWND hwnd, int w, int h);
	~ComputeLandscape();

	void setShaderParameters(ID3D11DeviceContext* dc, ID3D11ShaderResourceView* texture1, ShaderVariables* SVars);
	void createOutputUAV();
	ID3D11ShaderResourceView* getSRV() { return m_srvTexOutput; };
	void unbind(ID3D11DeviceContext* dc);


private:
	void initShader(const wchar_t* cfile, const wchar_t* blank);

	ID3D11ShaderResourceView* srv;
	ID3D11UnorderedAccessView* uav;

	// texture set
	ID3D11Texture2D* m_tex;
	ID3D11UnorderedAccessView* m_uavAccess;
	ID3D11ShaderResourceView* m_srvTexOutput;

	int sWidth;
	int sHeight;

	///
	ID3D11Buffer* chunkBuffer;
};

