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

//vulkan instance
VkInstance vkInstance = VK_NULL_HANDLE;

//vulkan presentation Surface
VkSurfaceKHR vkSurfaceKHR = VK_NULL_HANDLE;

//vulkan physical devices variable
VkPhysicalDevice vkPhysicalDevice_selected = VK_NULL_HANDLE;
uint32_t ghraphicsQueueFamilyIndex_selected = UINT32_MAX;
VkPhysicalDeviceMemoryProperties vkPhysicalDeviceMemoryProperties;


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
	////function declarations
	//VkResult fillInstanceExtensionNames(void);

	////variable declarations
	//VkResult vkResult = VK_SUCCESS;

	////code
	//vkResult = fillInstanceExtensionNames();
	//if (vkResult != VK_SUCCESS)
	//{
	//	fprintf(gpFile, "\ninlitialize() - fillInstanceExtensionNames function failed");
	//}
	//else
	//{
	//	fprintf(gpFile, "\ninlitialize() - fillInstanceExtensionNames function Successed");
	//}

	//return vkResult;

	//-----------------------------------------------------------------------------------------------
	//function declarations
	VkResult createVulkanInstance(void);
	VkResult getSupportedSurface(void);
	VkResult getPhysicalDevice(void);

	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//code
	vkResult = createVulkanInstance();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\ninlitialize() - createVulkanInstance function failed");
	}
	else
	{
		fprintf(gpFile, "\ninlitialize() - createVulkanInstance function Successed");
	}

	//Create vulkan presentation Surface
	vkResult = getSupportedSurface();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\ninlitialize() - getSupportedSurface function failed");
	}
	else
	{
		fprintf(gpFile, "\ninlitialize() - getSupportedSurface function Successed");
	}

	//select required physical device and its queue family index
	vkResult = getPhysicalDevice();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\ninlitialize() - getPhysicalDevice() function failed %d",vkResult);
	}
	else
	{
		fprintf(gpFile, "\ninlitialize() - getPhysicalDevice() function Successed");
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
	if (vkSurfaceKHR)
	{
		vkDestroySurfaceKHR(vkInstance, vkSurfaceKHR, NULL);
		vkSurfaceKHR = VK_NULL_HANDLE;
		fprintf(gpFile, "\nunintialize()- vkSurfaceKHR destroy successed");
	}
	//step 5 - destroy vulkan instance
	if (vkInstance)
	{
		vkDestroyInstance(vkInstance,NULL);
		vkInstance = VK_NULL_HANDLE;
		fprintf(gpFile, "\nunintialize()- vkInstance destroy successed");
	}

	if (gpFile)
	{
		fprintf(gpFile, "\nLog succesfully closed");
		fclose(gpFile);
		gpFile = NULL;
	}

	

}

//////////////////////////////////////////// Vulkan Related Function//////////////////////////////////
VkResult createVulkanInstance(void)
{
	//function declarations
	VkResult fillInstanceExtensionNames(void);

	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//code
	//step 1 - fill and initialize required extensions name
	vkResult = fillInstanceExtensionNames();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\ncreateVulkanInstance() - fillInstanceExtensionNames function failed");
	}
	else
	{
		fprintf(gpFile, "\ncreateVulkanInstance() - fillInstanceExtensionNames function Successed");
	}

	//step 2 - initialize struct VkApplicationInfo
	VkApplicationInfo vkApplicationInfo;
	memset((void*)&vkApplicationInfo, 0, sizeof(VkApplicationInfo));
	vkApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	vkApplicationInfo.pNext = NULL;
	vkApplicationInfo.pApplicationName = gpszAppName;
	vkApplicationInfo.applicationVersion = 1;
	vkApplicationInfo.pEngineName = gpszAppName;
	vkApplicationInfo.engineVersion = 1;
	vkApplicationInfo.apiVersion = VK_API_VERSION_1_4;

	//step 3 - initialize struct VkInstanceCreateInfo
	VkInstanceCreateInfo vkInstanceCreateInfo;
	memset((void*)&vkInstanceCreateInfo, 0, sizeof(VkInstanceCreateInfo));
	vkInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	vkInstanceCreateInfo.pNext = NULL;
	vkInstanceCreateInfo.pApplicationInfo = &vkApplicationInfo;
	vkInstanceCreateInfo.enabledExtensionCount = enabledIntanceExtensionCount;
	vkInstanceCreateInfo.ppEnabledExtensionNames = enabledIntsanceExtensionNames_array;

	//step 4 - call vkCreateInstance() to get VkInstance in a global variable and do error checking
	vkResult = vkCreateInstance(&vkInstanceCreateInfo,NULL/*custome memory allocator nahi , tuzya kadacha memory allocator use kar*/,&vkInstance);

	if (vkResult == VK_ERROR_INCOMPATIBLE_DRIVER)
	{
		fprintf(gpFile, "\ncreateVulkanInstance() - vkCreateInstance() failed due to incampatible driver - (%d)\n",vkResult);
		return vkResult;
	}
	else if (vkResult == VK_ERROR_EXTENSION_NOT_PRESENT)
	{
		fprintf(gpFile, "\ncreateVulkanInstance() - vkCreateInstance() failed due to required EXTENSION_NOT_PRESENT - (%d)\n", vkResult);
		return vkResult;
	}
	else if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\ncreateVulkanInstance() - vkCreateInstance() failed due to unknown reason - (%d)\n", vkResult);
		return vkResult;
	}
	else {
		fprintf(gpFile, "\ncreateVulkanInstance() - vkCreateInstance() Successed\n", vkResult);
	}

	return vkResult;
}

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

VkResult getSupportedSurface(void)
{
	//local variable
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	// declare and memset platform specific(windows , linux,android) surface create info structure.
	VkWin32SurfaceCreateInfoKHR  vkWin32SurfaceCreateInfoKHR;
	memset((void*)&vkWin32SurfaceCreateInfoKHR, 0, sizeof(VkWin32SurfaceCreateInfoKHR));
	vkWin32SurfaceCreateInfoKHR.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	vkWin32SurfaceCreateInfoKHR.pNext = NULL;
	vkWin32SurfaceCreateInfoKHR.flags = 0;
	vkWin32SurfaceCreateInfoKHR.hinstance = (HINSTANCE)GetWindowLongPtr(ghwnd, GWLP_HINSTANCE);
	vkWin32SurfaceCreateInfoKHR.hwnd = ghwnd;

	//now call vkCreate win32KHR() to create the presentation surface object.
	vkResult = vkCreateWin32SurfaceKHR(vkInstance,&vkWin32SurfaceCreateInfoKHR,NULL,&vkSurfaceKHR);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\n getSupportedSurface() - vkCreateWin32SurfaceKHR() function failed");
	}
	else
	{
		fprintf(gpFile, "\ngetSupportedSurface() - vkCreateWin32SurfaceKHR() function Successed");
	}

	return vkResult;
}

VkResult getPhysicalDevice(void)
{
	//local variable
	//variable declarations
	VkResult vkResult = VK_SUCCESS;
	uint32_t physicalDeviceCount = 0;
	VkPhysicalDevice* vkPhysicalDevice_array = NULL;
	

	/*VkResult vkEnumeratePhysicalDevices(
		VkInstance instance,
		uint32_t * pPhysicalDeviceCount,
		VkPhysicalDevice * pPhysicalDevices);*/

	vkResult = vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, NULL/*atta array nahi mahnun*/);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\n getPhysicalDevice() - first call vkEnumeratePhysicalDevices() function failed %d",vkResult);
	}
	else if (physicalDeviceCount == 0)
	{
		fprintf(gpFile, "\n getPhysicalDevice() -  vkEnumeratePhysicalDevices() resulted asphysicalDeviceCount = 0 ");
		return vkResult;
	}
	else
	{
	     fprintf(gpFile, "\n getPhysicalDevice() - first call vkEnumeratePhysicalDevices() function Successed");
	}

	vkPhysicalDevice_array = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice)* physicalDeviceCount);

	//step-4
	vkResult = vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, vkPhysicalDevice_array);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\n getPhysicalDevice() - second call vkEnumeratePhysicalDevices() function failed %d", vkResult);
	}
	else
	{
		fprintf(gpFile, "\n getPhysicalDevice() - second call vkEnumeratePhysicalDevices() function Successed");
	}

	//step-5
	VkBool32 bFound = VK_FALSE;
	for (uint32_t i = 0; i < physicalDeviceCount; i++)
	{
		uint32_t queueCount = UINT32_MAX;
		vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_array[i], &queueCount, NULL);
		VkQueueFamilyProperties* vkQueueFamilyProperties_array = NULL;
		vkQueueFamilyProperties_array = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties)* queueCount);
		vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_array[i], &queueCount, vkQueueFamilyProperties_array);

		VkBool32* isQueueSurfaceSupported_array = NULL;
		isQueueSurfaceSupported_array = (VkBool32*)malloc(sizeof(VkBool32)* queueCount);

		for (uint32_t j = 0; j < queueCount; j++)
		{
			vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice_array[i],j,vkSurfaceKHR, &isQueueSurfaceSupported_array[j]);
		}
		for (uint32_t j = 0; j < queueCount; j++)
		{
			if (vkQueueFamilyProperties_array[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				if (isQueueSurfaceSupported_array[j] == VK_TRUE)
				{
					vkPhysicalDevice_selected = vkPhysicalDevice_array[i];
					ghraphicsQueueFamilyIndex_selected = j;
					bFound = VK_TRUE;
					break;
				}
			}
		}
		if (isQueueSurfaceSupported_array)
		{
			free(isQueueSurfaceSupported_array);
			isQueueSurfaceSupported_array = NULL;
			fprintf(gpFile, "\n getPhysicalDevice() -  Successed to free isQueueSurfaceSupported_array ");
		}
		if (vkQueueFamilyProperties_array)
		{
			free(vkQueueFamilyProperties_array);
			vkQueueFamilyProperties_array = NULL;
			fprintf(gpFile, "\n getPhysicalDevice() -  Successed to free vkQueueFamilyProperties_array ");
		}

		if (bFound == VK_TRUE)
		{
			break;
		}

	}
	if (bFound == VK_TRUE)
	{
		fprintf(gpFile, "\n getPhysicalDevice() -  Successed to selected required physical device with graphics enabled ");
		if (vkPhysicalDevice_array)
		{
			free(vkPhysicalDevice_array);
			vkPhysicalDevice_array = NULL;
			fprintf(gpFile, "\n getPhysicalDevice() -  Successed to free vkPhysicalDevice_array ");
		}
	}
	else {
		fprintf(gpFile, "\n getPhysicalDevice() - failed to get graphics selected device ");
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;

	}
	memset((void*)&vkPhysicalDeviceMemoryProperties, 0, sizeof(VkPhysicalDeviceMemoryProperties));
	vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice_selected,&vkPhysicalDeviceMemoryProperties);

	VkPhysicalDeviceFeatures vkPhysicalDeviceFeatures;
	memset((void*)&vkPhysicalDeviceFeatures,0,sizeof(VkPhysicalDeviceFeatures));
	vkGetPhysicalDeviceFeatures(vkPhysicalDevice_selected,&vkPhysicalDeviceFeatures);

	if (vkPhysicalDeviceFeatures.tessellationShader)
	{
		fprintf(gpFile, "\n getPhysicalDevice() - selected physical device support tessellationShader");
	}
	else {
		fprintf(gpFile, "\n getPhysicalDevice() - selected physical device not support tessellationShader");
	}

	if (vkPhysicalDeviceFeatures.geometryShader)
	{
		fprintf(gpFile, "\n getPhysicalDevice() - selected physical device support geometryShader");
	}
	else {
		fprintf(gpFile, "\n getPhysicalDevice() - selected physical device not support geometryShader");
	}


	return vkResult;
}