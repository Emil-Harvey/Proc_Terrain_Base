// Main.cpp
#include "System.h"
#include "App1.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	App1* app = new App1();
	System* system;

	// Create the system object.
	system = new System(app, 1366, 768, true, false);//	1366, 768 | 1920, 1080

	// Initialize and run the system object.
	system->run();

	// Shutdown and release the system object.
	delete system;
	system = 0;

	return 0;
}