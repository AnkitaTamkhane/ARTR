// Header Files
#include <windows.h>
#include "vk.h"
#include<stdio.h> //for File IO functions
#include<stdlib.h> //for exit()

//Vulkan related header file
#define VK_USE_PLATFORM_WIN32_KHR 
#include <vulkan/vulkan.h>

//Vulkan related library
#pragma comment(lib,"vulkan-1.lib")

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

// Global function declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

const char* gpszAppName = "ARTR";

//Vulkan related variable
//instance extension related variables
uint32_t enabledIntanceExtensionCount = 0;

//VK_KHR_SURFACE_EXTENSION_NAME AND VK_KHR_WIN32_SURFACE_EXTENSION_NAME
const char* enabledIntsanceExtensionNames_array[2];

//Global function declarations
HWND ghwnd = NULL;
BOOL gbFullScreen = FALSE;
FILE* gpFile = NULL;
BOOL gbActiveWindow = FALSE;
int iRateVal = 0;

// Entry Point Function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	// Function declarations
	VkResult inlitialize(void);
	void display();
	void update(void);
	void unintialize(void);

	// Variable declarations
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[] = TEXT("MyWindow");
	int actualWindowHeight;
	int actualWindowWidth;
	int x;
	int y;
	int windowWidthSize = 500;
	int windowheightSize = 500;
	BOOL bDone = FALSE;
	VkResult vkResult = VK_SUCCESS;

	// Code

	if (fopen_s(&gpFile, "Log.text", "w") != 0)
	{
		MessageBox(NULL, TEXT("Creation of Log file failed. Exiting..."), TEXT("File I/O Error"), MB_OK);
		exit(0);
	}
	else
	{
		fprintf(gpFile, "Log file opend succesfully\n");
	}
	// Initialization of WNDCLASSEX structure
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.lpfnWndProc = WndProc;
	wndclass.hInstance = hInstance;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;
	wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));;

	//Center Window Code
	actualWindowWidth = GetSystemMetrics(SM_CXSCREEN);
	actualWindowHeight = GetSystemMetrics(SM_CYSCREEN);

	x = actualWindowWidth / 2;
	x = x - WIN_WIDTH / 2;

	y = actualWindowHeight / 2;
	y = y - WIN_HEIGHT / 2;

	// Registering WNDCLASSEX
	RegisterClassEx(&wndclass);

	// Create the window
	hwnd = CreateWindow(szAppName,
		TEXT("Ankita Sunil Tamkhane"),
		WS_OVERLAPPEDWINDOW,
		x,
		y,
		WIN_WIDTH,
		WIN_HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL);

	ghwnd = hwnd;

	//initialize
	//iRateVal = inlitialize();
	vkResult = inlitialize();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "WinMain() - initialized function failed");
		DestroyWindow(hwnd);
		hwnd = NULL;
	}
	else
	{
		fprintf(gpFile, "WinMain() - initialized function successed");
	}

	// Show Window
	ShowWindow(hwnd, iCmdShow);

	//Foregrounding and focusing the window
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);
	
	//Game Loop
	while (bDone == FALSE)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				bDone = TRUE;

			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);

			}
		}
		else
		{
			if (gbActiveWindow == TRUE)
			{
				//Render the scene
				display();

				//update the scene
				update();

			}
		}
	}

	unintialize();
	return((int)msg.wParam);
}

// Callback function
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	//function declaration
	void ToggleFullScreen();
	void resize(int, int);
	

	// code
	switch (iMsg)
	{
	case  WM_SETFOCUS:
		gbActiveWindow = TRUE;
		break;
	case  WM_KILLFOCUS:
		gbActiveWindow = FALSE;
		break;
	case WM_ERASEBKGND:
		//return(0);
		break; // as this is retendmode graphics there is WM_PAINT to paint 
	case WM_CHAR:
		switch (wParam)
		{
		case 27:
			if (gpFile)
			{
				fprintf(gpFile, "Log succesfully closed");
				fclose(gpFile);
				gpFile = NULL;
			}
			DestroyWindow(hwnd);
			break;
		case 'F':
		case 'f':
			ToggleFullScreen();
			break;
		default:
			break;
		}
		break;	
	case WM_SIZE :
		resize(LOWORD(lParam),HIWORD(lParam));
		break;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case  WM_DESTROY :
		PostQuitMessage(0);
		break;
	default :
		break;
	}

	return(DefWindowProc( hwnd,  iMsg,  wParam,  lParam));
}

void ToggleFullScreen()
{
	//variable declarations
	static DWORD dwStyle;
	static WINDOWPLACEMENT wp;
	MONITORINFO mi;

	//code
	wp.length = sizeof(WINDOWPLACEMENT);

	if (gbFullScreen == FALSE)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
		if (dwStyle & WS_OVERLAPPEDWINDOW)
		{
			mi.cbSize = sizeof(MONITORINFO);
			if (GetWindowPlacement(ghwnd, &wp) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
			{
				SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW); // we remove WS_OVERLAPPEDWINDOW here
				SetWindowPos(ghwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOZORDER | SWP_FRAMECHANGED);
			}

			ShowCursor(FALSE);
			gbFullScreen = TRUE;
		}
		fprintf(gpFile, "Full Screen\n");
	}
	else
	{
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);// we add WS_OVERLAPPEDWINDOW here
		SetWindowPlacement(ghwnd, &wp);
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);


		ShowCursor(TRUE);
		gbFullScreen = FALSE;
		fprintf(gpFile, "Regular Screen\n");
	}

}

VkResult inlitialize(void)
{
	//function declarations
	VkResult fillInstanceExtensionNames(void);

	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//code
	vkResult = fillInstanceExtensionNames();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "inlitialize() - fillInstanceExtensionNames function failed");
	}
	else
	{
		fprintf(gpFile, "inlitialize() - fillInstanceExtensionNames function Successed");
	}

	return vkResult;
}

void resize(int width, int height)
{
	//code
	if (height == 0) 
	{
		height = 1;   // to avoid divided by zero illegal instructure which may occure in future

	}

}

void display()
{
	//code

}

void update(void)
{
	//code
}

void unintialize(void)
{
	//function declarations
	void ToggleFullScreen(void);

	//variable declarations

	//code
	if (gbFullScreen)
	{
		ToggleFullScreen(); //to convert normal screen and then destroy
	}
	if (ghwnd)
	{
		DestroyWindow(ghwnd);
		ghwnd = NULL;
	}
	if (gpFile)
	{
		fprintf(gpFile, "Log succesfully closed");
		fclose(gpFile);
		gpFile = NULL;
	}

}
//////////////////////////////////////////// Vulkan Related Function//////////////////////////////////
VkResult fillInstanceExtensionNames(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//step 1 - how many instance extension are supported by vulkan driver and store in variable
	uint32_t instanceExtensionCount = 0;

	vkResult = vkEnumerateInstanceExtensionProperties(
		NULL/*konatya layer chi extension pahije,karan apalya vulkan chi sarv extension pahije*/,
		&instanceExtensionCount/*count aanel*/,
		NULL/*instance extension chi null karan property cha count mahiti nahi to second parameter mhadhun milnar*/);

	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\n");
		fprintf(gpFile, " fillInstanceExtensionNames() - vkEnumerateInstanceExtensionProperties function failed");
		fprintf(gpFile, "Fist call to vkEnumerateInstanceExtensionProperties failed");
	}
	else
	{
		fprintf(gpFile, "\nfillInstanceExtensionNames() - vkEnumerateInstanceExtensionProperties function Successed");
		fprintf(gpFile, "\nFirst call to vkEnumerateInstanceExtensionProperties Successed");
	}

	//step 2 - allocate and fill struct VkExtensionProperties array corresponding to above count
	VkExtensionProperties* vkExtensionProperties_array = NULL;
	vkExtensionProperties_array = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties)* instanceExtensionCount);
	vkResult = vkEnumerateInstanceExtensionProperties(
		NULL, 
		&instanceExtensionCount,
		vkExtensionProperties_array/*ethe ha array barala*/);

	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "fillInstanceExtensionNames() - vkEnumerateInstanceExtensionProperties function failed");
		fprintf(gpFile, "Second call to vkEnumerateInstanceExtensionProperties failed");
	}
	else
	{
		fprintf(gpFile, "\n fillInstanceExtensionNames() - vkEnumerateInstanceExtensionProperties function Successed");
		fprintf(gpFile, "\nSecond call to vkEnumerateInstanceExtensionProperties Successed");
	}

	//step 3 - Fill and display a local string array of extension names obtain from vkExtensionProperties Array
	char** instanceExtensionNames_array = NULL;
	instanceExtensionNames_array = (char**)malloc(sizeof(char*) * instanceExtensionCount);
	for (uint32_t i = 0; i < instanceExtensionCount; i++)
	{
		//atta je pn nav(string) rahil tevdhi memory dyaychi
		instanceExtensionNames_array[i] = (char*)malloc(sizeof(char)*strlen(vkExtensionProperties_array[i].extensionName)+1); // +1 means array for the null terminator (\0) 
		memcpy(instanceExtensionNames_array[i], vkExtensionProperties_array[i].extensionName, strlen(vkExtensionProperties_array[i].extensionName) + 1);
		fprintf(gpFile, "\nfillInstanceExtensionNames() - Vulkan Extension Name = %s\n", instanceExtensionNames_array[i]);

	}

	//step 4 - Free extensionPropertiesArray
	free(vkExtensionProperties_array);
	vkExtensionProperties_array = NULL;

	//step 5 - find wether above extension names array contain our required two extension
	VkBool32 vulkanSurfaceExtensionFound = VK_FALSE;
	VkBool32 vulkanWin32SurfaceExtensionFound = VK_FALSE;

	for (uint32_t i = 0; i < instanceExtensionCount; i++)
	{
		if (strcmp(instanceExtensionNames_array[i], VK_KHR_SURFACE_EXTENSION_NAME) == 0)
		{
			 vulkanSurfaceExtensionFound = VK_TRUE;
			 enabledIntsanceExtensionNames_array[enabledIntanceExtensionCount++] = VK_KHR_SURFACE_EXTENSION_NAME;
		}

		if (strcmp(instanceExtensionNames_array[i], VK_KHR_WIN32_SURFACE_EXTENSION_NAME) == 0)
		{
			vulkanWin32SurfaceExtensionFound = VK_TRUE;
			enabledIntsanceExtensionNames_array[enabledIntanceExtensionCount++] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
		}
	}

	//step 6 - free local string array
	for (uint32_t i = 0; i < instanceExtensionCount; i++)
	{
		free(instanceExtensionNames_array[i]);
	}
	free(instanceExtensionNames_array);

	//step 7 - print wether our vulkan driver support required extension names or not
	if (vulkanSurfaceExtensionFound == VK_FALSE)
	{
		vkResult = VK_ERROR_INITIALIZATION_FAILED; //return hardcoded failuare
		fprintf(gpFile, "\nIn fillInstanceExtensionNames(),VK_KHR_SURFACE_EXTENSION_NAME extension not found");
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nIn fillInstanceExtensionNames(),VK_KHR_SURFACE_EXTENSION_NAME extension found");

	}
	if (vulkanWin32SurfaceExtensionFound == VK_FALSE)
	{
		vkResult = VK_ERROR_INITIALIZATION_FAILED; //return hardcoded failuare
		fprintf(gpFile, "\nIn fillInstanceExtensionNames(),VK_KHR_WIN32_SURFACE_EXTENSION_NAME extension not found");
		return vkResult;
	}
	else {
		fprintf(gpFile, "\nIn fillInstanceExtensionNames(),VK_KHR_WIN32_SURFACE_EXTENSION_NAME extension found");

	}

	// step 8 - Print only supported extension names
	for (uint32_t i = 0; i < enabledIntanceExtensionCount; i++)
	{
		fprintf(gpFile, "\nIn fillInstanceExtensionNames(),supported extension names = %s\n", enabledIntsanceExtensionNames_array[i]);

	}


	return vkResult;
}