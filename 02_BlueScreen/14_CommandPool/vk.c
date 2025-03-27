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

uint32_t physicalDeviceCount = 0;
VkPhysicalDevice* vkPhysicalDevice_array = NULL;

//Device extension related variables
uint32_t enabledDeviceExtensionCount = 0;
//VK_KHR_SWAPPCHAIN_EXTENSION_NAME
const char* enabledDeviceExtensionNames_array[1];

//Vulkan device
VkDevice vkDevice = VK_NULL_HANDLE;

//device queue
VkQueue vkQueue = VK_NULL_HANDLE;

//color formate and color space
VkFormat vkFormate_color = VK_FORMAT_UNDEFINED;
VkColorSpaceKHR vkColorSpaceKHR = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

//presentation mode
VkPresentModeKHR vkPresentModeKHR = VK_PRESENT_MODE_FIFO_KHR;

//swapchain
int winWidth = WIN_WIDTH;
int winHeight = WIN_HEIGHT;
VkSwapchainKHR vkSwapchainKHR = VK_NULL_HANDLE;
VkExtent2D vkExtent2D_swapchain;

//swapchain Images and swapchain views
uint32_t swapchainImageCount = UINT32_MAX;
VkImage* swapchainImage_array = NULL;
VkImageView* swapchainImageView_array = NULL;

//command pool
VkCommandPool vkCommandPool = VK_NULL_HANDLE;

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
		fprintf(gpFile, "\n WinMain() - initialized function successed");
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
	
	//-----------------------------------------------------------------------------------------------
	//function declarations
	VkResult createVulkanInstance(void);
	VkResult getSupportedSurface(void);
	VkResult getPhysicalDevice(void);
	VkResult printVkInfo(void);
	VkResult createVulkanDevice(void);
	void getDeviceQueue(void);
	VkResult createSwapChain(VkBool32);
	VkResult createImagesAndImageView(void);
	VkResult createCommandPool(void);

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

	//Print vulkan Info
	vkResult = printVkInfo();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\ninlitialize() - printVkInfo() function failed %d", vkResult);
	}
	else
	{
		fprintf(gpFile, "\ninlitialize() - printVkInfo() function Successed");
	}
	
	//create Vulkan Device
	vkResult = createVulkanDevice();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\ninlitialize() - createVulkanDevice() function failed %d", vkResult);
	}
	else
	{
		fprintf(gpFile, "\ninlitialize() - createVulkanDevice() function Successed");
	}

	//getDeviceQueue
	getDeviceQueue();

	//swap chain
	vkResult = createSwapChain(FALSE);
	if (vkResult != VK_SUCCESS)
	{

		fprintf(gpFile, "\ninlitialize() - createSwapChain() function failed %d", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "\ninlitialize() - creatSwapChain() function Successed");
	}

	//create vulkan images and imageviews
	vkResult = createImagesAndImageView();
	if (vkResult != VK_SUCCESS)
	{

		fprintf(gpFile, "\ninlitialize() - createImagesAndImageView() function failed %d", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "\ninlitialize() - createImagesAndImageView() function Successed\n");
		fprintf(gpFile, "\ninlitialize() - createImagesAndImageView() gives swapchainImage count = %d\n",swapchainImageCount);
	}

	//create command pool
	vkResult = createCommandPool();
	if (vkResult != VK_SUCCESS)
	{

		fprintf(gpFile, " inlitialize() - createCommandPool() function failed %d", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "inlitialize() - createCommandPool() function Successed\n");
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
	
	//no need to destroy device queue

	//destroy vulkan device
	if (vkDevice)
	{
		vkDeviceWaitIdle(vkDevice);
		fprintf(gpFile, "\n unintialize()- vkDevice wait Idle is done");
	}
	//destroy commandpool
	if (vkCommandPool)
	{
		vkDestroyCommandPool(vkDevice,vkCommandPool,NULL);
		vkCommandPool = VK_NULL_HANDLE;
		fprintf(gpFile, "\n unintialize()- vkCommandPool destroy successed");
	}

	//destroy imageviews
	for (uint32_t i = 0; i < swapchainImageCount; i++)
	{
		vkDestroyImageView(vkDevice, swapchainImageView_array[i], NULL);
		fprintf(gpFile, "\n unintialize()- swapchain Imagesviews destroy successed");
	}

	//free array swapchainImageView_array
	if (swapchainImageView_array)
	{
		free(swapchainImageView_array);
		swapchainImageView_array = NULL;
		fprintf(gpFile, "\n unintialize()- swapchainImageView_array destroy successed");
	}

	////free swapchain Images
	//for (uint32_t i = 0; i < swapchainImageCount; i++)
	//{
	//	vkDestroyImage(vkDevice, swapchainImage_array[i], NULL);
	//	fprintf(gpFile, "\n unintialize()- swapchain Images destroy successed");
	//}
	
	//free array swapchainImage_array
	if (swapchainImage_array)
	{
		free(swapchainImage_array);
		swapchainImage_array = NULL;
		fprintf(gpFile, "\n unintialize()- swapchainImage_array destroy successed");
	}
	
	//destroy Swapchain
	if (vkSwapchainKHR)
	{
		vkDestroySwapchainKHR(vkDevice, vkSwapchainKHR, NULL);
		vkSwapchainKHR = VK_NULL_HANDLE;
		fprintf(gpFile, "\n unintialize()- vkSwapchainKHR destroy successed");
	}

	if (vkDevice)
	{
		vkDestroyDevice(vkDevice, NULL);
		vkDevice = VK_NULL_HANDLE;
		fprintf(gpFile, "\n unintialize()- vkDevice destroy successed");
	}

	//no need to destroy selected physical device

	//destroy vkSurfaceKHR
	if (vkSurfaceKHR)
	{
		vkDestroySurfaceKHR(vkInstance, vkSurfaceKHR, NULL);
		vkSurfaceKHR = VK_NULL_HANDLE;
		fprintf(gpFile, "\n unintialize()- vkSurfaceKHR destroy successed");
	}
	//step 5 - destroy vulkan instance
	if (vkInstance)
	{
		vkDestroyInstance(vkInstance,NULL);
		vkInstance = VK_NULL_HANDLE;
		fprintf(gpFile, "\n unintialize()- vkInstance destroy successed");
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
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
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
			fprintf(gpFile, "\n** printVkInfo() -  Successed to selected required physical device with graphics enabled ");
			break;
		}

	}
	if (bFound == VK_TRUE)
	{
		fprintf(gpFile, "\n == printVkInfo() -  Successed to selected required physical device with graphics enabled ");
		
	}
	else {
		free(vkPhysicalDevice_array);
		vkPhysicalDevice_array = NULL;
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

VkResult printVkInfo(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;


	//code
	fprintf(gpFile, "\n******************** Vulkan Information ************************");
	for (uint32_t i = 0; i < physicalDeviceCount ; i++)
	{
		VkPhysicalDeviceProperties vkPhysicalDeviceProperties;
		memset((void*)&vkPhysicalDeviceProperties, 0, sizeof(VkPhysicalDeviceProperties));
		vkGetPhysicalDeviceProperties(vkPhysicalDevice_array[i],&vkPhysicalDeviceProperties);


		uint32_t majorVersion = VK_API_VERSION_MAJOR(vkPhysicalDeviceProperties.apiVersion);//1.4.304   1 is major version number
		uint32_t minorVersion = VK_API_VERSION_MINOR(vkPhysicalDeviceProperties.apiVersion);//1.4.304   4 is minor version number
		uint32_t patchVersion = VK_API_VERSION_PATCH(vkPhysicalDeviceProperties.apiVersion);//1.4.304  304 patch number

		//API Version
		fprintf(gpFile, "\n API version = %d.%d.%d", majorVersion, minorVersion, patchVersion);

		//Device name
		fprintf(gpFile, "\n Device Name :%s", vkPhysicalDeviceProperties.deviceName);

		//Device Type
		switch (vkPhysicalDeviceProperties.deviceType)
		{
			case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
				fprintf(gpFile, "\n Device Type: Integrated GPU (iGPU) ");
				break;
			case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
				fprintf(gpFile, "\n Device Type: Discrete GPU (dGPU) ");
				break;
			case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
				fprintf(gpFile, "\n Device Type: Virtual vGPU");
				break;
			case VK_PHYSICAL_DEVICE_TYPE_CPU:
				fprintf(gpFile, "\n Device Type: CPU");
				break;
			case VK_PHYSICAL_DEVICE_TYPE_OTHER:
				fprintf(gpFile, "\n Device Type: Other");
				break;
			default:
				fprintf(gpFile, "\n Device Type: Unkown");
				break;
		}

		//Vendor ID
		fprintf(gpFile, "\n Vendor ID :0x%04x", vkPhysicalDeviceProperties.vendorID);

		//Device ID
		fprintf(gpFile, "\n Device ID :0x%04x", vkPhysicalDeviceProperties.deviceID);	
	}
	//freePhysicalDeviceArray
	if (vkPhysicalDevice_array)
	{
		free(vkPhysicalDevice_array);
		vkPhysicalDevice_array = NULL;
		fprintf(gpFile, "\n printVkInfo() -  Successed to free vkPhysicalDevice_array ");
	}

	return vkResult;
}

VkResult fillDeviceExtensionNames(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//step 1 - how many device extension are supported by vulkan driver and store in variable
	uint32_t deviceExtensionCount = 0;

	vkResult = vkEnumerateDeviceExtensionProperties(
		vkPhysicalDevice_selected,
		NULL,
		&deviceExtensionCount,NULL);

	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\n");
		fprintf(gpFile, " \n fillDeviceExtensionNames() - vkEnumerateDeviceExtensionProperties function failed");
		fprintf(gpFile, "\n Fist call to vkEnumerateDeviceExtensionProperties failed");
	}
	else
	{
		fprintf(gpFile, "\n fillDeviceExtensionNames() - vkEnumerateDeviceExtensionProperties function Successed");
		fprintf(gpFile, "\nFirst call to vkEnumerateDeviceExtensionProperties Successed");
	}

	//step 2 - allocate and fill struct VkExtensionProperties array corresponding to above count
	VkExtensionProperties* vkExtensionProperties_array = NULL;
	vkExtensionProperties_array = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * deviceExtensionCount);
	vkResult = vkEnumerateDeviceExtensionProperties(
		vkPhysicalDevice_selected,
		NULL,
		&deviceExtensionCount, vkExtensionProperties_array);

	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\n fillDeviceExtensionNames() - vkEnumerateDeviceExtensionProperties function failed");
		fprintf(gpFile, "\n Second call to vkEnumerateDeviceExtensionProperties failed");
	}
	else
	{
		fprintf(gpFile, "\n fillDeviceExtensionNames() - vkEnumerateDeviceExtensionProperties function Successed");
		fprintf(gpFile, "\nSecond call to vkEnumerateDeviceExtensionProperties Successed");
	}

	//step 3 - Fill and display a local string array of extension names obtain from vkExtensionProperties Array
	char** deviceExtensionNames_array = NULL;
	deviceExtensionNames_array = (char**)malloc(sizeof(char*) * deviceExtensionCount);
	for (uint32_t i = 0; i < deviceExtensionCount; i++)
	{
		//atta je pn nav(string) rahil tevdhi memory dyaychi
		deviceExtensionNames_array[i] = (char*)malloc(sizeof(char) * strlen(vkExtensionProperties_array[i].extensionName) + 1); // +1 means array for the null terminator (\0) 
		memcpy(deviceExtensionNames_array[i], vkExtensionProperties_array[i].extensionName, strlen(vkExtensionProperties_array[i].extensionName) + 1);
		fprintf(gpFile, "\n fillDeviceExtensionNames() - Device extension Name = %s\n", deviceExtensionNames_array[i]);

	}

	//step 4 - Free extensionPropertiesArray
	free(vkExtensionProperties_array);
	vkExtensionProperties_array = NULL;

	//step 5 - find wether above extension names array contain our required two extension
	VkBool32 vulkanSwapChainExtensionFound = VK_FALSE;

	for (uint32_t i = 0; i < deviceExtensionCount; i++)
	{
		if (strcmp(deviceExtensionNames_array[i], VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
		{
			vulkanSwapChainExtensionFound = VK_TRUE;
			enabledDeviceExtensionNames_array[enabledDeviceExtensionCount++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
		}
	}

	//step 6 - free local string array
	for (uint32_t i = 0; i < deviceExtensionCount; i++)
	{
		free(deviceExtensionNames_array[i]);
	}
	free(deviceExtensionNames_array);

	//step 7 - print wether our vulkan driver support required extension names or not
	if (vulkanSwapChainExtensionFound == VK_FALSE)
	{
		vkResult = VK_ERROR_INITIALIZATION_FAILED; //return hardcoded failuare
		fprintf(gpFile, "\nIn fillDeviceExtensionNames(),VK_KHR_SWAPCHAIN_EXTENSION_NAME extension not found");
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nIn fillDeviceExtensionNames(),VK_KHR_SWAPCHAIN_EXTENSION_NAME extension found");

	}
	

	// step 8 - Print only supported device extension names
	for (uint32_t i = 0; i < enabledDeviceExtensionCount; i++)
	{
		fprintf(gpFile, "\n In fillDeviceExtensionNames(),supported device extension names = %s\n", enabledDeviceExtensionNames_array[i]);

	}
	fprintf(gpFile, "\n In fillDeviceExtensionNames(),supported device extension count = %d\n", deviceExtensionCount);

	return vkResult;
}

VkResult createVulkanDevice(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//function declarations
	VkResult fillDeviceExtensionNames(void);

	//code
	//fill Device Extension
	vkResult = fillDeviceExtensionNames();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\n createVulkanDevice() - fillDeviceExtensionNames function failed");
	}
	else
	{
		fprintf(gpFile, "\n createVulkanDevice() - fillDeviceExtensionNames function Successed");
	}
	//newly added code
	float queuePriorities[1];
	queuePriorities[0] = 1.0;
	VkDeviceQueueCreateInfo vkDeviceQueueCreateInfo;
	memset((void*)&vkDeviceQueueCreateInfo, 0, sizeof(VkDeviceQueueCreateInfo));
	vkDeviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	vkDeviceQueueCreateInfo.pNext = NULL;
	vkDeviceQueueCreateInfo.flags = 0;
	vkDeviceQueueCreateInfo.queueFamilyIndex = ghraphicsQueueFamilyIndex_selected;
	vkDeviceQueueCreateInfo.queueCount = 1;
	vkDeviceQueueCreateInfo.pQueuePriorities = queuePriorities;

	//initialize vkDeviceCreateInfo structure
	VkDeviceCreateInfo vkDeviceCreateInfo;
	memset((void*)&vkDeviceCreateInfo, 0, sizeof(VkDeviceCreateInfo));
	vkDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	vkDeviceCreateInfo.pNext = NULL;
	vkDeviceCreateInfo.flags = 0;
	vkDeviceCreateInfo.enabledExtensionCount = enabledDeviceExtensionCount;
	vkDeviceCreateInfo.ppEnabledExtensionNames = enabledDeviceExtensionNames_array;
	vkDeviceCreateInfo.enabledLayerCount = 0;
	vkDeviceCreateInfo.ppEnabledLayerNames = NULL;
	vkDeviceCreateInfo.pEnabledFeatures = NULL;
	vkDeviceCreateInfo.queueCreateInfoCount = 1;
	vkDeviceCreateInfo.pQueueCreateInfos = &vkDeviceQueueCreateInfo;

	vkResult = vkCreateDevice(vkPhysicalDevice_selected,&vkDeviceCreateInfo,NULL,&vkDevice);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\n createVulkanDevice() - vkCreateDevice function failed");
	}
	else
	{
		fprintf(gpFile, "\n createVulkanDevice() - vkCreateDevice function Successed");
	}
	return vkResult;
}

void getDeviceQueue(void)
{
	//code
	vkGetDeviceQueue(vkDevice, ghraphicsQueueFamilyIndex_selected, 0, &vkQueue);
	if(vkQueue == VK_NULL_HANDLE)
	{

		fprintf(gpFile, "\n getDeviceQueue() - vkGetDeviceQueue to return null");
		return;
	}
	else
	{
		fprintf(gpFile, "\n getDeviceQueue() - vkGetDeviceQueue function Successed");
	}
	
}

VkResult getPhysicalDeviceSurfaceFormatAndColorSpace(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//code
	//get the count of supported color formate
	uint32_t formatCount = 0;
	vkResult= vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice_selected, vkSurfaceKHR,&formatCount,NULL);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\n getPhysicalDeviceSurfaceFormatAndColorSpace() -first vkGetPhysicalDeviceSurfaceFormatsKHR function failed");
	}
	else if (formatCount == 0)
	{
		fprintf(gpFile, "\n getPhysicalDeviceSurfaceFormatAndColorSpace() - vkGetPhysicalDeviceSurfaceFormatsKHR() return 0");
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\n getPhysicalDeviceSurfaceFormatAndColorSpace() -first vkGetPhysicalDeviceSurfaceFormatsKHR function Successed");
	}

	//declare and allocate VkSurfaceFormatKHR array
	VkSurfaceFormatKHR* vkSurfaceFormatKHR_array = (VkSurfaceFormatKHR*)malloc(formatCount*sizeof(VkSurfaceFormatKHR));

	//filling the array
	vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice_selected, vkSurfaceKHR, &formatCount, vkSurfaceFormatKHR_array);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\n getPhysicalDeviceSurfaceFormatAndColorSpace() -second vkGetPhysicalDeviceSurfaceFormatsKHR function failed");
	}
	else
	{
		fprintf(gpFile, "\n getPhysicalDeviceSurfaceFormatAndColorSpace() -second vkGetPhysicalDeviceSurfaceFormatsKHR function Successed");
	}
	//decide the surface color fomate first
	if (formatCount ==1 && vkSurfaceFormatKHR_array[0].format == VK_FORMAT_UNDEFINED)
	{
		vkFormate_color = VK_FORMAT_R8G8B8_UNORM;
	}
	else {
		vkFormate_color = vkSurfaceFormatKHR_array[0].format;
	}

	//decide the surface color space 

	vkColorSpaceKHR = vkSurfaceFormatKHR_array[0].colorSpace;

	if (vkSurfaceFormatKHR_array)
	{
		free(vkSurfaceFormatKHR_array);
		vkSurfaceFormatKHR_array = NULL;
		fprintf(gpFile, "\n unintialize()- vkSurfaceFormatKHR_array free successed");
	}
	return vkResult;
}

VkResult getPhysicalDevicePresentMode(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//code
	//get the count of present mode
	uint32_t presentModesCount = 0;

	vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice_selected,vkSurfaceKHR,&presentModesCount,NULL);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\n getPhysicalDevicePresentMode() -first vkGetPhysicalDeviceSurfacePresentModesKHR function failed");
	}
	else if (presentModesCount == 0)
	{
		fprintf(gpFile, "\n getPhysicalDevicePresentMode() - vkGetPhysicalDeviceSurfacePresentModesKHR() return 0");
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\n getPhysicalDevicePresentMode() -first vkGetPhysicalDeviceSurfacePresentModesKHR function Successed");
	}

	//declare and allocate
	VkPresentModeKHR* vkPresentModeKHR_array = (VkPresentModeKHR*)malloc(presentModesCount * sizeof(VkPresentModeKHR));
	

	vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice_selected, vkSurfaceKHR, &presentModesCount, vkPresentModeKHR_array);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\n getPhysicalDevicePresentMode() -second vkGetPhysicalDeviceSurfacePresentModesKHR function failed");
	}
	else
	{
		fprintf(gpFile, "\n getPhysicalDevicePresentMode() -second vkGetPhysicalDeviceSurfacePresentModesKHR function Successed");
	}

	//decide present mode
	for (uint32_t i = 0; i < presentModesCount; i++)
	{
		if(vkPresentModeKHR_array[i]==VK_PRESENT_MODE_MAILBOX_KHR)
		{
			vkPresentModeKHR = VK_PRESENT_MODE_MAILBOX_KHR;
			fprintf(gpFile, "\n ****** In getPhysicalDevicePresentMode() vkPresentModeKHR - %d", vkPresentModeKHR);
			break;
		}
	}
	if (vkPresentModeKHR != VK_PRESENT_MODE_MAILBOX_KHR)
	{
		vkPresentModeKHR = VK_PRESENT_MODE_FIFO_KHR;
		fprintf(gpFile, "\n ***In getPhysicalDevicePresentMode() vkPresentModeKHR - %d", vkPresentModeKHR);
	}

	if (vkPresentModeKHR_array)
	{
		free(vkPresentModeKHR_array);
		vkPresentModeKHR_array = NULL;
		fprintf(gpFile, "\n getPhysicalDevicePresentMode()- vkPresentModeKHR_array free successed");
	}

	return vkResult;
}

VkResult createSwapChain(VkBool32 vsync)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//function declaration
	VkResult getPhysicalDeviceSurfaceFormatAndColorSpace(void);
	VkResult getPhysicalDevicePresentMode(void);

	//code
	//color formate and color space 
	vkResult = getPhysicalDeviceSurfaceFormatAndColorSpace();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\ncreateSwapChain() - getPhysicalDeviceSurfaceFormatAndColorSpace() function failed %d", vkResult);
	}
	else
	{
		fprintf(gpFile, "\ncreateSwapChain() - getPhysicalDeviceSurfaceFormatAndColorSpace() function Successed");
	}

	//step-1 get physical device surface capabilities
	VkSurfaceCapabilitiesKHR vkSurfaceCapabilitiesKHR;
	memset((void*)&vkSurfaceCapabilitiesKHR, 0, sizeof(VkSurfaceCapabilitiesKHR));

	//step-2 get PhysicalDeviceSurfaceCapabilities
	//second parameter - vulkan surface(display) for which the capabilities are needed.
	vkResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhysicalDevice_selected,vkSurfaceKHR,&vkSurfaceCapabilitiesKHR);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\n createSwapChain() - vkGetPhysicalDeviceSurfaceCapabilitiesKHR() function failed %d", vkResult);
	}
	else
	{
		fprintf(gpFile, "\n createSwapChain() - vkGetPhysicalDeviceSurfaceCapabilitiesKHR() function Successed");
	}

	//step-3 find out desired swap no. count
	uint32_t testingNumberOfSwapchainIamges = vkSurfaceCapabilitiesKHR.minImageCount+1; //count = 1+1 = 2
	uint32_t desiredNumberOfSwapchainImages = 0; 

	//5>0 && 5< 2
	if (vkSurfaceCapabilitiesKHR.maxImageCount > 0 && vkSurfaceCapabilitiesKHR.maxImageCount< testingNumberOfSwapchainIamges)
	{
		desiredNumberOfSwapchainImages = vkSurfaceCapabilitiesKHR.maxImageCount;
		fprintf(gpFile, "\n createSwapChain() - desiredNumberOfSwapchainImages maxImageCount = %d", vkSurfaceCapabilitiesKHR.maxImageCount);
	}
	else {
		desiredNumberOfSwapchainImages = vkSurfaceCapabilitiesKHR.minImageCount;
		fprintf(gpFile, "\n createSwapChain() - desiredNumberOfSwapchainImages minImageCount = %d", vkSurfaceCapabilitiesKHR.minImageCount);
	}

	//step-4 choose size of swapchain image
	memset((void*)&vkExtent2D_swapchain, 0, sizeof(VkExtent2D));
	if (vkSurfaceCapabilitiesKHR.currentExtent.width != UINT32_MAX)
	{
		vkExtent2D_swapchain.width = vkSurfaceCapabilitiesKHR.currentExtent.width;
		vkExtent2D_swapchain.height = vkSurfaceCapabilitiesKHR.currentExtent.height;
		fprintf(gpFile, "\n createSwapChain() - Swapchain image width x height =  %d x %d", vkExtent2D_swapchain.width, vkExtent2D_swapchain.height);
	}
	else {
		//if surface size is already define then swapchain image size must match with it
		VkExtent2D vkExtent2D;
		memset((void*)&vkExtent2D, 0, sizeof(VkExtent2D));
		vkExtent2D.width = (uint32_t)winWidth;
		vkExtent2D.height = (uint32_t)winHeight;
		vkExtent2D_swapchain.width = max(vkSurfaceCapabilitiesKHR.minImageExtent.width, min(vkSurfaceCapabilitiesKHR.maxImageExtent.width, vkExtent2D.width));
		vkExtent2D_swapchain.height = max(vkSurfaceCapabilitiesKHR.minImageExtent.height, min(vkSurfaceCapabilitiesKHR.maxImageExtent.height, vkExtent2D.height));
		fprintf(gpFile, "\n * createSwapChain() - Swapchain image width x height =  %d x %d", vkExtent2D_swapchain.width, vkExtent2D_swapchain.height);

	}

	//step 5 - set swapchain image usage flag
	//although VK_IMAGE_USGAE_TRANFER_SRC_BIT not useful here for triangle like simple use ,but it will useful in texture , FBO , compute

	VkImageUsageFlags vkImageUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

	//step-6 whether to consider pretransform ,flipping or not // jar kahi image rotation and flipping garjech nasel tr te avoid karaych 
	VkSurfaceTransformFlagBitsKHR vkSurfaceTransformFlagBitsKHR; //enum

	 // 0101 in binary
	 // 0011 in binary
	 // 0001 (1 in decimal)
	if(vkSurfaceCapabilitiesKHR.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		vkSurfaceTransformFlagBitsKHR = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else {
		vkSurfaceTransformFlagBitsKHR = vkSurfaceCapabilitiesKHR.currentTransform;
	}

	//step 7 - Present mode  // got FIFO for intel and mailbox for nvidia
	vkResult = getPhysicalDevicePresentMode();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\ncreateSwapChain() - getPhysicalDevicePresentMode() function failed %d", vkResult);
	}
	else
	{
		fprintf(gpFile, "\ncreateSwapChain() - getPhysicalDevicePresentMode() function Successed");
	}

	//step -8 - initialize vkSwapChainInfo structure
	VkSwapchainCreateInfoKHR vkSwapchainCreateInfoKHR;
	memset((void*)&vkSwapchainCreateInfoKHR, 0, sizeof(VkSwapchainCreateInfoKHR));
	vkSwapchainCreateInfoKHR.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR; // structure type
	vkSwapchainCreateInfoKHR.pNext = NULL;
	vkSwapchainCreateInfoKHR.flags = 0;
	vkSwapchainCreateInfoKHR.surface = vkSurfaceKHR;
	vkSwapchainCreateInfoKHR.minImageCount = desiredNumberOfSwapchainImages;
	vkSwapchainCreateInfoKHR.imageFormat = vkFormate_color;
	vkSwapchainCreateInfoKHR.imageColorSpace = vkColorSpaceKHR;
	vkSwapchainCreateInfoKHR.imageExtent.width = vkExtent2D_swapchain.width;
	vkSwapchainCreateInfoKHR.imageExtent.height = vkExtent2D_swapchain.height;
	vkSwapchainCreateInfoKHR.imageUsage = vkImageUsageFlags;
	vkSwapchainCreateInfoKHR.preTransform = vkSurfaceTransformFlagBitsKHR;
	vkSwapchainCreateInfoKHR.imageArrayLayers = 1; //mobile mhadhe asata view vr view, ethe nahi mhanun 1
	vkSwapchainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;//
	vkSwapchainCreateInfoKHR.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	vkSwapchainCreateInfoKHR.presentMode = vkPresentModeKHR;
	vkSwapchainCreateInfoKHR.clipped = VK_TRUE; // dusari window jar present window vr ali tr swapchain ne swata changes karave laganar mhanun yes

	vkResult = vkCreateSwapchainKHR(vkDevice, &vkSwapchainCreateInfoKHR, NULL/*allocator*/, &vkSwapchainKHR);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\ncreateSwapChain() - vkCreateSwapchainKHR() function failed %d", vkResult);
	}
	else
	{
		fprintf(gpFile, "\ncreateSwapChain() - vkCreateSwapchainKHR() function Successed");
	}


	return vkResult;
}

VkResult createImagesAndImageView(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//code
	//getImagesAndImageView
	vkResult = vkGetSwapchainImagesKHR(vkDevice, vkSwapchainKHR, &swapchainImageCount, NULL);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\n createImagesAndImageView() - first call vkGetSwapchainImagesKHR()  function failed %d\n", vkResult);
	}
	else
	{
		fprintf(gpFile, "\n createImagesAndImageView() -first call vkGetSwapchainImagesKHR() function Successed\n");
	}

	//allocate the swapchainImage array
	swapchainImage_array = (VkImage*)malloc(sizeof(VkImage)* swapchainImageCount);

	//fill the array by swapchain images
	vkResult = vkGetSwapchainImagesKHR(vkDevice, vkSwapchainKHR, &swapchainImageCount, swapchainImage_array);

	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\n createImagesAndImageView() - second call vkGetSwapchainImagesKHR()  function failed %d\n", vkResult);
	}
	else
	{
		fprintf(gpFile, "\n createImagesAndImageView() - second call vkGetSwapchainImagesKHR() function Successed\n");
	}

	//allocate array of swapchain imageview 
	swapchainImageView_array = (VkImageView*)malloc(sizeof(VkImageView) * swapchainImageCount);

	//initialize vkImageViewCreateInfo structure
	VkImageViewCreateInfo vkImageViewCreateInfo;
	memset((void*)&vkImageViewCreateInfo,0, sizeof(VkImageViewCreateInfo));
	vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	vkImageViewCreateInfo.pNext = NULL;
	vkImageViewCreateInfo.flags = 0;
	vkImageViewCreateInfo.format = vkFormate_color;
	vkImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R; //enum VkComponentSwizzle in which this member avaiable
	vkImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
	vkImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
	vkImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
	vkImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; 
	vkImageViewCreateInfo.subresourceRange.baseMipLevel = 0; //array start from 0
	vkImageViewCreateInfo.subresourceRange.levelCount = 1;
	vkImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	vkImageViewCreateInfo.subresourceRange.layerCount = 1;
	vkImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // cubemap pn asto jevha cube map

	//fill imageview array by using above struct
	for (uint32_t i = 0; i < swapchainImageCount; i++)
	{
		vkImageViewCreateInfo.image = swapchainImage_array[i];
		vkResult = vkCreateImageView(vkDevice, &vkImageViewCreateInfo, NULL, &swapchainImageView_array[i]);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "\n createImagesAndImageView() - vkCreateImageViews function failed for iteration %d .(%d)\n", i,vkResult);
		}
		else
		{
			fprintf(gpFile, "\n createImagesAndImageView() - vkCreateImageViews function Successed for iteration %d\n",i);
		}
	}

	return vkResult;
}

VkResult createCommandPool(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//code
	VkCommandPoolCreateInfo vkCommandPoolCreateInfo;
	memset((void*)&vkCommandPoolCreateInfo, 0, sizeof(VkCommandPoolCreateInfo));

	vkCommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	vkCommandPoolCreateInfo.pNext = NULL;
	vkCommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // create such type of command pool can reset
	vkCommandPoolCreateInfo.queueFamilyIndex = ghraphicsQueueFamilyIndex_selected;
	
	vkResult = vkCreateCommandPool(vkDevice,&vkCommandPoolCreateInfo,NULL,&vkCommandPool);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, " createCommandPool() - vkCreateCommandPool function failed \n",vkResult);
	}
	else
	{
		fprintf(gpFile, "createCommandPool() - vkCreateCommandPool function Successed \n");
	}

	return vkResult;
}