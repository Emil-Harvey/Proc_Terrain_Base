// Application.h
#ifndef _APP1_H
#define _APP1_H

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
	void depthPass();
	void renderDOF();
	void firstPass();
	void finalPass();
	void gui();
	void initTextures();

	float lerp(float a,float b,float x){
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
	TessellationPlane* m_Terrain;// main terrain tess-mesh
	TessellationPlane* x_Terrain;// neighbour terrain on the x axis (+ or -)
	TessellationPlane* z_Terrain;// neighbour on the z axis (north/south)
	TessellationPlane* xz_Terrain;// diagonal neighbour 
	const float xz_TerrainMeshOffset = -1920;//	(half of terrain width - 1920 @ 16 res): 3840x3 = 11520 

	TessellationPlane* far_Terrain;// this will be 3x the size

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
	int terrainResolution = 16;
	int octaves = 3;
	float xOffset = 640;
	float yOffset = 640;
	float scale = 1;
	float height = 1;
	// misc
	float test = 0.1;
	// TOD sim
	float time = 11.00;
	bool unpaused = true;
	//float timeOfYear = 54.0;//
	float timescale = 1.0;
	// bools
	bool floraOn = false;
	bool renderLODchunks = false;
	bool autoSmooth = true;
	bool DepthOfField;
	//
	float mountainPoint = 76.0;
	float minReducedHeight = 38;
	float maxIncreasedHeight = 180;
	// erosion
	int erosionRadius = 5;
	float erosionRate = 0.4f;
	// tessellation
	float LODnear= 0;
	float LODfar= 20.0;
	int tessellationFactor =4;
	float waterAmplitude = 1.5;
	//sphere
	XMFLOAT3 testPosition = { 0,-16,0 };
	bool gameSettingsMenuActive = false;
	//
	///
	
	///
	// 'map'
	OrthoMesh* mapMesh;
	RenderTexture* mapRenderTexture;
	int mapResolution = 1500;
	int mapZoom = 6;// bigger means see more
	//
	ID3D11ShaderResourceView* curHeightmapSRV;
};

#endif

