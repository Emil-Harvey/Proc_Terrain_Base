#include "App1.h"
//#include <cmath>
App1::App1()
{
	light = nullptr;
	defaultShader = nullptr;
	m_Water = nullptr;
	treeShader = nullptr;
	waterShader = nullptr;
	textureShader = nullptr;
	terrainShader = nullptr;
	sunShader = nullptr;
	cloudShader = nullptr;
	horBlur = nullptr;
	verBlur = nullptr;
	csLand = nullptr;
	csErosion = nullptr;
	mapMesh = nullptr;
	mapRenderTexture = nullptr;//
	m_Terrain = nullptr;
	x_Terrain = nullptr;
	z_Terrain = nullptr;
	xz_Terrain = nullptr;
	far_Terrain = nullptr;
	m_clouds = nullptr;
	f_Terrain = nullptr;
	sun_mesh = nullptr;
	sky_sphere = nullptr;
	curHeightmapSRV = nullptr;
}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	// Load textures     -- PBR textures courtesy of freepbr.com
	initTextures();

	/// Create Mesh object and shader objects
		// Create geometry object
	//terrains[0] = new TerrainMesh(renderer->getDevice(), renderer->getDeviceContext(), terrainResolution, 400, {-200,-200});// central chunk
	//
	//terrains[0]->initRadialEffect(erosionRadius);
	
	m_Water = new TessellationPlane(renderer->getDevice(), renderer->getDeviceContext(), Water_Mesh_Res);
	sun_mesh = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext(), 10);
	sky_sphere = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext(), 8);

	// Load and initialise required shader handle objects.
	defaultShader = new LightShader(renderer->getDevice(), hwnd);// may be defunct
	sunShader = new TextureShader(renderer->getDevice(), hwnd, false);
	treeShader = new Grower(renderer->getDevice(), hwnd);
	waterShader = new TessShader(renderer->getDevice(), hwnd);
	terrainShader = new TerrainShader(renderer->getDevice(), hwnd);
	horBlur = new ComputeBlurHor(renderer->getDevice(), hwnd, screenWidth, screenHeight);
	verBlur = new ComputeBlurVert(renderer->getDevice(), hwnd, screenWidth, screenHeight);
	csLand = new ComputeLandscape(renderer->getDevice(), hwnd, 1500, 1500);
	csErosion = new ComputeErosion(renderer->getDevice(), hwnd, 1500, 1500);

	
	// minimap
	mapMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth, screenHeight, 0, 0);
	mapRenderTexture = new RenderTexture(renderer->getDevice(), 1500, 1500, SCREEN_NEAR, SCREEN_DEPTH);
	textureShader = new TextureShader(renderer->getDevice(), hwnd);

	cloudShader = new CloudShader(renderer->getDevice(), hwnd);

	// initialise RT objects. 
	preDOFRT = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, /*400,300,*/ 0.1f, 200.f);
	cameraDepth = new ShadowMap(renderer->getDevice(), screenWidth, screenHeight);

	m_Terrain = new TessellationPlane(renderer->getDevice(), renderer->getDeviceContext(), terrainResolution);
	x_Terrain = new TessellationPlane(renderer->getDevice(), renderer->getDeviceContext(), terrainResolution);
	z_Terrain = new TessellationPlane(renderer->getDevice(), renderer->getDeviceContext(), terrainResolution);
	xz_Terrain = new TessellationPlane(renderer->getDevice(), renderer->getDeviceContext(), terrainResolution);
	far_Terrain = new TessellationPlane(renderer->getDevice(), renderer->getDeviceContext(), terrainResolution);

	f_Terrain = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext(), 280);

	m_clouds = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext(), 16);

	vars.LODfar = 53;//
	vars.Scale = 7;
	vars.PlanetDiameter = 127420;// a 1/1000th of earth
	vars.TessellationFactor = 5;
	vars.TimeOfYear = 54.0;

	// Initialise light
	light = new Light();
	light->setAmbientColour(0.040f, .0150f, 0.10f, 1.0f);
	light->setDirection(01.0f, -0.50f, 0.0f);
	light->setSpecularPower(208.0f);

	// set time of day-light colour
	//light->setDiffuseColour(lerp(0.6, 0.9, -0.5 * (cos(time / 3.81) - 1)), lerp(0.14, 0.85, -0.5 * (cos(time / 3.81) - 1)), lerp(0.01, 0.8, -0.5 * (cos(time / 3.81) - 1)), 1.0f);
	//light->setDirection(sin(time / 3.81), cos(time / 3.81), sin((time / 3.81) + 1.78) / 4);

	/// set camera speed to lower
	camera->speedScale = 10;

	renderMinimap();
}


App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	if (light)
	{
		delete light;
		light = 0;
	}
	
	if (f_Terrain)
	{
		delete f_Terrain;
		f_Terrain = 0;
	}
	
	if (m_Water)
	{
		delete m_Water;
		m_Water = 0;
	}
	if (m_Terrain)
	{
		delete m_Terrain;
		m_Terrain = 0;
	}
	if (sun_mesh)
	{
		delete sun_mesh;
		sun_mesh = 0;
	}
	if (sky_sphere)
	{
		delete sky_sphere;
		sky_sphere = 0;
	}

	if (defaultShader)
	{
		delete defaultShader;
		defaultShader = 0;
	}
	if (sunShader)
	{
		delete sunShader;
		sunShader = 0;
	}
	if (treeShader)
	{
		delete treeShader;
		treeShader = 0;
	}
	if (waterShader)
	{
		delete waterShader;
		waterShader = 0;
	}
	if (textureShader)
	{
		delete textureShader;
		textureShader = 0;
	}
	if (terrainShader)
	{
		delete terrainShader;
		terrainShader = 0;
	}
	if (horBlur)
	{
		delete horBlur;
		horBlur = 0;
	}
	if (verBlur)
	{
		delete verBlur;
		verBlur = 0;
	}
	if (csLand)
	{
		delete csLand;
		csLand = 0;
	}
	if (m_clouds){
		delete m_clouds;
		m_clouds = 0;
	}
}


bool App1::frame()
{
	bool result;

	result = BaseApplication::frame();
	if (!result){ return false; }

	while (time >= 24)  time -= 24; 
	if (vars.TimeOfYear >= 361) vars.TimeOfYear = 1;
	
	if (unpaused) {
		time += 0.01 * timescale * timer->getTime();// pass time
		vars.TimeOfYear += (0.01 / 24.f) * timer->getTime();

		
	}

	// Generate the view matrix based on the camera's position.
	camera->update();

	// Render the graphics.
	result = render();
	if (!result) { return false; }
	
	return true;
}
bool App1::render()
{
	//depthPass();
	//renderMinimap();///	<--	give this function inputs so fewer duplicate vars are calculated

	firstPass();

	if (camera->getPosition().y < -4) { DepthOfField = true; }
	else DepthOfField = false;
	if(DepthOfField)
		renderDOF();

	finalPass();
	
// Render GUI
	gui();

	// Swap the buffers
	renderer->endScene();

	return true;
}

//generate terrain in compute shader
bool App1::renderMinimap(bool erode_as_well)
{
	csLand->setShaderParameters(renderer->getDeviceContext(), curHeightmapSRV, &vars);
	csLand->compute(renderer->getDeviceContext(), 60, 60, 1);
	csLand->unbind(renderer->getDeviceContext());
	curHeightmapSRV = csLand->getSRV();

	if (erode_as_well)
		return erodeTerrain();

	return true;
}
bool App1::erodeTerrain()
{
	if (erosion_enabled) {
		csErosion->setShaderParameters(renderer->getDeviceContext(), curHeightmapSRV, &vars);
		csErosion->compute(renderer->getDeviceContext(), 80, 80, 1);
		csErosion->unbind(renderer->getDeviceContext());
		curHeightmapSRV = csErosion->getSRV();
	}
	return true;
}
//
//void App1::depthPass()
//{	/// OPTIMISE: move duplicate variables to render() function
//
//	cameraDepth->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());
//
//	
//	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
//	const XMMATRIX positionMatrix = XMMatrixTranslation(xz_TerrainMeshOffset, 0.0, xz_TerrainMeshOffset);
//	 
//	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
//	worldMatrix = renderer->getWorldMatrix();
//	viewMatrix = camera->getViewMatrix();
//	projectionMatrix = (renderLODchunks ? renderer->getOrthoMatrix() : renderer->getProjectionMatrix());// renderer->getOrthoMatrix();//getProjectionMatrix();//
//
//	static const XMMATRIX terrainScaleMatrix = XMMatrixScaling(256, 1.0, 256);// 4096.0 / terrainResolution // 4096 may be incorrect, it may also be x^2 or something
//	const XMMATRIX m_TerrainMatrix = XMMatrixMultiply(terrainScaleMatrix, positionMatrix);
//
//	/// main terrain
//	m_Terrain->sendData(renderer->getDeviceContext(), D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);//													
//	terrainShader->setShaderParameters(renderer->getDeviceContext(), XMMatrixMultiply(worldMatrix, m_TerrainMatrix), viewMatrix, projectionMatrix, textures, light, camera, &vars, /*verBlur->getSRV()*/csLand->getSRV());// XMFLOAT4(tessellationFactor, height * 100, LODnear, LODfar), scale, XMFLOAT2(xOffset, yOffset), timeOfYear);
//	terrainShader->render(renderer->getDeviceContext(), m_Terrain->getIndexCount());
//
//	/* foliage (geo shader)
//	if (floraOn) {
//		//renderer->setAlphaBlending(true);
//		f_Terrain->sendData(renderer->getDeviceContext());//worldMatrix//m_TerrainMatrix)
//		treeShader->setShaderParameters(renderer->getDeviceContext(), XMMatrixMultiply(worldMatrix, XMMatrixMultiply(XMMatrixScaling(5, 1.0, 5), XMMatrixTranslation(int(camera->getPosition().x) - 700.0, 0, int(camera->getPosition().z) - 700.0))), viewMatrix, projectionMatrix, trees, light, camera, &vars, csLand->getSRV());
//		treeShader->render(renderer->getDeviceContext(), f_Terrain->getIndexCount());
//		//renderer->setAlphaBlending(false);
//	}*/
//
//	/// neighbouring terrains
//	float xMeshOffset = xz_TerrainMeshOffset * 3;
//	float zMeshOffset = xz_TerrainMeshOffset * 3;
//	float x2MeshOffset= xz_TerrainMeshOffset * -1;
//	float z2MeshOffset= xz_TerrainMeshOffset * -1;
//	//const int chunk = 1920;
//	/*if (camera->getPosition().x < 0)// negative x (west)
//	{
//		xMeshOffset = xz_TerrainMeshOffset * 3;
//	}
//	else {// east
//		xMeshOffset = xz_TerrainMeshOffset * -1;
//	}
//	if (camera->getPosition().z < 0)// negative z (south)
//	{
//		zMeshOffset = xz_TerrainMeshOffset * 3;
//	}
//	else {// north
//		zMeshOffset = xz_TerrainMeshOffset * -1;
//	}*/
//	const XMMATRIX xPositionMatrix = XMMatrixTranslation(xMeshOffset, 0.0, xz_TerrainMeshOffset);// 
//	const XMMATRIX zPositionMatrix = XMMatrixTranslation(xz_TerrainMeshOffset, 0.0, zMeshOffset);// 
//	const XMMATRIX xzPositionMatrix = XMMatrixTranslation(xMeshOffset, 0.0, zMeshOffset);
//
//	
//
//	XMMATRIX neighbourWorldMatrix;
//
//	// X TERRAIN
//	neighbourWorldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixMultiply(terrainScaleMatrix, xPositionMatrix));
//	x_Terrain->sendData(renderer->getDeviceContext(), D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
//	terrainShader->setShaderParameters(renderer->getDeviceContext(), neighbourWorldMatrix, viewMatrix, projectionMatrix, textures, light, camera, &vars, csLand->getSRV());
//	terrainShader->render(renderer->getDeviceContext(), x_Terrain->getIndexCount());
//	// Z TERRAIN
//	neighbourWorldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixMultiply(terrainScaleMatrix, zPositionMatrix));
//	z_Terrain->sendData(renderer->getDeviceContext(), D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
//	terrainShader->setShaderParameters(renderer->getDeviceContext(), neighbourWorldMatrix, viewMatrix, projectionMatrix, textures, light, camera, &vars, csLand->getSRV());
//	terrainShader->render(renderer->getDeviceContext(), z_Terrain->getIndexCount());
//	// XZ TERRAIN
//	neighbourWorldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixMultiply(terrainScaleMatrix, xzPositionMatrix));
//	xz_Terrain->sendData(renderer->getDeviceContext(), D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
//	terrainShader->setShaderParameters(renderer->getDeviceContext(), neighbourWorldMatrix, viewMatrix, projectionMatrix, textures, light, camera, &vars, csLand->getSRV());//XMFLOAT4(tessellationFactor, height * 100, LODnear, LODfar), scale, XMFLOAT2(xOffset, yOffset), timeOfYear);
//	terrainShader->render(renderer->getDeviceContext(), xz_Terrain->getIndexCount());
//	
//	
//
//	// Set back buffer as render target and reset view port.
//	renderer->setBackBufferRenderTarget();
//	renderer->resetViewport();
//	
//
//}
//*/

void App1::firstPass()
{
	// Set the render target to be the render to texture.
	preDOFRT->setRenderTarget(renderer->getDeviceContext());
	preDOFRT->clearRenderTarget(renderer->getDeviceContext(), 0.10f, 0.10f, 0.10f, 1.0f);


	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	const XMMATRIX positionMatrix = XMMatrixTranslation(xz_TerrainMeshOffset, 0.0, xz_TerrainMeshOffset);
	//water
	static const XMMATRIX waterScaleMatrix = XMMatrixScaling(3 * 4096 / Water_Mesh_Res, 1.0, 3 * 4096 / Water_Mesh_Res);
	const XMMATRIX waterMatrix = XMMatrixMultiply(waterScaleMatrix, XMMatrixTranslation(xz_TerrainMeshOffset*3, 0.0, xz_TerrainMeshOffset*3));
	//terrain
	static const XMMATRIX terrainScaleMatrix = XMMatrixScaling(256, 1.0, 256);// 4096 may be incorrect, it may also be x^2 or something4096.0 / terrainResolution
	const XMMATRIX m_TerrainMatrix = XMMatrixMultiply(terrainScaleMatrix, positionMatrix);


	////////
	float lengthOfDay = -1 * cos(vars.TimeOfYear / 57.29577);// replace -6 [-1] w/ -12*sin(latitude)  <--- to be moved to PS?
///
	float sunAltitude = lengthOfDay - (cos(time / 3.81) / 0.8);// necessary for sun mesh position


// Clear the scene. 
	renderer->beginScene(0.3, 0.3, 0.3, 1.0f);
	light->setDiffuseColour(1, 0.98, 0.96, 1.0);

	

	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	worldMatrix = renderer->getWorldMatrix();
	//renderer.
	viewMatrix = camera->getViewMatrix();
	projectionMatrix = (renderLODchunks ? renderer->getOrthoMatrix() : renderer->getProjectionMatrix());

	// Send geometry data, set shader parameters, render object with shader

	renderer->setZBuffer(false);///	render sky
	XMMATRIX orthoMatrix = renderer->getOrthoMatrix();  // ortho matrix for 2D rendering
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();	// Default camera position for orthographic rendering
	XMMATRIX cameraPositionMatrix = XMMatrixTranslation(camera->getPosition().x, camera->getPosition().y, camera->getPosition().z);

	renderer->set2SidedMode(true);
	sky_sphere->sendData(renderer->getDeviceContext());// hopefully passing nullptr texture here is ok -->
	sunShader->setShaderParameters(renderer->getDeviceContext(), XMMatrixMultiply(XMMatrixMultiply(worldMatrix, XMMatrixScaling(8, 8, 8)),cameraPositionMatrix ), viewMatrix, projectionMatrix, nullptr, vars.TimeOfYear);
	sunShader->render(renderer->getDeviceContext(), sky_sphere->getIndexCount());
	renderer->set2SidedMode(false);

	///*// test sphere -- SUN
	sun_mesh->sendData(renderer->getDeviceContext());//						  	
	XMMATRIX testTranslation = XMMatrixTranslation(-1000 * sin(time / 3.81), 1000* sunAltitude, 1000 * -sin((time / 3.81) + 1.78) / (1.75 + sin(vars.TimeOfYear / 57.29577)));//XMMatrixTranslation(testPosition.x, testPosition.y, testPosition.z);	
	testTranslation = XMMatrixMultiply(testTranslation, cameraPositionMatrix);
	sunShader->setShaderParameters(renderer->getDeviceContext(), XMMatrixMultiply(XMMatrixMultiply(worldMatrix, XMMatrixScaling(10, 10, 10)), testTranslation), viewMatrix, projectionMatrix, nullptr);// light, camera, terrains[0]->GetChunkPosition(), scale, timeOfYear);
	sunShader->render(renderer->getDeviceContext(), sun_mesh->getIndexCount());
	//*/
	renderer->setZBuffer(true);

	

	/// main terrain
	m_Terrain->sendData(renderer->getDeviceContext(), D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);//													
	terrainShader->setShaderParameters(renderer->getDeviceContext(), XMMatrixMultiply(worldMatrix, m_TerrainMatrix), viewMatrix, projectionMatrix, textures, light, camera, &vars, curHeightmapSRV);// XMFLOAT4(tessellationFactor, height * 100, LODnear, LODfar), scale, XMFLOAT2(xOffset, yOffset), timeOfYear);
	terrainShader->render(renderer->getDeviceContext(), m_Terrain->getIndexCount());

	
	/// neighbouring terrains
	//float xMeshOffset = xz_TerrainMeshOffset * 3;//XMMATRIX xPositionMatrix;
	//float zMeshOffset = xz_TerrainMeshOffset * 3;//XMMATRIX zPositionMatrix;
	
	const int chunk = 1920;

	if (camera->getPosition().x < -chunk) /// if player moves to chunk border, reset position (relative to chunk) & update global offset (not seed) - illusion of seamless movement
	{
		vars.GlobalPosition.x -= chunk;
		camera->setPosition(camera->getPosition().x + chunk, camera->getPosition().y, camera->getPosition().z);//	  reset player to center of mesh (originally planned to set to opposite edge of mesh)
		renderMinimap();// regenerate terrain
	}	
	else if (camera->getPosition().x > chunk)
	{
		vars.GlobalPosition.x += chunk;
		camera->setPosition(camera->getPosition().x - chunk, camera->getPosition().y, camera->getPosition().z);
		renderMinimap();
	}
	
	if (camera->getPosition().z < -chunk) // negative z (south)
	{
		vars.GlobalPosition.y -= chunk;
		camera->setPosition(camera->getPosition().x, camera->getPosition().y, camera->getPosition().z + chunk);
		renderMinimap();
	}	
	else if (camera->getPosition().z > chunk) // north
	{
		vars.GlobalPosition.y += chunk;
		camera->setPosition(camera->getPosition().x, camera->getPosition().y, camera->getPosition().z - chunk);
		renderMinimap();
	}
	
	const XMMATRIX xPositionMatrix = XMMatrixTranslation(xz_TerrainMeshOffset * 3, 0.0, xz_TerrainMeshOffset);// 
	const XMMATRIX zPositionMatrix = XMMatrixTranslation(xz_TerrainMeshOffset, 0.0, xz_TerrainMeshOffset * 3);// 
	const XMMATRIX xzPositionMatrix = XMMatrixTranslation(xz_TerrainMeshOffset * 3, 0.0, xz_TerrainMeshOffset * 3);

	const XMMATRIX x2PositionMatrix = XMMatrixTranslation(-xz_TerrainMeshOffset, 0.0, xz_TerrainMeshOffset);// 
	const XMMATRIX z2PositionMatrix = XMMatrixTranslation(xz_TerrainMeshOffset, 0.0, -xz_TerrainMeshOffset);// 
	const XMMATRIX xz2PositionMatrix = XMMatrixTranslation(-xz_TerrainMeshOffset, 0.0, -xz_TerrainMeshOffset);
	const XMMATRIX xz3PositionMatrix = XMMatrixTranslation(xz_TerrainMeshOffset * 3, 0.0, -xz_TerrainMeshOffset);
	const XMMATRIX xz4PositionMatrix = XMMatrixTranslation(-xz_TerrainMeshOffset, 0.0, xz_TerrainMeshOffset * 3);

	XMMATRIX neighbourWorldMatrix;

	// X TERRAIN
	neighbourWorldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixMultiply(terrainScaleMatrix, xPositionMatrix));
	x_Terrain->sendData(renderer->getDeviceContext(), D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	terrainShader->setShaderParameters(renderer->getDeviceContext(), neighbourWorldMatrix, viewMatrix, projectionMatrix, textures, light, camera, &vars, curHeightmapSRV);
	terrainShader->render(renderer->getDeviceContext(), x_Terrain->getIndexCount());
	// Z TERRAIN
	neighbourWorldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixMultiply(terrainScaleMatrix, zPositionMatrix));
	z_Terrain->sendData(renderer->getDeviceContext(), D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	terrainShader->setShaderParameters(renderer->getDeviceContext(), neighbourWorldMatrix, viewMatrix, projectionMatrix, textures, light, camera, &vars, curHeightmapSRV);
	terrainShader->render(renderer->getDeviceContext(), z_Terrain->getIndexCount());
	// XZ TERRAIN
	neighbourWorldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixMultiply(terrainScaleMatrix, xzPositionMatrix));
	xz_Terrain->sendData(renderer->getDeviceContext(), D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	terrainShader->setShaderParameters(renderer->getDeviceContext(), neighbourWorldMatrix, viewMatrix, projectionMatrix, textures, light, camera, &vars, curHeightmapSRV);//XMFLOAT4(tessellationFactor, height * 100, LODnear, LODfar), scale, XMFLOAT2(xOffset, yOffset), timeOfYear);
	terrainShader->render(renderer->getDeviceContext(), xz_Terrain->getIndexCount());

	// X2 TERRAIN
	neighbourWorldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixMultiply(terrainScaleMatrix, x2PositionMatrix));
	x_Terrain->sendData(renderer->getDeviceContext(), D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	terrainShader->setShaderParameters(renderer->getDeviceContext(), neighbourWorldMatrix, viewMatrix, projectionMatrix, textures, light, camera, &vars, curHeightmapSRV);
	terrainShader->render(renderer->getDeviceContext(), x_Terrain->getIndexCount());
	// Z2 TERRAIN
	neighbourWorldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixMultiply(terrainScaleMatrix, z2PositionMatrix));
	z_Terrain->sendData(renderer->getDeviceContext(), D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	terrainShader->setShaderParameters(renderer->getDeviceContext(), neighbourWorldMatrix, viewMatrix, projectionMatrix, textures, light, camera, &vars, curHeightmapSRV);
	terrainShader->render(renderer->getDeviceContext(), z_Terrain->getIndexCount());
	// XZ2 TERRAIN
	neighbourWorldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixMultiply(terrainScaleMatrix, xz2PositionMatrix));
	xz_Terrain->sendData(renderer->getDeviceContext(), D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	terrainShader->setShaderParameters(renderer->getDeviceContext(), neighbourWorldMatrix, viewMatrix, projectionMatrix, textures, light, camera, &vars, curHeightmapSRV);
	terrainShader->render(renderer->getDeviceContext(), xz_Terrain->getIndexCount());
	// XZ3 TERRAIN
	neighbourWorldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixMultiply(terrainScaleMatrix, xz3PositionMatrix));
	xz_Terrain->sendData(renderer->getDeviceContext(), D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	terrainShader->setShaderParameters(renderer->getDeviceContext(), neighbourWorldMatrix, viewMatrix, projectionMatrix, textures, light, camera, &vars, curHeightmapSRV);
	terrainShader->render(renderer->getDeviceContext(), xz_Terrain->getIndexCount());
	// XZ4 TERRAIN
	neighbourWorldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixMultiply(terrainScaleMatrix, xz4PositionMatrix));
	xz_Terrain->sendData(renderer->getDeviceContext(), D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	terrainShader->setShaderParameters(renderer->getDeviceContext(), neighbourWorldMatrix, viewMatrix, projectionMatrix, textures, light, camera, &vars, curHeightmapSRV);
	terrainShader->render(renderer->getDeviceContext(), xz_Terrain->getIndexCount());

	// render 6 far_terrains. messy temporary code.
	// -x 0y; -x y; 0x y; x y; x 0y.
	//const XMMATRIX farX_PositionMatrix = XMMatrixTranslation(xz_TerrainMeshOffset * 3, 0.0, xz_TerrainMeshOffset);// 
/*	neighbourWorldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixMultiply(XMMatrixMultiply(terrainScaleMatrix, xPositionMatrix), XMMatrixScaling(3.0, 1.0, 3.0)));
	far_Terrain->sendData(renderer->getDeviceContext(), D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	terrainShader->setShaderParameters(renderer->getDeviceContext(), neighbourWorldMatrix, viewMatrix, projectionMatrix, textures, light, camera, &vars, csLand->getSRV());
	terrainShader->render(renderer->getDeviceContext(), xz_Terrain->getIndexCount());

	neighbourWorldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixMultiply(XMMatrixMultiply(terrainScaleMatrix, xzPositionMatrix),XMMatrixScaling(3.0, 1.0, 3.0) ));
	far_Terrain->sendData(renderer->getDeviceContext(), D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	terrainShader->setShaderParameters(renderer->getDeviceContext(), neighbourWorldMatrix, viewMatrix, projectionMatrix, textures, light, camera, &vars, csLand->getSRV());
	terrainShader->render(renderer->getDeviceContext(), xz_Terrain->getIndexCount());

	neighbourWorldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixMultiply(XMMatrixMultiply(terrainScaleMatrix, zPositionMatrix), XMMatrixScaling(3.0, 1.0, 3.0)));
	far_Terrain->sendData(renderer->getDeviceContext(), D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	terrainShader->setShaderParameters(renderer->getDeviceContext(), neighbourWorldMatrix, viewMatrix, projectionMatrix, textures, light, camera, &vars, csLand->getSRV());
	terrainShader->render(renderer->getDeviceContext(), xz_Terrain->getIndexCount());

	neighbourWorldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixMultiply(XMMatrixMultiply(terrainScaleMatrix, xz2PositionMatrix),XMMatrixScaling(3.0, 1.0, 3.0) ));
	far_Terrain->sendData(renderer->getDeviceContext(), D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	terrainShader->setShaderParameters(renderer->getDeviceContext(), neighbourWorldMatrix, viewMatrix, projectionMatrix, textures, light, camera, &vars, csLand->getSRV());
	terrainShader->render(renderer->getDeviceContext(), xz_Terrain->getIndexCount());

	neighbourWorldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixMultiply(XMMatrixMultiply(terrainScaleMatrix, x2PositionMatrix),XMMatrixScaling(3.0, 1.0, 3.0) ));
	far_Terrain->sendData(renderer->getDeviceContext(), D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	terrainShader->setShaderParameters(renderer->getDeviceContext(), neighbourWorldMatrix, viewMatrix, projectionMatrix, textures, light, camera, &vars, csLand->getSRV());
	terrainShader->render(renderer->getDeviceContext(), xz_Terrain->getIndexCount());

	////neighbourWorldMatrix = XMMatrixMultiply(worldMatrix, XMMatrixMultiply(XMMatrixScaling(3.0, 1.0, 3.0), XMMatrixMultiply(terrainScaleMatrix, xz4PositionMatrix)));
	////far_Terrain->sendData(renderer->getDeviceContext(), D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	////terrainShader->setShaderParameters(renderer->getDeviceContext(), neighbourWorldMatrix, viewMatrix, projectionMatrix, textures, light, camera, &vars, csLand->getSRV());
	////terrainShader->render(renderer->getDeviceContext(), xz_Terrain->getIndexCount());
*/
	/*			Back Face Culling
	
	D3D11_RASTERIZER_DESC wfdesc;
	//ZeroMemory(&wfdesc, sizeof(D3D11_RASTERIZER_DESC));
	//wfdesc.FillMode = D3D11_FILL_WIREFRAME;
	wfdesc.CullMode = D3D11_CULL_NONE;
	ID3D11RasterizerState* noCullMode;
	auto r = renderer->getDevice()->CreateRasterizerState(&wfdesc, &noCullMode);
	renderer->getDeviceContext()->RSSetState(noCullMode);
	//*/
	renderer->set2SidedMode(true);
	/// foliage (geo shader)
	if (floraOn) {
		//renderer->setAlphaBlending(true);

		/// grass
		f_Terrain->sendData(renderer->getDeviceContext());//worldMatrix//m_TerrainMatrix)
		treeShader->setShaderParameters(renderer->getDeviceContext(), XMMatrixMultiply(worldMatrix, XMMatrixMultiply(XMMatrixScaling(5, 1.0, 5), XMMatrixTranslation(int(camera->getPosition().x) - 700.0, 0, int(camera->getPosition().z) - 700.0))), viewMatrix, projectionMatrix, macroTexture, trees, light, camera, &vars, csLand->getSRV());
		treeShader->render(renderer->getDeviceContext(), f_Terrain->getIndexCount());

		/// trees
		f_Terrain->sendData(renderer->getDeviceContext());//worldMatrix//m_TerrainMatrix)
		treeShader->setShaderParameters(renderer->getDeviceContext(), XMMatrixMultiply(worldMatrix, XMMatrixMultiply(XMMatrixScaling(15, 1.0, 15), XMMatrixTranslation(int(camera->getPosition().x) - 2100.0, 0, int(camera->getPosition().z) - 2100.0))), viewMatrix, projectionMatrix, macroTexture, trees, light, camera, &vars, csLand->getSRV(), true);
		treeShader->render(renderer->getDeviceContext(), f_Terrain->getIndexCount());
		//renderer->setAlphaBlending(false);
	}

	/// render water
	renderer->setAlphaBlending(true);

	m_Water->sendData(renderer->getDeviceContext(), D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);//								vars.vars.
	waterShader->setShaderParameters(renderer->getDeviceContext(), XMMatrixMultiply(worldMatrix, waterMatrix), viewMatrix, projectionMatrix, textures[4], light, camera, XMFLOAT4(tessellationFactor, waterAmplitude, LODnear, LODfar), time);
	waterShader->render(renderer->getDeviceContext(), m_Water->getIndexCount());
	//renderer->setAlphaBlending(false);

	///	clouds		 - cant get em to look right ... (switch to back-face? - fixed) fix matrix/shader pipeline
	//
	//renderer->setAlphaBlending(true);
	///renderer->getDeviceContext()->RSSetState(renderer->getDeviceContext()->RSGetState())
	m_clouds->sendData(renderer->getDeviceContext());
	//
	textureShader->setShaderParameters(renderer->getDeviceContext(), XMMatrixMultiply(XMMatrixMultiply(worldMatrix, XMMatrixRotationX(0)), XMMatrixMultiply(XMMatrixMultiply(terrainScaleMatrix, XMMatrixScaling(2.0, 1.0, 02.0)), XMMatrixTranslation(-6400, 300.0, -6400.0))), viewMatrix, projectionMatrix, cloudTexture, vars.TimeOfYear);
	//textureShader->render(renderer->getDeviceContext(), m_clouds->getIndexCount());
	renderer->set2SidedMode(false);// return to regular fill rasterState
	//																																																																							cloudTexture
	//cloudShader->setShaderParameters(renderer->getDeviceContext(), XMMatrixMultiply(XMMatrixMultiply(worldMatrix, XMMatrixRotationX(0)), XMMatrixMultiply(XMMatrixMultiply(terrainScaleMatrix, XMMatrixScaling(0.32, 1.0, 0.32)), XMMatrixTranslation(-300+ camera->getPosition().x, 300.0, -300.0 + -300+ camera->getPosition().z))), viewMatrix, projectionMatrix, rainTexture, vars.TimeOfYear);
	//cloudShader->render(renderer->getDeviceContext(), m_clouds->getIndexCount());

	renderer->setAlphaBlending(false);
	


	// Set back buffer as render target and reset view port.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();

}
void App1::renderDOF()
{
	// horiontal pass using unblurred copy of the scene
	horBlur->setShaderParameters(renderer->getDeviceContext(), preDOFRT->getShaderResourceView());
	horBlur->compute(renderer->getDeviceContext(), ceil(sWidth / 256.f), sHeight, 1);
	horBlur->unbind(renderer->getDeviceContext());/// 

	verBlur->setShaderParameters(renderer->getDeviceContext(), horBlur->getSRV());//preDOFRT->getShaderResourceView()
	verBlur->compute(renderer->getDeviceContext(), sWidth, ceil(sHeight / 256.f), 1);
	verBlur->unbind(renderer->getDeviceContext());/// works!

}
void App1::finalPass()
{
	// Clear the scene. (default blue colour)
	renderer->beginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// Generate the view matrix based on the camera's position.
	///camera->update();

	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX orthoMatrix = renderer->getOrthoMatrix();// ortho matrix for 2D rendering
	XMMATRIX baseViewMatrix = camera->getOrthoViewMatrix();

	renderer->setWireframeMode(false);
	mapMesh->sendData(renderer->getDeviceContext());// renderr the rendertexture
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, (DepthOfField) ? cameraDepth->getDepthMapSRV() : preDOFRT->getShaderResourceView()); //verBlur->getSRV());//csLand->getSRV());//
	textureShader->render(renderer->getDeviceContext(), mapMesh->getIndexCount());
	//renderer->setZBuffer(true);

}





void App1::gui()
{
	
	
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	// Build UI
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);

	ImGui::Checkbox("Toggle Flora", &floraOn);
	ImGui::Checkbox("(orthographic view)", &renderLODchunks);
	//ImGui::SliderFloat("thingy", &test, 0.51, 1.999);
	
	//if (ImGui::CollapsingHeader("Terrain Generation Settings##")) {/// TERRAIN GEN
	//	//ImGui::SliderInt("##Terrain Resolution", &terrainResolution, 2, 1024, "Terrain Resolution: %d");
	//	//ImGui::SameLine();
	//	//if (ImGui::Button("16")) 
	//	//	terrainResolution = 16;
	//
	//	//ImGui::Text("Pre-generation vertical offset");
	//	//ImGui::SliderFloat("##height", &vars.Amplitude, -20, 20, "Height: %.4f", 2.5f);
	//
	//	//ImGui::SliderInt("##Octaves", &octaves, 1, 16, "%d Octaves");
	//}
	ImGui::Text("\nHigher scale magnifies the terrain");
	ImGui::SliderFloat("##Scale", &vars.Scale, 0.01, 50, "Scale: %.4f", 2.5f);

		ImGui::Text("Seed:");
		ImGui::SliderFloat("##x", &vars.seed.x, 0, 980000, "X Offset: %.2f");
		ImGui::SliderFloat("##y", &vars.seed.y, 0, 980000, "Y Offset: %.2f");		
		
		
		ImGui::Checkbox("Toggle Erosion", &erosion_enabled);
	
		if (ImGui::Button("Regenerate Terrain")) {
			renderMinimap(false);
		}ImGui::SameLine(); if (ImGui::Button("Erode Terrain")) {
			erodeTerrain();
		}ImGui::SameLine(); if (ImGui::Button("Both")) {
			renderMinimap(true);
		}
	if (ImGui::Button("Export Terrain")) {
		textureMgr->exportToFile(L"Exported Data/heightmap1.png", curHeightmapSRV);
			//(,curHeightmapSRV, ,L"Exported Data/heightmap1.png")//

	}
	//	if (terrainResolution != terrains[0]->GetResolution()) {
	//		terrains[0]->Resize(terrainResolution);
	//	}
	//	for (int i = 0; i < chunks; i++) {
	//		terrains[i]->SetScale(scale);
	//		terrains[i]->SetOffset(xOffset, yOffset);
	//		//if (i==0)
	//		terrains[i]->BrownianHtMap(octaves, height);
	//		//else// generate lower LOD terrain with fewer octaves (for performance), as the differince is less visible
	//		//	terrains[i]->BrownianHtMap((int)ceil(octaves/1.5), height);
	//		if (autoSmooth)
	//			terrains[i]->perlinSmooth(mountainPoint, minReducedHeight / 100.0, maxIncreasedHeight / 100.0);
	//		terrains[i]->Regenerate(renderer->getDevice(), renderer->getDeviceContext(), false);
	//	}
	//}
	ImGui::SliderFloat("##latitude", &vars.GlobalPosition.y, -2.0*vars.PlanetDiameter, 2.0*vars.PlanetDiameter, "Latitude: %.2f", 3.0f);
	ImGui::SliderFloat("##Planetdiameter", &vars.PlanetDiameter, 50, 127420, "Planet Diameter: %.2f", 5.0f);

	

	ImGui::Begin("Tessellation options");
		ImGui::SliderFloat("near threshold", &vars.LODnear, 0.0, vars.LODfar);
		ImGui::SliderFloat("far threshold", &vars.LODfar, vars.LODnear, 200);
		ImGui::SliderInt("Maximum Tessellation Factor", &vars.TessellationFactor, 0, 6, "2^%i");
		//ImGui::SliderFloat("Amplitude", &waterAmplitude, 0, 20, "%.1f", 2.0);
	ImGui::End();

	//ImGui::Begin("Minimap");
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::Begin("Minimap", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);
		ImGui::Text("(%.2f, %.2f)", vars.GlobalPosition.x, vars.GlobalPosition.y);
		ImGui::SameLine();
		if (ImGui::ArrowButton("zoom+", ImGuiDir_Up) && mapZoom < 80) {
			mapZoom++;
		}ImGui::SameLine();
		if (ImGui::ArrowButton("zoom-", ImGuiDir_Down) && mapZoom > 1) {
			mapZoom--;
		}
		ImGui::Image(curHeightmapSRV/*verBlur->getSRV()*/, ImVec2(200, 200));///
		//ImGui::Image(my_tex_id, ImVec2(my_tex_w, my_tex_h), ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
		//ImGui::ShowDemoWindow();
		//if (ImGui::IsItemHovered())
		{
			//ImGui::BeginTooltip();
			//float region_sz = 32.0f;
			//float region_x = io.MousePos.x - pos.x - region_sz * 0.5f; if (region_x < 0.0f) region_x = 0.0f; else if (region_x > my_tex_w - region_sz) region_x = my_tex_w - region_sz;
			//float region_y = io.MousePos.y - pos.y - region_sz * 0.5f; if (region_y < 0.0f) region_y = 0.0f; else if (region_y > my_tex_h - region_sz) region_y = my_tex_h - region_sz;
			//float zoom = 4.0f;
			//ImGui::Text("Min: (%.2f, %.2f)", region_x, region_y);
			//ImGui::Text("Max: (%.2f, %.2f)", region_x + region_sz, region_y + region_sz);
			//ImVec2 uv0 = ImVec2((region_x) / my_tex_w, (region_y) / my_tex_h);
			//ImVec2 uv1 = ImVec2((region_x + region_sz) / my_tex_w, (region_y + region_sz) / my_tex_h);
			//ImGui::Image(my_tex_id, ImVec2(region_sz * zoom, region_sz * zoom), uv0, uv1, ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
			//ImGui::EndTooltip();
		}
		//ImVec2 p = ImGui::GetCursorScreenPos();
		//ImGui::SetCursorScreenPos(ImVec2(p.x + 200, p.y + 250));// sets the window to 200x200
	ImGui::End();
	ImGui::PopStyleVar();
	//ImGui::End();

	ImGui::Begin("Time and Season options");/// TIME
	
		ImGui::Checkbox("Toggle Dynamic Time", &unpaused);
		ImGui::SameLine();
		ImGui::SliderFloat("##time", &time, 0.0, 24.00, "Time: %5.2f");
		int day = int(vars.TimeOfYear);
		ImGui::SliderInt("##date", &day, 0.0, 360.00, "Time of Year: %5.2f");
		vars.TimeOfYear = day + (time/24.00);//
		//ImGui::SameLine();
		ImGui::SliderFloat("##timescale", &timescale, 0.02777777, 1200, "x %5.2f", 5.0f);
	ImGui::End();

	ImGui::Begin("Options", &gameSettingsMenuActive);
		ImGui::SliderFloat("##cams", camera->getSpeedScale(), 0.5, 400.00, "Camera Speed: %.2f", 3);

	ImGui::End();

	//*
	ImGui::Begin("Test  options");
		auto v = camera->getViewMatrix();
		float pos[4] = { v.r[0].m128_f32[0],v.r[0].m128_f32[1],v.r[0].m128_f32[2],v.r[0].m128_f32[3] };
		float pas[4] = { v.r[1].m128_f32[0],v.r[1].m128_f32[1],v.r[1].m128_f32[2],v.r[1].m128_f32[3] };
		float pus[4] = { v.r[2].m128_f32[0],v.r[2].m128_f32[1],v.r[2].m128_f32[2],v.r[2].m128_f32[3] };
		v = XMMatrixInverse(&XMMatrixDeterminant(v), v);
		float pes[4] = { v.r[3].m128_f32[0],v.r[3].m128_f32[1],v.r[3].m128_f32[2],v.r[3].m128_f32[3] };
		ImGui::SliderFloat4("a", pos, 0, 1);
		ImGui::SliderFloat4("b", pas, 0, 1);
		ImGui::SliderFloat4("c", pus, 0, 1);
		ImGui::SliderFloat4("d", pes, 0, 1);
		//testPosition = { pos[0], pos[1], pos[2] };
	ImGui::End();//*/

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void App1::initTextures() {
//			

	const wchar_t* material[8] = { L"grass",L"stone",L"snow",L"rock",L"water",L"sand",L"mulch",L"gravel" };
	const wchar_t* type[4] = { L"_c",L"_h",L"_n",L"_s" };

	const wchar_t* path = L"res/nice textures/";
	const wchar_t* sep = L"_/";
	const wchar_t* extn = L".png";

	//for (int t = 0; t < 4; t++) {		unfortunately i can't get this to work
	//	for (int m = 0; m < 8; m++) {
	//		// use wstring for simple concatenation -- stackoverflow.com/questions/1855956/
	//		std::wstring name = std::wstring(material[m]) + std::wstring(type[t]);// ie "grass_c"
	//		std::wstring fpath = std::wstring(path) + std::wstring(material[m]) + std::wstring(sep) + std::wstring(name) + std::wstring(extn);
	//		// ie "res/nice textures/grass_/grass_c.png"
	//		textureMgr->loadTexture(name.c_str(), fpath.c_str());
	//		
	//		// put the texture in the array ready for the shader
	//		//textures[m + (t * 8)] = textureMgr->getTexture(name.c_str());		
	//	}
	//}
	
	//auto xzzj = textures;
//*
	textureMgr->loadTexture(L"rock_c", L"res/lofi textures/rock_/rock_c.png");
	//textureMgr->loadTexture(L"rock_h", L"res/nice textures/rock_/rock_h.png");
	textureMgr->loadTexture(L"rock_n", L"res/nice textures/rock_/rock_nh.png");
	//textureMgr->loadTexture(L"rock_s", L"res/nice textures/rock_/rock_s.png");
	//
	textureMgr->loadTexture(L"snow_c", L"res/nice textures/snow_/snow_as.png");
	//textureMgr->loadTexture(L"snow_h", L"res/nice textures/snow_/snow_h.png");
	textureMgr->loadTexture(L"snow_n", L"res/nice textures/snow_/snow_nh.png");
	//textureMgr->loadTexture(L"snow_s", L"res/nice textures/snow_/snow_s.png");
	//
	textureMgr->loadTexture(L"grass_c", L"res/nice textures/grass_/grass_as.png");
	//textureMgr->loadTexture(L"grass_h", L"res/nice textures/grass_/grass_h.png");
	textureMgr->loadTexture(L"grass_n", L"res/nice textures/grass_/grass_nh.png");
	//textureMgr->loadTexture(L"grass_s", L"res/nice textures/grass_/grass_s.png");
	//
	textureMgr->loadTexture(L"stone_c", L"res/nice textures/stone_/stone_as.png");
	//textureMgr->loadTexture(L"stone_h", L"res/nice textures/stone_/stone_h.png");
	textureMgr->loadTexture(L"stone_n", L"res/nice textures/stone_/stone_nh.png");
	//textureMgr->loadTexture(L"stone_s", L"res/nice textures/stone_/stone_s.png");
	//
	textureMgr->loadTexture(L"mulch_c", L"res/nice textures/mulch_/mulch_as.png");
	//textureMgr->loadTexture(L"mulch_h", L"res/nice textures/mulch_/mulch_h.png");
	textureMgr->loadTexture(L"mulch_n", L"res/nice textures/mulch_/mulch_nh.png");
	//textureMgr->loadTexture(L"mulch_s", L"res/nice textures/mulch_/mulch_s.png");
	//
	textureMgr->loadTexture(L"sand_c", L"res/nice textures/sand_/sand_as.png");
	//textureMgr->loadTexture(L"sand_h", L"res/nice textures/sand_/sand_h.png");
	textureMgr->loadTexture(L"sand_n", L"res/nice textures/sand_/sand_nh.png");
	//textureMgr->loadTexture(L"sand_s", L"res/nice textures/sand_/sand_s.png");
	//
	textureMgr->loadTexture(L"gravel_c", L"res/nice textures/gravel_/gravel_as.png");
	//textureMgr->loadTexture(L"gravel_h", L"res/nice textures/gravel_/gravel_h.png");
	textureMgr->loadTexture(L"gravel_n", L"res/nice textures/gravel_/gravel_nh.png");
	//textureMgr->loadTexture(L"gravel_s", L"res/nice textures/gravel_/gravel_s.png");
	//
	textureMgr->loadTexture(L"water_c", L"res/nice textures/water_/water_c.png");
	//textureMgr->loadTexture(L"water_h", L"res/nice textures/water_/water_h.png");
	textureMgr->loadTexture(L"water_n", L"res/nice textures/water_/water_n.png");
	//textureMgr->loadTexture(L"water_s", L"res/nice textures/water_/water_s.png");//
	//
	textureMgr->loadTexture(L"savan_c", L"res/nice textures/savan_/savan_as.png");
	//textureMgr->loadTexture(L"savan_h", L"res/nice textures/savan_/savan_h.png");
	textureMgr->loadTexture(L"savan_n", L"res/nice textures/savan_/savan_nh.png");
	//textureMgr->loadTexture(L"savan_s", L"res/nice textures/savan_/savan_s.png");
//*/
/*
	for (int t = 0; t < 4; t++) { //		 does not work either
		for (int m = 0; m < 8; m++) {
			std::wstring name = std::wstring(material[m]) + std::wstring(type[t]);
			// put the texture in the array ready for the shader
			auto gggg0 = name.c_str();
			bool wat = gggg0 == name;
			textures[m + (t * 8)] = textureMgr->getTexture(gggg0);
			int x = m;
		}
	}
//*/
	// albedo/specular maps (c for colour)
	textures[0] = textureMgr->getTexture(L"grass_c");
	textures[1] = textureMgr->getTexture(L"stone_c");
	textures[2] = textureMgr->getTexture(L"snow_c");
	textures[3] = textureMgr->getTexture(L"rock_c");
	textures[4] = textureMgr->getTexture(L"water_c");
	textures[5] = textureMgr->getTexture(L"sand_c");
	textures[6] = textureMgr->getTexture(L"mulch_c");
	textures[7] = textureMgr->getTexture(L"gravel_c");
	textures[8] = textureMgr->getTexture(L"savan_c");
	
	// normal/height maps
	textures[9] = textureMgr->getTexture(L"grass_n");
	textures[10] = textureMgr->getTexture(L"stone_n");
	textures[11] = textureMgr->getTexture(L"snow_n");
	textures[12] = textureMgr->getTexture(L"rock_n");
	textures[13] = textureMgr->getTexture(L"water_n");
	textures[14] = textureMgr->getTexture(L"sand_n");
	textures[15] = textureMgr->getTexture(L"mulch_n");
	textures[16] = textureMgr->getTexture(L"gravel_n");
	textures[17] = textureMgr->getTexture(L"savan_n");
	/*//	normal maps
	textures[16]  = textureMgr->getTexture(L"grass_h");
	textures[17]  = textureMgr->getTexture(L"stone_h");
	textures[18] = textureMgr->getTexture(L"snow_h");
	textures[19] = textureMgr->getTexture(L"rock_h");
	textures[20]  = textureMgr->getTexture(L"water_h");
	textures[21] = textureMgr->getTexture(L"sand_h");
	textures[22]  = textureMgr->getTexture(L"mulch_h");
	textures[23] = textureMgr->getTexture(L"gravel_h");
	
	//	specular maps
	textures[24] = textureMgr->getTexture(L"grass_s");
	textures[25] = textureMgr->getTexture(L"stone_s");
	textures[26] = textureMgr->getTexture(L"snow_s");
	textures[27] = textureMgr->getTexture(L"rock_s");
	textures[28] = textureMgr->getTexture(L"water_s");
	textures[29] = textureMgr->getTexture(L"sand_s");
	textures[30] = textureMgr->getTexture(L"mulch_s");
	textures[31] = textureMgr->getTexture(L"gravel_s");
	*/
	
	//textures[33] = textureMgr->getTexture(L"savan_h");
	
	//textures[35] = textureMgr->getTexture(L"savan_s");

	// flora	(use "oak2.png" etc for low res textures)								
	textureMgr->loadTexture(L"grass",			  L"res/flora/grass.png");
	textureMgr->loadTexture(L"pine",	     L"res/flora/pine.png");
	textureMgr->loadTexture(L"beech_n", L"res/flora/pbr/beech_n.png");
	textureMgr->loadTexture(L"beech_c",         L"res/flora/pbr/beech_c.png");

	trees[0] = textureMgr->getTexture(L"grass");
	trees[1] = textureMgr->getTexture(L"pine");
	trees[2] = textureMgr->getTexture(L"beech_n");
	trees[3] = textureMgr->getTexture(L"beech_c");

	textureMgr->loadTexture(L"cloud", L"res/nice textures/cloud.png");
	cloudTexture = textureMgr->getTexture(L"cloud");
	textureMgr->loadTexture(L"rain", L"res/nice textures/rain.png");
	rainTexture = textureMgr->getTexture(L"rain");
	textureMgr->loadTexture(L"macro", L"res/macro_variation.png");
	macroTexture = textureMgr->getTexture(L"macro");
	//textureMgr->loadTexture(L"foo", L"res/test.png");
	//cloudTexture = textureMgr->getTexture(L"foo");
}