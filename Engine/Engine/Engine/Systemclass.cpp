#include "Systemclass.h"


SystemClass::SystemClass()
{
	m_Input = 0;
	m_Graphics = 0;
}

SystemClass::SystemClass(const SystemClass& other)
{
}

SystemClass::~SystemClass()
{
}

bool SystemClass::Initialize()
{
	int screenWidth, screenHeight;
	bool result;

	// initialize the width and height of the screen to zero before sending the variables into the function
	screenWidth = 0;
	screenHeight = 0;

	// initialize the windows api
	InitializeWindows(screenWidth, screenHeight);

	// create the input object. this object will be used to handle reading the keyboard input from the user
	m_Input = new InputClass;
	if(!m_Input)
	{
		return false;
	}

	// initialize the input object
	result = m_Input->Initialize(m_hinstance, m_hwnd, screenWidth, screenHeight);
	if(!result)
	{
		MessageBox(m_hwnd, L"Could not initialize the input object", L"Error", MB_OK);
		return false;
	}

	// create the graphics object. this object will handle rendering all the graphics for this application
	m_Graphics = new GraphicsClass;
	if(!m_Graphics)
	{
		return false;
	}

	// initialize the graphics object
	result = m_Graphics->Initialize(screenWidth, screenHeight, m_hwnd);
	if(!result)
	{
		return false;
	}

	return true;
}

void SystemClass::ShutDown()
{
	// release the graphics object
	if(m_Graphics)
	{
		m_Graphics->Shutdown();
		delete m_Graphics;
		m_Graphics = 0;
	}

	// release the input object
	if(m_Input)
	{
		m_Input->Shutdown();
		delete m_Input;
		m_Input = 0;
	}

	// shutdown the window
	ShutDownWindows();

	return;
}

void SystemClass::Run()
{
	MSG msg;
	bool done, result;

	//initialize the message structure
	ZeroMemory(&msg, sizeof(MSG));

	// loop until there is a quit message from the window or the user
	done = false;
	while (!done)
	{
		//handle the windows message
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// if windows signals to end the application then exit out
		if(msg.message == WM_QUIT)
		{
			done = true;
		}
		else
		{
			// otherwise do the frame processing
			result = Frame();
			if(!result)
			{
				done = true;
			}
		}

		// check if the user pressed escape and wants to quit
		if(m_Input->IsEscapePressed())
		{
			done = true;
		}
	}
}

bool SystemClass::Frame()
{
	bool result;
	int mouseX, mouseY, wheelZ;


	// do the input frame processing
	result = m_Input->Frame();
	if(!result)
	{
		return false;
	}

	// get the location of the mouse from the input object
	m_Input->GetMouseLocation(mouseX, mouseY, wheelZ);

	// do the frame processing for the grapics object
	int mousePressState = -1;

	if(m_Input->IsLeftMouseButtonPressed())
	{
		mousePressState = 0;
	}
	else if (m_Input->IsRightMouseButtonPressed())
	{
		mousePressState = 1;
	}

	result = m_Graphics->Frame(mouseX, mouseY, wheelZ, mousePressState);
	if (!result)
	{
		return false;
	}



	return true;
}

LRESULT CALLBACK SystemClass::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch (umsg)
	{
	default:
		return DefWindowProc(hwnd, umsg, wparam, lparam);
	}
}

void SystemClass::InitializeWindows(int& screenWidth, int& screenHeight)
{
	WNDCLASSEX wc;
	DEVMODE dmScreenSettings;
	int posX, posY;

	// get an external pointer to the object
	ApplicationHandle = this;

	// get the instance of the application
	m_hinstance = GetModuleHandle(NULL);

	// give the application a name
	m_applicationName = L"Engine";

	// set up the windows class with default setttings
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = m_applicationName;
	wc.cbSize = sizeof(WNDCLASSEX);

	// register the window class
	RegisterClassEx(&wc);

	// determine the resolution of the clients desktop screen
	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	// setup the screen setting depending on whether it is running in full screen or in windowed mode
	if(FULL_SCREEN)
	{
		// if full screen set the screen to maximum size of the users desktop and 32bit
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = (unsigned long)screenWidth;
		dmScreenSettings.dmPelsHeight = (unsigned long)screenHeight;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// change the display settings to full screen
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		// set the position of the window to the top left corner
		posX = posY = 0;
	}
	else
	{
		// if windowed then set it to 800x600 resolution
		screenWidth = 800;
		screenHeight = 600;

		// place the window in the middle of the screen
		posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;
	}

	// create the window with the screen settings and get the handle to it
	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName, m_applicationName, WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, posX, posY, screenWidth, screenHeight, NULL, NULL, m_hinstance, NULL);

	// bring the window up on the screen and set it as main focus
	ShowWindow(m_hwnd, SW_SHOW);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);

	// hide the mouse cursor
	ShowCursor(true);

	return;
}

void SystemClass::ShutDownWindows()
{
	// show the mouse cursor
	ShowCursor(true);

	// fix the display settings if leaving full screen mode
	if(FULL_SCREEN)
	{
		ChangeDisplaySettings(NULL, 0);
	}

	// remove the window
	DestroyWindow(m_hwnd);
	m_hwnd = NULL;

	// remove the application instance
	UnregisterClass(m_applicationName, m_hinstance);
	m_hinstance = NULL;

	// release the pointer to this class
	ApplicationHandle = NULL;

	return;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch (umessage)
	{
	// check if the window is being destroyed
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	// check if the window is being closed
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	
	// all other messages pass to the message handler in the system class
	default:
		return ApplicationHandle->MessageHandler(hwnd, umessage, wparam, lparam);
	}
}