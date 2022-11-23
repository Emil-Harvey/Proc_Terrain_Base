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
	qt_Terrain = nullptr;
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
		
	m_Water = new TessellationPlane(renderer->getDevice(), renderer->getDeviceContext(), Water_Mesh_Res);
	sun_mesh = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext(), 10);
	sky_sphere = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext(), 3);

	// Load and initialise required shader handle objects.
	defaultShader = new LightShader(renderer->getDevice(), hwnd);// may be defunct
	sunShader = new TextureShader(renderer->getDevice(), hwnd, false);
	treeShader = new Grower(renderer->getDevice(), hwnd);
	waterShader = new TessShader(renderer->getDevice(), hwnd);
	terrainShader = new TerrainShader(renderer->getDevice(), hwnd);
	horBlur = new ComputeBlurHor(renderer->getDevice(), hwnd, screenWidth, screenHeight);
	verBlur = new ComputeBlurVert(renderer->getDevice(), hwnd, screenWidth, screenHeight);
	csLand = new ComputeLandscape(renderer->getDevice(), hwnd, MAP_DIM, MAP_DIM);
	csErosion = new ComputeErosion(renderer->getDevice(), hwnd, 1500, 1500);

	
	// minimap
	mapMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth, screenHeight, 0, 0);
	mapRenderTexture = new RenderTexture(renderer->getDevice(), 1500, 1500, SCREEN_NEAR, SCREEN_DEPTH);
	textureShader = new TextureShader(renderer->getDevice(), hwnd);

	cloudShader = new CloudShader(renderer->getDevice(), hwnd);

	// initialise RT objects. 
	preDOFRT = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, /*400,300,*/ 0.1f, 200.f);
	cameraDepth = new ShadowMap(renderer->getDevice(), screenWidth, screenHeight);

	/// Terrain stuff
	qt_Terrain = new QuadTreeMesh(renderer->getDevice(), renderer->getDeviceContext(), { 0.0,0.0 }, 45000.0, 4, {0.0,0.0});

	f_Terrain = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext(), 280);

	m_clouds = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext(), 16);

	vars.LODfar = 53;//
	vars.Scale = 7;
	vars.PlanetDiameter = 127420;// a 1/1000th of earth
	vars.TessellationFactor = 1;
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
	qt_Terrain->SetHeightmap(&pixelData);

	///init imgui styling
	{
		ImVec4* colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
		colors[ImGuiCol_Border] = ImVec4(0.63f, 0.63f, 0.70f, 0.50f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.44f, 0.44f, 0.44f, 0.60f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.57f, 0.57f, 0.57f, 0.70f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.76f, 0.76f, 0.76f, 0.80f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.60f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.13f, 0.75f, 0.55f, 0.80f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.13f, 0.75f, 0.75f, 0.80f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);
		colors[ImGuiCol_Button] = ImVec4(0.13f, 0.75f, 0.55f, 0.40f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.13f, 0.75f, 0.75f, 0.60f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);
		colors[ImGuiCol_Header] = ImVec4(0.13f, 0.75f, 0.55f, 0.40f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.13f, 0.75f, 0.75f, 0.60f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);
		colors[ImGuiCol_Separator] = ImVec4(0.13f, 0.75f, 0.55f, 0.40f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.13f, 0.75f, 0.75f, 0.60f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.13f, 0.75f, 0.55f, 0.40f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.13f, 0.75f, 0.75f, 0.60f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.13f, 0.75f, 1.00f, 0.80f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
		/// corner shaping
		ImGui::GetStyle().WindowRounding = 0.0f;
		ImGui::GetStyle().ChildRounding = 0.0f;
		ImGui::GetStyle().FrameRounding = 0.0f;
		ImGui::GetStyle().GrabRounding = 0.0f;
		ImGui::GetStyle().PopupRounding = 0.0f;
		ImGui::GetStyle().ScrollbarRounding = 0.0f;
	}
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
	if (qt_Terrain)
	{
		delete qt_Terrain;
		qt_Terrain = 0;
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
		time += 0.01f * timescale * timer->getTime();// pass time
		vars.TimeOfYear += (0.01f / 24.f) * timer->getTime();

		
	}

	// Generate the view matrix based on the camera's position.
	camera->update();

	/// check for chunk update
	if(chunkUpdatesEnabled){
	// if player moves to chunk border, reset position (relative to chunk) & update global offset (not seed) - illusion of seamless/infinite movement
	static const int chunk = 2050*2;
	const float xxxg = 3.1f;// idk why this is a random number it should be 1
	if (camera->getPosition().x < -chunk) 
	{
		vars.GlobalPosition.x -= chunk * xxxg;
		camera->setPosition(camera->getPosition().x + chunk, camera->getPosition().y, camera->getPosition().z);//	  reset player to center of mesh (originally planned to set to opposite edge of mesh)
		renderMinimap();// regenerate terrain
	}
	else if (camera->getPosition().x > chunk)
	{
		vars.GlobalPosition.x += chunk * xxxg;
		camera->setPosition(camera->getPosition().x - chunk, camera->getPosition().y, camera->getPosition().z);
		renderMinimap();
	}

	if (camera->getPosition().z < -chunk) // negative z (south)
	{
		vars.GlobalPosition.y -= chunk * xxxg;
		camera->setPosition(camera->getPosition().x, camera->getPosition().y, camera->getPosition().z + chunk);
		renderMinimap();
	}
	else if (camera->getPosition().z > chunk) // north
	{
		vars.GlobalPosition.y += chunk * xxxg;
		camera->setPosition(camera->getPosition().x, camera->getPosition().y, camera->getPosition().z - chunk);
		renderMinimap();
	}

	XMFLOAT2 camera_xz = { camera->getPosition().x, camera->getPosition().z };
	static XMFLOAT2 old_camera_xz;
	if(camera_xz.x != old_camera_xz.x && camera_xz.y != old_camera_xz.y)
		qt_Terrain->Reconstruct(renderer->getDevice(), renderer->getDeviceContext(), 4, camera_xz);// careful of memory leak
		old_camera_xz = camera_xz;
	}

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
	csLand->compute(renderer->getDeviceContext(), MAP_DIM / 8, MAP_DIM / 8, 1);
	csLand->unbind(renderer->getDeviceContext());
	// make a copy of the SRV to be used in the future, as the pointer may 'break'(?)
	curHeightmapSRV = csLand->getSRV();//&csLandscapeSRV;//

	if (erode_as_well)
		erodeTerrain();

#ifdef CPU_TERRAIN_ENABLED
	TransferHeightmapToCPU();
#endif // CPU_TERRAIN_ENABLED

	return true;
}
bool App1::erodeTerrain()
{
	if (erosion_enabled) {
		csErosion->setShaderParameters(renderer->getDeviceContext(), curHeightmapSRV, &vars);//TODO: find out why erosion does not work on 'eroded' heightmap
		csErosion->compute(renderer->getDeviceContext(), 80, 80, 1);
		csErosion->unbind(renderer->getDeviceContext());
		// make a copy of the SRV to be used in the future, as the pointer may 'break'(?) //
		curHeightmapSRV = csErosion->getSRV();//&csErosionSRV;//

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
//	projectionMatrix = (orthoCameraEnabled ? renderer->getOrthoMatrix() : renderer->getProjectionMatrix());// renderer->getOrthoMatrix();//getProjectionMatrix();//
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

	//water
	static const XMMATRIX waterScaleMatrix = XMMatrixScaling(45000 , 1.0f, 45000 );
	
	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	worldMatrix = renderer->getWorldMatrix();
	viewMatrix = camera->getViewMatrix();
	projectionMatrix = (orthoCameraEnabled ? renderer->getOrthoMatrix() : renderer->getProjectionMatrix());


	////////
	float lengthOfDay = -12 * sin(vars.GlobalPosition.y) * cos(vars.TimeOfYear / 57.29577f);// replace -6 [-1] w/ -12*sin(latitude)  <--- to be moved to PS?
///
	float sunAltitude = lengthOfDay - (cos(time / 3.81f) / 0.8f);// necessary for sun mesh position


// Clear the scene. 
	renderer->beginScene(0.3f, 0.3f, 0.3f, 1.0f);
	light->setDiffuseColour(1, 0.98f, 0.96f, 1.0f);



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
	XMFLOAT3 sunPos = { 
		-sin(time / 3.81f), 
		sunAltitude,
		-sin((time / 3.81f) + 1.78f) / (1.75f + sin(vars.TimeOfYear / 57.29577f)) 
	};
	XMStoreFloat3( &sunPos, XMVector3Normalize(XMLoadFloat3(&sunPos)));
	XMMATRIX testTranslation = XMMatrixTranslation(sunPos.x*1000, sunPos.y*1000, sunPos.z*1000);

	testTranslation = XMMatrixMultiply(testTranslation, cameraPositionMatrix);
	sunShader->setShaderParameters(renderer->getDeviceContext(), XMMatrixMultiply(XMMatrixMultiply(worldMatrix, XMMatrixScaling(10, 10, 10)), testTranslation), viewMatrix, projectionMatrix, nullptr);// light, camera, terrains[0]->GetChunkPosition(), scale, timeOfYear);
	sunShader->render(renderer->getDeviceContext(), sun_mesh->getIndexCount());
	//*/
	renderer->setZBuffer(true);

	
	
	Frustum* frustum = new Frustum();
	frustum->ConstructFrustum(projectionMatrix, viewMatrix);
	qt_Terrain->render(renderer->getDeviceContext(), terrainShader, worldMatrix /*XMMatrixMultiply(worldMatrix, m_TerrainMatrix)*/, viewMatrix, projectionMatrix, frustum, textures, light, camera, &vars, curHeightmapSRV);

	/// main terrain
//	m_Terrain->sendData(renderer->getDeviceContext(), D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);//													
//	terrainShader->setShaderParameters(renderer->getDeviceContext(), XMMatrixMultiply(worldMatrix, m_TerrainMatrix), viewMatrix, projectionMatrix, textures, light, camera, &vars, curHeightmapSRV);// XMFLOAT4(tessellationFactor, height * 100, LODnear, LODfar), scale, XMFLOAT2(xOffset, yOffset), timeOfYear);
//	terrainShader->render(renderer->getDeviceContext(), m_Terrain->getIndexCount());

	
	/// neighbouring terrains
	//float xMeshOffset = xz_TerrainMeshOffset * 3;//XMMATRIX xPositionMatrix;
	//float zMeshOffset = xz_TerrainMeshOffset * 3;//XMMATRIX zPositionMatrix;
	
	
	
	//		Disable Back Face Culling
	
	renderer->set2SidedMode(true);
	/// foliage (geo shader)
	if (floraOn) {
		//renderer->setAlphaBlending(true);

		/// grass
		f_Terrain->sendData(renderer->getDeviceContext());//worldMatrix//m_TerrainMatrix)
		treeShader->setShaderParameters(renderer->getDeviceContext(), XMMatrixMultiply(worldMatrix, XMMatrixMultiply(XMMatrixScaling(5, 1.0f, 5), XMMatrixTranslation(int(camera->getPosition().x) - 700.0f, 0, int(camera->getPosition().z) - 700.0))), viewMatrix, projectionMatrix, macroTexture, trees, light, camera, &vars, csLand->getSRV());
		treeShader->render(renderer->getDeviceContext(), f_Terrain->getIndexCount());

		/// trees
		f_Terrain->sendData(renderer->getDeviceContext());//worldMatrix//m_TerrainMatrix)
		treeShader->setShaderParameters(renderer->getDeviceContext(), XMMatrixMultiply(worldMatrix, XMMatrixMultiply(XMMatrixScaling(15, 1.0f, 15), XMMatrixTranslation(int(camera->getPosition().x) - 2100.0f, 0, int(camera->getPosition().z) - 2100.0))), viewMatrix, projectionMatrix, macroTexture, trees, light, camera, &vars, csLand->getSRV(), true);
		treeShader->render(renderer->getDeviceContext(), f_Terrain->getIndexCount());
		//renderer->setAlphaBlending(false);
	}

	/// render water
	renderer->setAlphaBlending(true);

//	m_Water->sendData(renderer->getDeviceContext(), D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);//								vars.vars.
//	waterShader->setShaderParameters(renderer->getDeviceContext(), XMMatrixMultiply(worldMatrix, waterScaleMatrix), viewMatrix, projectionMatrix, textures[4], light, camera, XMFLOAT4(tessellationFactor, waterAmplitude, LODnear, LODfar), time);
//	waterShader->render(renderer->getDeviceContext(), m_Water->getIndexCount());
	//renderer->setAlphaBlending(false);

	///	clouds (and rain..)
	//
	//renderer->setAlphaBlending(true);
	
	m_clouds->sendData(renderer->getDeviceContext()); //
	cloudShader->setShaderParameters(renderer->getDeviceContext(), XMMatrixMultiply(XMMatrixMultiply(worldMatrix, XMMatrixRotationX(0)), XMMatrixMultiply( XMMatrixScaling(2048.0, 1.0, 2048.0), XMMatrixTranslation(-14000, 1700.0, -14000.0))), viewMatrix, projectionMatrix, cloudTexture, vars.TimeOfYear);
	cloudShader->render(renderer->getDeviceContext(), m_clouds->getIndexCount());
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
	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, (DepthOfField) ? verBlur->getSRV() : preDOFRT->getShaderResourceView()); //verBlur->getSRV());//csLand->getSRV());//
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
	ImGui::Checkbox("(orthographic view)", &orthoCameraEnabled);
	//ImGui::SliderFloat("thingy", &test, 0.51, 1.999);
	
	
	//ImGui::Text("\nHigher scale 'magnifies' the terrain");
	ImGui::SliderFloat("##Scale", &vars.Scale, 0.01f, 50, "Scale: %.4f", 2.5f);

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

	ImGui::SliderFloat("##latitude", &vars.GlobalPosition.y, -2.0f*vars.PlanetDiameter, 2.0f*vars.PlanetDiameter, "Latitude: %.2f", 3.0f);
	ImGui::SliderFloat("##Planetdiameter", &vars.PlanetDiameter, 50, 127420, "Planet Diameter: %.2f", 5.0f);

	ImGui::Begin("Quadtree Heightmap ");
	QuadtreeNode* node_to_render = nullptr;
	if (ImGui::Button("LOD0-Root")) {
		node_to_render = qt_Terrain->getRoot();
		QuadtreeHeightmap(node_to_render);
	}if (ImGui::Button("LOD1-Root-NE")) {
		node_to_render = qt_Terrain->getRoot()->Nodes()->at(northeast).get();
		QuadtreeHeightmap(node_to_render);
	}if (ImGui::Button("LOD1-Root-SE")) {
		node_to_render = qt_Terrain->getRoot()->Nodes()->at(southeast).get();
		QuadtreeHeightmap(node_to_render);
	}if (ImGui::Button("LOD1-Root-SW")) {
		node_to_render = qt_Terrain->getRoot()->Nodes()->at(southwest).get();
		QuadtreeHeightmap(node_to_render);
	}if (ImGui::Button("LOD1-Root-NW")) {
		node_to_render = qt_Terrain->getRoot()->Nodes()->at(northwest).get();
		QuadtreeHeightmap(node_to_render);
	}
	if (ImGui::Button("LOD1-Root-NE-SW")) {
		node_to_render = qt_Terrain->getRoot()->Nodes()->at(northeast).get()->Nodes()->at(southwest).get();
		QuadtreeHeightmap(node_to_render);
	}
	if (ImGui::Button("LOD1-Root-NE-SE")) {
		node_to_render = qt_Terrain->getRoot()->Nodes()->at(northeast).get()->Nodes()->at(southeast).get();
		QuadtreeHeightmap(node_to_render);
	}
	ImGui::End();

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
		if (ImGui::ArrowButton("zoom+", ImGuiDir_Up) && mapZoom <= 80) {
			mapZoom++;
		}ImGui::SameLine();
		if (ImGui::ArrowButton("zoom-", ImGuiDir_Down) && mapZoom > 1) {
			mapZoom--;
		}
		ImGui::Image(curHeightmapSRV/*verBlur->getSRV()*/, ImVec2(200, 200), ImVec2(0,1), ImVec2(1,0));///
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
		ImGui::SliderInt("##date", &day, 0.0f, 360.00f, "Time of Year: %5.2f");
		vars.TimeOfYear = day + (time/24.00f);//
		//ImGui::SameLine();
		ImGui::SliderFloat("##timescale", &timescale, 0.02777777f, 1200, "x %5.2f", 5.0f);
	ImGui::End();

	ImGui::Begin("Options", &gameSettingsMenuActive);
		ImGui::SliderFloat("##cams", camera->getSpeedScale(), 0.5f, 600.00f, "Camera Speed: %.2f", 3);


		if (ImGui::Button("Toggle chunk & quad tree update")) {
			chunkUpdatesEnabled = !chunkUpdatesEnabled;
		}

	ImGui::End();

	/*
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

	const wchar_t* material[9] = { L"grass",L"stone",L"snow",L"rock",L"water",L"sand",L"mulch",L"gravel",L"savan" };
	const wchar_t* type[2] = { L"_as",L"_nh" };//{ L"_c",L"_h",L"_n",L"_s" };

	const wchar_t* path = L"res/nice textures/";
	const wchar_t* sep = L"_/";
	const wchar_t* extn = L".png";

	for (int t = 0; t < 2; t++) {//		unfortunately i can't get this to work
		for (int m = 0; m < 9; m++) {
			// use wstring for simple concatenation -- stackoverflow.com/questions/1855956/
			std::wstring name = std::wstring(material[m]) + std::wstring(type[t]);// ie "grass_c"
			std::wstring fpath = std::wstring(path) + std::wstring(material[m]) + std::wstring(sep) + std::wstring(name) + std::wstring(extn);
			// ie "res/nice textures/grass_/grass_c.png"
			textureMgr->loadTexture(name.c_str(), fpath.c_str());
			
			// put the texture in the array ready for the shader
			textures[m + (t * 8)] = textureMgr->getTexture(name.c_str());		
		}
	}
	
	//auto xzzj = textures;
/*
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
//* /
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
	/* //	normal maps
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

inline void App1::TransferHeightmapToCPU() 
{//*
	// Delete old pixelData
	//pixelData
/// Create a 'resource' that is able to be accessed & read on the CPU; a copy of the heightmap texture
	//Getting the resources of the texture view
	
	ID3D11Resource* textureResourceGPU = nullptr;
	ID3D11Texture2D* textureInterface = nullptr;

	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = MAP_DIM;
	textureDesc.Height = MAP_DIM;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	textureDesc.Usage = D3D11_USAGE_STAGING;
	//textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	textureDesc.MiscFlags = 0;
	ID3D11Texture2D* staging_texture = 0;
	renderer->getDevice()->CreateTexture2D(&textureDesc, 0, &staging_texture);

	curHeightmapSRV->GetResource(&textureResourceGPU);
	textureResourceGPU->QueryInterface(&textureInterface);

	renderer->getDeviceContext()->CopyResource(staging_texture, textureResourceGPU);

	HRESULT h;
	h = renderer->getDeviceContext()->Map(staging_texture, 0, D3D11_MAP_READ, 0, &heightmap_mappedResource); // this line causes memory leak

	// heightmap_mappedResource.RowPitch contains the value that the runtime adds to pData to move from row to row, where each row contains multiple pixels.

	/// access the raw data from the resource
	// we know the heightmap is 1600x1600 pixels = 2560000. 
	// bytes = heightmap_mappedResource.RowPitch * height; = 25600 * 1600 = 40,960,000
	const int heightmap_size = MAP_DIM * MAP_DIM;

	//XMFLOAT4 temp_arr[1600 * 1600] = {(XMFLOAT4*)heightmap_mappedResource.pData} //std::copy_n(std::begin(), heightmap_size, std::begin(pixelData))//pixelData = ((XMFLOAT4*)heightmap_mappedResource.pData);

	for (int row = 0; row < MAP_DIM; row++) {
		for (int pixel = 0; pixel < MAP_DIM; pixel++) {
			const int pixel_number = pixel + (row * MAP_DIM);
			pixelData[pixel_number] = (XMFLOAT4)((XMFLOAT4*)heightmap_mappedResource.pData)[pixel_number];
		}
	}

	//qt_Terrain->SetHeightmap(&pixelData); // not even needed
	//delete textureResourceGPU;
	//delete textureInterface;*/
	//return h;
}

// size: fraction<=1 of the quad proportional to the full mesh
// position: world position of the quad center relative to the full mesh center
void App1::QuadtreeHeightmap( QuadtreeNode* node)
{
	//
	const float size = node->Size() / node->total_size;
	const XMFLOAT2 position = node->Position();//{ node->Position().x / node->total_size, node->Position().y / node->total_size };

	ShaderVariables alt_vars = vars;
	alt_vars.GlobalPosition = { alt_vars.GlobalPosition.x + position.x, alt_vars.GlobalPosition.y + position.y };
	alt_vars.Amplitude = size;//alt_vars.Scale /= size;

	csLand->setShaderParameters(renderer->getDeviceContext(), curHeightmapSRV, &alt_vars);
	csLand->compute(renderer->getDeviceContext(), MAP_DIM / 8, MAP_DIM / 8, 1); // MAP_DIM = 4096
	csLand->unbind(renderer->getDeviceContext());

}
