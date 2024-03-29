// Application.h
#pragma once


// Includes
#include "DXF.h"	// include dxframework
#include "LightShader.h"
#include "TextureShader.h"
#include "TerrainMesh.h"
#include "QuadMesh.h"
#include "Grower.h"
#include "TessShader.h"
#include "TessellationPlane.h"
#include "TextureShader.h"
#include "CloudShader.h"
#include "TerrainShader.h"//<-- tesselating terrain mesh
#include "ShaderVariables.h"
#include "TnDepthShader.h"
#include "ComputeBlurHor.h"
#include "ComputeBlurVert.h"
#include "ComputeLandscape.h"
#include "ComputeErosion.h"
#include "QuadTreeMesh.h"//"QuadTreeClass.h"
#include "CreateTGA.h"
//#include <wrl\client.h>// ComPtr
#include <array>
#include "Noise.h"
#include <thread>

class App1 : public BaseApplication
{
public:

	App1();
	~App1();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();

protected:

	bool render();
	bool renderMinimap(bool erode_as_well = true);
	bool erodeTerrain();// maybe move this to other class?
	//void depthPass();
	void renderDOF();
	void firstPass();
	void finalPass();
	void gui();
	void initTextures();

	void TransferHeightmapToCPU();
	void CreateHeightmapOnCPU();
	void QuadtreeHeightmap( QuadtreeNode* node_to_render, const char* filename = "Exported Data/test.tga", bool isRAW = false);//const float size, const XMFLOAT2 position
	void RecursiveHeightmapExport(QuadtreeNode* node_to_render, string namePrefix = "Exported Data/0000");
	void lammmbda(int a, int b);

	static inline float lerp(const float& a, const float &b, const float &x){
		return (a + x * (b - a));
	}

private:
	LightShader* defaultShader;
	TextureShader* sunShader;
	Grower* treeShader;
	TessShader* waterShader;
	TextureShader* textureShader;
	TerrainShader* terrainShader;
	CloudShader* cloudShader;//
	TnDepthShader* depthShader;

	ComputeBlurHor* horBlur;
	ComputeBlurVert* verBlur;
	RenderTexture* preDOFRT;
	ShadowMap* cameraDepth;

	ComputeLandscape* csLand;
	ComputeErosion* csErosion;

	PlaneMesh *f_Terrain;// used for foliage

	QuadTreeMesh* qt_Terrain;//QuadTreePlane* qt_Terrain;

	TessellationPlane* m_Water;
	PlaneMesh* m_clouds;
	int Water_Mesh_Res = 48;
	SphereMesh* sun_mesh;
	SphereMesh* sky_sphere;

	Light* light;

	ShaderVariables vars;

	ID3D11ShaderResourceView* textures[18];
	ID3D11ShaderResourceView* trees[4];
	ID3D11ShaderResourceView* cloudTexture;
	ID3D11ShaderResourceView* rainTexture;
	ID3D11ShaderResourceView* macroTexture;

	// terrain gen 
	///const int terrainResolution = 16;
	///int octaves = 3;

	// misc
	float test = 0.1f;
	// TOD sim
	float time = 11.00f;
	bool unpaused = true;
	//float timeOfYear = 54.0;//
	float timescale = 1.0f;
	// bools
	bool floraOn = false;
	bool orthoCameraEnabled = false;
	bool chunkUpdatesEnabled = true;
	//bool autoSmooth = true;
	bool DepthOfField;
	bool erosion_enabled = false;
	bool isHeightmapRAW = false;
	//
	///float mountainPoint = 76.0;
	///float minReducedHeight = 38;
	///float maxIncreasedHeight = 180;
	// erosion
	///int erosionRadius = 5;
	///float erosionRate = 0.4f;
	// water mesh
	const float LODnear= 0;
	const float LODfar= 20.0f;
	const int tessellationFactor =6;
	const float waterAmplitude = 1.5f;
	//sphere
	//XMFLOAT3 testPosition = { 0,-16,0 };
	bool gameSettingsMenuActive = false;
	//
	///
	
	///
	// 'map'
	OrthoMesh* mapMesh;
	RenderTexture* mapRenderTexture;
	const int mapResolution = 1500;
	int mapZoom = 6;// bigger means see more
	//
	ID3D11ShaderResourceView* curHeightmapSRV;
	D3D11_MAPPED_SUBRESOURCE heightmap_mappedResource;
	const int MAP_DIM =4096; // 512;//
	//std::array<XMFLOAT4, 1600 * 1600> pixelData{};
	std::array<uint8_t, 4096 * 4096 * 16> pixelData{};//512*512*16> pixelData{};//
};



