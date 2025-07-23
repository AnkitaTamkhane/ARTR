// Header Files
#include <windows.h>
#include "vk.h"
#include<stdio.h> //for File IO functions
#include<stdlib.h> //for exit()

//Vulkan related header file
#define VK_USE_PLATFORM_WIN32_KHR 
#include <vulkan/vulkan.h>

//glm related macro and header files
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

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

//VK_KHR_SURFACE_EXTENSION_NAME AND VK_KHR_WIN32_SURFACE_EXTENSION_NAME AND VK_EXT_DEBUG_REPORT_EXTENSION_NAME
const char* enabledIntsanceExtensionNames_array[3];

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

//command buffer
VkCommandBuffer* vkCommandBuffer_array = NULL;

//render pass
VkRenderPass vkRenderPass = VK_NULL_HANDLE;

//frameBuffer
VkFramebuffer* vkFramebuffer_array = NULL;

//semaphore
VkSemaphore vkSemaphore_backBuffer = VK_NULL_HANDLE;
VkSemaphore vkSemaphore_renderComplete = VK_NULL_HANDLE;

//fence
VkFence* vkFence_array = NULL;

//clear color value
VkClearColorValue vkClearColorValue;

//
BOOL bInitialized = FALSE;

uint32_t currentImageIndex = UINT32_MAX;

//validation
BOOL bValidation = TRUE;
uint32_t enabledValidationLayerCount = 0;
const char* enabledValidationLayersNames_array[1]; //for VK_LAYER_KHRONOUS_validation
VkDebugReportCallbackEXT vkDebugReportCallbackEXT = VK_NULL_HANDLE;
PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT_fnptr = NULL;

//Vertex Buffer related variables
typedef struct {
	VkBuffer vkBuffer;
	VkDeviceMemory vkDeviceMemory;
}VertexData; 

//position
VertexData vertexData_position;

//uniform related declarations
struct MyUniformData {
	glm::mat4 modelMatrix;
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
};

struct UniformData {
	VkBuffer vkBuffer;
	VkDeviceMemory vkDeviceMemory;
};
UniformData uniformData;

//Global function declarations
HWND ghwnd = NULL;
BOOL gbFullScreen = FALSE;
BOOL bWindowMinimized = FALSE;
FILE* gpFile = NULL;
BOOL gbActiveWindow = FALSE;
int iRateVal = 0;

//shader related variable
VkShaderModule vkShaderModule_vertex_shader = VK_NULL_HANDLE;
VkShaderModule vkShaderModule_fragment_shader = VK_NULL_HANDLE;

//descriptor set layout
VkDescriptorSetLayout vkDescriptorSetLayout = VK_NULL_HANDLE;

//pipeline layout
VkPipelineLayout vkPipelineLayout = VK_NULL_HANDLE;

//descriptor Pool
VkDescriptorPool vkDescriptorPool = VK_NULL_HANDLE;

//descriptor set
VkDescriptorSet vkDescriptorSet = VK_NULL_HANDLE;

VkViewport vkViewport;
VkRect2D vkRect2D_scissor;
VkPipeline vkPipeline = VK_NULL_HANDLE;



// Entry Point Function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	// Function declarations
	VkResult inlitialize(void);
	VkResult display(void);
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
				if (bWindowMinimized == FALSE)
				{ 				
					//Render the scene
					vkResult = display();
					if (vkResult != VK_FALSE && vkResult != VK_SUCCESS && vkResult != VK_ERROR_OUT_OF_DATE_KHR && vkResult != VK_SUBOPTIMAL_KHR)
					{
						fprintf(gpFile, " WinMain()-Called to display failed \n");
						bDone = TRUE;
					}

					//update the scene
					update();
				}

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
	VkResult resize(int, int);
	

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
		if (wParam == SIZE_MINIMIZED)
		{
			bWindowMinimized = TRUE;
		}
		else {
			bWindowMinimized = FALSE;
			resize(LOWORD(lParam), HIWORD(lParam));

		}
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
	VkResult createCommandBuffers(void);
	VkResult createVertexBuffer(void);
	VkResult createShaders(void);
	VkResult createDescriptorSetLayout(void);
	VkResult createPipelineLayout(void);
	VkResult createDescriptorPool(void);
	VkResult createDescriptorSet(void);
	VkResult createRenderPass(void);
	VkResult createPipeline(void);
	VkResult createFramebuffers(void);
	VkResult createSemaPhores(void);
	VkResult createFences(void);
	VkResult buildCommandBuffers(void);
	VkResult createUniformBuffer(void);
	

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

	//create command buffer
	vkResult = createCommandBuffers();
	if (vkResult != VK_SUCCESS)
	{

		fprintf(gpFile, " inlitialize() - createCommandBuffers() function failed %d", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "inlitialize() - createCommandBuffers() function Successed\n");
	}

	//create vertes buffer
	vkResult = createVertexBuffer();
	if (vkResult != VK_SUCCESS)
	{

		fprintf(gpFile, " inlitialize() - createVertexBuffer() function failed %d\n", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "inlitialize() - createVertexBuffer() function Successed\n");
	}

	//create uniform buffer
	vkResult = createUniformBuffer();
	if (vkResult != VK_SUCCESS)
	{

		fprintf(gpFile, " inlitialize() - createUniformBuffer() function failed %d\n", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "inlitialize() - createUniformBuffer() function Successed\n");
	}

	//create shader
	vkResult = createShaders();
	if (vkResult != VK_SUCCESS)
	{

		fprintf(gpFile, " inlitialize() - createShaders() function failed %d\n", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "inlitialize() - createShaders() function Successed\n");
	}
	
	//create descriptor set layout
	vkResult = createDescriptorSetLayout();
	if (vkResult != VK_SUCCESS)
	{

		fprintf(gpFile, " inlitialize() - createDescriptorSetLayout() function failed %d\n", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "inlitialize() - createDescriptorSetLayout() function Successed\n");
	}

	//create pipeline layout
	vkResult = createPipelineLayout();
	if (vkResult != VK_SUCCESS)
	{

		fprintf(gpFile, " inlitialize() - createPipelineLayout() function failed %d\n", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "inlitialize() - createPipelineLayout() function Successed\n");
	}

	//create descriptor pool
	vkResult = createDescriptorPool();
	if (vkResult != VK_SUCCESS)
	{

		fprintf(gpFile, " inlitialize() - createDescriptorPool() function failed %d\n", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "inlitialize() - createDescriptorPool() function Successed\n");
	}

	//create descriptor set
	vkResult = createDescriptorSet();
	if (vkResult != VK_SUCCESS)
	{

		fprintf(gpFile, " inlitialize() - createDescriptorSet() function failed %d\n", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "inlitialize() - createDescriptorSet() function Successed\n");
	}
	//create render pass
	vkResult = createRenderPass();
	if (vkResult != VK_SUCCESS)
	{

		fprintf(gpFile, " inlitialize() - createRenderPass() function failed %d", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "inlitialize() - createRenderPass() function Successed\n");
	}

	//create pipeline
	vkResult = createPipeline();
	if (vkResult != VK_SUCCESS)
	{

		fprintf(gpFile, " inlitialize() - createPipeline() function failed %d", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "inlitialize() - createPipeline() function Successed\n");
	}


	//create framebuffer
	vkResult = createFramebuffers();
	if (vkResult != VK_SUCCESS)
	{

		fprintf(gpFile, " inlitialize() - createFramebuffers() function failed %d", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "inlitialize() - createFramebuffers() function Successed\n");
	}

	//create semaphores
	vkResult = createSemaPhores();
	if (vkResult != VK_SUCCESS)
	{

		fprintf(gpFile, " inlitialize() - createSemaPhores() function failed %d", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "inlitialize() - createSemaPhores() function Successed\n");
	}

	//create fences
	vkResult = createFences();
	if (vkResult != VK_SUCCESS)
	{

		fprintf(gpFile, " inlitialize() - createFences() function failed %d", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "inlitialize() - createFences() function Successed\n");
	}

	//initialize clear color value
	memset((void*)&vkClearColorValue, 0, sizeof(VkClearColorValue));
	vkClearColorValue.float32[0] = 0.0f;
	vkClearColorValue.float32[1] = 0.0f;
	vkClearColorValue.float32[2] = 1.0f;
	vkClearColorValue.float32[3] = 1.0f; // analogous to GLClear color

	//Build CommandBuffers
	vkResult = buildCommandBuffers();
	if (vkResult != VK_SUCCESS)
	{

		fprintf(gpFile, " inlitialize() - buildCommandBuffers() function failed %d", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "inlitialize() - buildCommandBuffers() function Successed\n");
	}

	//initialization is completed
	bInitialized = TRUE;
	fprintf(gpFile, "inlitialize() function Completed\n");
	return vkResult;
}

VkResult resize(int width, int height)
{

	//function declarations
	VkResult createSwapChain(VkBool32);
	VkResult createImagesAndImageView(void);
	VkResult createCommandBuffers(void);
	VkResult createPipelineLayout(void);
	VkResult createPipeline(void);
	VkResult createRenderPass(void);
	VkResult createFramebuffers(void);
	VkResult buildCommandBuffers(void);

	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//code
	if (height <= 0) 
	{
		height = 1;   // to avoid divided by zero illegal instructure which may occure in future
	}

	//checked the bInitiazied variable
	if (bInitialized == FALSE)
	{
		fprintf(gpFile, " Resize() : Initizations yet not completed or failed\n");
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		
	}
	//as re-creation of swap chain is needed we are going to repeat many step of initiazied hense set bInitialized false again
	bInitialized = FALSE;

	//set global winWidth and WinHeight variables
	winWidth = width;
	winHeight = height;

	//wait for to device to complete inhand task 
	if (vkDevice)
	{
		vkDeviceWaitIdle(vkDevice);
		fprintf(gpFile, "resize()- vkDevice wait Idle is done\n");
	}

	//check presence of swapchain
	if (vkSwapchainKHR == VK_NULL_HANDLE)
	{
		fprintf(gpFile, "resize()- swapChain is already NULL so can not proceed \n");
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}

	//destroy Framebuffer
	for (uint32_t i = 0; i < swapchainImageCount; i++)
	{
		vkDestroyFramebuffer(vkDevice, vkFramebuffer_array[i], NULL);
		fprintf(gpFile, "\n resize()- FrameBuffer[%d] destroy successed\n", i);
	}

	if (vkFramebuffer_array)
	{
		free(vkFramebuffer_array);
		vkFramebuffer_array = NULL;
		fprintf(gpFile, "\n resize()- vkFramebuffer_array destroy successed\n");
	}

	
	//destroy commandbuffer
	for (uint32_t i = 0; i < swapchainImageCount; i++)
	{
		vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_array[i]);
		fprintf(gpFile, "\n resize()- CommandBuffers[%d] destroy successed", i);
	}
	//free actual commandBuffer array
	if (vkCommandBuffer_array)
	{
		free(vkCommandBuffer_array);
		vkCommandBuffer_array = NULL;
		fprintf(gpFile, "\n resize()- vkCommandBuffer_array destroy successed");
	}

	//destroy pipeline
	if (vkPipeline)
	{
		vkDestroyPipeline(vkDevice, vkPipeline, NULL);
		vkPipeline = VK_NULL_HANDLE;
		fprintf(gpFile, "\n resize()- vkPipeline destroy successed");

	}

	//destroy PipelineLayout
	if (vkPipelineLayout)
	{
		vkDestroyPipelineLayout(vkDevice, vkPipelineLayout, NULL);
		vkPipelineLayout = VK_NULL_HANDLE;
		fprintf(gpFile, "\n resize()- vkPipelineLayout destroy successed");
	}
	//destroy renderpass
	if (vkRenderPass)
	{
		vkDestroyRenderPass(vkDevice, vkRenderPass, NULL);
		vkRenderPass = VK_NULL_HANDLE;
		fprintf(gpFile, "\n resize()- vkRenderPass destroy successed");
	}

	//destroy imageviews
	for (uint32_t i = 0; i < swapchainImageCount; i++)
	{
		vkDestroyImageView(vkDevice, swapchainImageView_array[i], NULL);
		fprintf(gpFile, "\n resize()- swapchain Imagesviews[%d] destroy successed", i);
	}

	//free array swapchainImageView_array
	if (swapchainImageView_array)
	{
		free(swapchainImageView_array);
		swapchainImageView_array = NULL;
		fprintf(gpFile, "\n resize()- swapchainImageView_array destroy successed");
	}

	////free swapchain Images
	//for (uint32_t i = 0; i < swapchainImageCount; i++)
	//{
	//	vkDestroyImage(vkDevice, swapchainImage_array[i], NULL);
	//	fprintf(gpFile, "\n resize()- swapchain Images destroy successed");

	//}

	//free array swapchainImage_array
	if (swapchainImage_array)
	{
		free(swapchainImage_array);
		swapchainImage_array = NULL;
		fprintf(gpFile, "\n resize()- swapchainImage_array destroy successed");
	}

	//destroy swapchain
	if (vkSwapchainKHR)
	{
		vkDestroySwapchainKHR(vkDevice, vkSwapchainKHR, NULL);
		vkSwapchainKHR = VK_NULL_HANDLE;
		fprintf(gpFile, "\n resize()- vkSwapchainKHR destroy successed");
	}

	//---Re-Create for Resize()------

	//create swapchain
	vkResult = createSwapChain(FALSE);
	if (vkResult != VK_SUCCESS)
	{

		fprintf(gpFile, "resize() - createSwapChain() function failed %d\n", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}

	//creat swapchain images and image view
	vkResult = createImagesAndImageView();
	if (vkResult != VK_SUCCESS)
	{

		fprintf(gpFile, "resize() - createImagesAndImageView() function failed %d\n", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}
	//create render pass
	vkResult = createRenderPass();
	if (vkResult != VK_SUCCESS)
	{

		fprintf(gpFile, " resize() - createRenderPass() function failed %d\n", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}


	//create pipeline layout
	vkResult = createPipelineLayout();
	if (vkResult != VK_SUCCESS)
	{

		fprintf(gpFile, "resize() - createPipelineLayout() function failed %d\n", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}

	//create pipeline
	vkResult = createPipeline();
	if (vkResult != VK_SUCCESS)
	{

		fprintf(gpFile, "resize() - createPipeline() function failed %d\n", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}

	//create framebuffer
	vkResult = createFramebuffers();
	if (vkResult != VK_SUCCESS)
	{

		fprintf(gpFile, "resize() - createFramebuffers() function failed %d\n", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}

	//create command buffer
	vkResult = createCommandBuffers();
	if (vkResult != VK_SUCCESS)
	{

		fprintf(gpFile, "resize()- createCommandBuffers() function failed %d\n", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}

	//Build CommandBuffers
	vkResult = buildCommandBuffers();
	if (vkResult != VK_SUCCESS)
	{

		fprintf(gpFile, " resize() - buildCommandBuffers() function failed %d\n", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}

	bInitialized = TRUE;
	return vkResult;
}

VkResult display(void)
{
	//function declaration
	VkResult resize(int, int);
	VkResult updateUniformBuffer(void);

	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//code
	//if controls come here before initialization gets completed return false
	if (bInitialized == FALSE)
	{
		fprintf(gpFile, "display() - initialization yet not completed\n");
		return (VkResult)VK_FALSE;
	}
	//acquire index of next swap chain image
	vkResult = vkAcquireNextImageKHR(vkDevice,vkSwapchainKHR,UINT64_MAX/*nano second*/, vkSemaphore_backBuffer,VK_NULL_HANDLE, &currentImageIndex);
	if (vkResult != VK_SUCCESS)
	{
		if (vkResult == VK_ERROR_OUT_OF_DATE_KHR || vkResult == VK_SUBOPTIMAL_KHR)
		{
			resize(winWidth,winHeight);
		}
		else {
			fprintf(gpFile, "\n display() - vkAcquireNextImageKHR() function failed %d", vkResult);
			return vkResult;
		}
		
	}

	//use fence to allows host to wait for completion of execution of previous command buffer
	vkResult = vkWaitForFences(vkDevice,1,&vkFence_array[currentImageIndex],VK_TRUE, UINT64_MAX);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\n display() - vkWaitForFences() function failed %d", vkResult);
		return vkResult;
	}

	//make fences ready for execution of next command buffer
	vkResult = vkResetFences(vkDevice,1,&vkFence_array[currentImageIndex]);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\n display() - vkResetFence() function failed %d", vkResult);
		return vkResult;
	}

	// one of the member of VkSubmitInfo Struture array of pipe line stage ,we have only one of completion of color --
	//attachment output still we need one member array

	const VkPipelineStageFlags waitDSTStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	//declare and memset and initialize VkSubmitInfo structure
	VkSubmitInfo vkSubmitInfo;
	memset((void*)&vkSubmitInfo, 0, sizeof(VkSubmitInfo));
	vkSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	vkSubmitInfo.pNext = NULL;
	vkSubmitInfo.pWaitDstStageMask = &waitDSTStageMask;
	vkSubmitInfo.waitSemaphoreCount = 1;
	vkSubmitInfo.pWaitSemaphores = &vkSemaphore_backBuffer;
	vkSubmitInfo.commandBufferCount = 1;
	vkSubmitInfo.pCommandBuffers = &vkCommandBuffer_array[currentImageIndex];
	vkSubmitInfo.signalSemaphoreCount = 1;
	vkSubmitInfo.pSignalSemaphores = &vkSemaphore_renderComplete;

	//now submit above work to the queue
	vkResult = vkQueueSubmit(vkQueue,1,&vkSubmitInfo,vkFence_array[currentImageIndex]);

	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\n display() - vkQueueSubmit() function failed %d", vkResult);
		return vkResult;
	}

	//we are going to present rendered image after declaring VkPresentInfoKHR Structure
	VkPresentInfoKHR vkPresentInfoKHR;
	memset((void*)&vkPresentInfoKHR, 0, sizeof(VkPresentInfoKHR));
	vkPresentInfoKHR.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	vkPresentInfoKHR.pNext = NULL;
	vkPresentInfoKHR.swapchainCount = 1;
	vkPresentInfoKHR.pSwapchains = &vkSwapchainKHR;
	vkPresentInfoKHR.pImageIndices = &currentImageIndex;
	vkPresentInfoKHR.waitSemaphoreCount = 1;
	vkPresentInfoKHR.pWaitSemaphores = &vkSemaphore_renderComplete;

	//now preset the queue
	vkResult = vkQueuePresentKHR(vkQueue,&vkPresentInfoKHR);
	
	if (vkResult != VK_SUCCESS)
	{
		if (vkResult == VK_ERROR_OUT_OF_DATE_KHR || vkResult == VK_SUBOPTIMAL_KHR)
		{
			resize(winWidth, winHeight);
		}
		else {
			fprintf(gpFile, "\n display() - vkQueuePresentKHR() function failed %d", vkResult);
			return vkResult;
		}
		
	}

	vkResult = updateUniformBuffer();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\n display() - updateUniformBuffer() function failed %d", vkResult);
		return vkResult;
	}

	vkDeviceWaitIdle(vkDevice);

	return vkResult;
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

	//destroy fences
	for (uint32_t i = 0; i < swapchainImageCount; i++)
	{
		vkDestroyFence(vkDevice, vkFence_array[i], NULL);
		fprintf(gpFile, "\n unintialize()- Fence[%d] destroy successed\n", i);
	}

	if (vkFence_array)
	{
		free(vkFence_array);
		vkFence_array = NULL;
		fprintf(gpFile, "\n unintialize()- vkFence_array destroy successed\n");
	}

	//destroy vkSemaphore_renderComplete
	if (vkSemaphore_renderComplete)
	{
		vkDestroySemaphore(vkDevice, vkSemaphore_renderComplete, NULL);
		vkSemaphore_renderComplete = VK_NULL_HANDLE;
		fprintf(gpFile, "\n unintialize()- vkSemaphore_renderComplete destroy successed\n");
	}
	//destroy vkSemaphore_backBuffer
	if (vkSemaphore_backBuffer)
	{
		vkDestroySemaphore(vkDevice, vkSemaphore_backBuffer, NULL);
		vkSemaphore_backBuffer = VK_NULL_HANDLE;
		fprintf(gpFile, "\n unintialize()- vkSemaphore_backBuffer destroy successed\n");
	}

	//destroy Framebuffer
	for (uint32_t i = 0; i < swapchainImageCount; i++)
	{
		vkDestroyFramebuffer(vkDevice, vkFramebuffer_array[i], NULL);
		fprintf(gpFile, "\n unintialize()- FrameBuffer[%d] destroy successed\n", i);
	}

	if (vkFramebuffer_array)
	{
		free(vkFramebuffer_array);
		vkFramebuffer_array = NULL;
		fprintf(gpFile, "\n unintialize()- vkFramebuffer_array destroy successed\n");
	}

	//destroy uniform
	if (uniformData.vkBuffer)
	{
		vkDestroyBuffer(vkDevice, uniformData.vkBuffer, NULL);
		uniformData.vkBuffer = VK_NULL_HANDLE;
		fprintf(gpFile, "\n unintialize()- uniformData.vkBuffer destroy successed");

	}

	if (uniformData.vkDeviceMemory)
	{
		vkFreeMemory(vkDevice, uniformData.vkDeviceMemory, NULL);
		uniformData.vkDeviceMemory = VK_NULL_HANDLE;
		fprintf(gpFile, "\n unintialize()- uniformData.vkDeviceMemory destroy successed");
	}

	if (vertexData_position.vkDeviceMemory)
	{
		vkFreeMemory(vkDevice, vertexData_position.vkDeviceMemory, NULL);
		vertexData_position.vkDeviceMemory = VK_NULL_HANDLE;
		fprintf(gpFile, "\n unintialize()- vertexData_position.vkDeviceMemory destroy successed");
	}

	if (vertexData_position.vkBuffer)
	{
		vkDestroyBuffer(vkDevice, vertexData_position.vkBuffer, NULL);
		vertexData_position.vkBuffer = VK_NULL_HANDLE;
		fprintf(gpFile, "\n unintialize()- vertexData_position.vkBuffer destroy successed");
	}
	if (vkDescriptorSetLayout)
	{
		vkDestroyDescriptorSetLayout(vkDevice, vkDescriptorSetLayout,NULL);
		vkDescriptorSetLayout = VK_NULL_HANDLE;
		fprintf(gpFile, "\n unintialize()- vkDescriptorSetLayout destroy successed");
	}

	//destroy discriptor pool
	//when discriptot pool is distroyed discriptor set created by pool destroy implicitly
	if (vkDescriptorPool)
	{
		vkDestroyDescriptorPool(vkDevice, vkDescriptorPool, NULL);
		vkDescriptorPool = VK_NULL_HANDLE;
		vkDescriptorSet = VK_NULL_HANDLE;
		fprintf(gpFile, "\n unintialize()- vkDescriptorPool and vkDescriptorSet are destroy successed");
	}

	if (vkPipelineLayout)
	{
		vkDestroyPipelineLayout(vkDevice, vkPipelineLayout, NULL);
		vkPipelineLayout = VK_NULL_HANDLE;
		fprintf(gpFile, "\n unintialize()- vkPipelineLayout destroy successed");
	}

	if (vkPipeline)
	{
		vkDestroyPipeline(vkDevice, vkPipeline,NULL);
		vkPipeline = VK_NULL_HANDLE;
		fprintf(gpFile, "\n unintialize()- vkPipeline destroy successed");

	}
	if (vkRenderPass)
	{
		vkDestroyRenderPass(vkDevice, vkRenderPass, NULL);
		vkRenderPass = VK_NULL_HANDLE;
		fprintf(gpFile, "\n unintialize()- vkRenderPass destroy successed");
	}

	if (vkShaderModule_fragment_shader)
	{
		vkDestroyShaderModule(vkDevice, vkShaderModule_fragment_shader,NULL);
		vkShaderModule_fragment_shader = VK_NULL_HANDLE;
		fprintf(gpFile, "\n unintialize()- vkShaderModule_fragment_shader destroy successed");
	}

	if (vkShaderModule_vertex_shader)
	{
		vkDestroyShaderModule(vkDevice, vkShaderModule_vertex_shader, NULL);
		vkShaderModule_vertex_shader = VK_NULL_HANDLE;
		fprintf(gpFile, "\n unintialize()- vkShaderModule_vertex_shader destroy successed");
	}

	for (uint32_t i = 0; i < swapchainImageCount; i++)
	{
		vkFreeCommandBuffers(vkDevice, vkCommandPool, 1,&vkCommandBuffer_array[i]);
		fprintf(gpFile, "\n unintialize()- CommandBuffers[%d] destroy successed",i);
	}

	//free actual commandBuffer array
	if (vkCommandBuffer_array)
	{
		free(vkCommandBuffer_array);
		vkCommandBuffer_array = NULL;
		fprintf(gpFile, "\n unintialize()- vkCommandBuffer_array destroy successed");
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
		fprintf(gpFile, "\n unintialize()- swapchain Imagesviews[%d] destroy successed",i);
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

	if (vkDebugReportCallbackEXT && vkDestroyDebugReportCallbackEXT_fnptr)
	{
		vkDestroyDebugReportCallbackEXT_fnptr(vkInstance, vkDebugReportCallbackEXT,NULL);
		vkDebugReportCallbackEXT = VK_NULL_HANDLE;
		vkDestroyDebugReportCallbackEXT_fnptr = NULL;
		fprintf(gpFile, "\n unintialize()- vkDebugReportCallbackEXT destroy successed");
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
	VkResult fillValidationLayerNames(void);
	VkResult createValidationCallback(void);

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

	if (bValidation == TRUE)
	{
		//fill the validation
		vkResult = fillValidationLayerNames();
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "\ncreateVulkanInstance() - fillValidationLayerNames function failed");
		}
		else
		{
			fprintf(gpFile, "\ncreateVulkanInstance() - fillValidationLayerNames function Successed");
		}
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
	if (bValidation == TRUE)
	{
		vkInstanceCreateInfo.enabledLayerCount = enabledValidationLayerCount;
		vkInstanceCreateInfo.ppEnabledLayerNames = enabledValidationLayersNames_array;
	}
	else
	{
		vkInstanceCreateInfo.enabledLayerCount = 0;
		vkInstanceCreateInfo.ppEnabledLayerNames = NULL;
	}


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

	//do for validation callbacks
	if (bValidation == TRUE)
	{
		vkResult = createValidationCallback();
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "createVulkanInstance() - createValidationCallback function failed\n");
		}
		else
		{
			fprintf(gpFile, "createVulkanInstance() - createValidationCallback function Successed\n");
		}
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
	VkBool32 debugReportExtensionFound = VK_FALSE;

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

		if (strcmp(instanceExtensionNames_array[i], VK_EXT_DEBUG_REPORT_EXTENSION_NAME) == 0)
		{
			debugReportExtensionFound = VK_TRUE;
			if (bValidation == TRUE)
			{
				
				enabledIntsanceExtensionNames_array[enabledIntanceExtensionCount++] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
			}
			else {
				//array will not have entry of VK_EXT_DEBUG_REPORT_EXTENSION_NAME
			}
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
	if (debugReportExtensionFound == VK_FALSE)
	{
		if (bValidation == TRUE)
		{
			vkResult = VK_ERROR_INITIALIZATION_FAILED; //return hardcoded failuare
			fprintf(gpFile, "\nIn fillInstanceExtensionNames(),validation is on VK_EXT_DEBUG_REPORT_EXTENSION_NAME is not supported");
			return vkResult;
		}
		else {
			fprintf(gpFile, "\nIn fillInstanceExtensionNames(),validation is off  VK_EXT_DEBUG_REPORT_EXTENSION_NAME is not supported");
		}
		
	}
	else {
		if (bValidation == TRUE)
		{
			fprintf(gpFile, "\nIn fillInstanceExtensionNames(),validation is on and VK_EXT_DEBUG_REPORT_EXTENSION_NAME is supported");
		}
		else {
			fprintf(gpFile, "\nIn fillInstanceExtensionNames(),validation is off  VK_EXT_DEBUG_REPORT_EXTENSION_NAME is  supported");
		}
	}

	// step 8 - Print only supported extension names
	for (uint32_t i = 0; i < enabledIntanceExtensionCount; i++)
	{
		fprintf(gpFile, "\nIn fillInstanceExtensionNames(),supported extension names = %s\n", enabledIntsanceExtensionNames_array[i]);

	}


	return vkResult;
}

VkResult fillValidationLayerNames(void)
{
	//code
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	uint32_t validationLayerCount = 0;
	vkResult = vkEnumerateInstanceLayerProperties(
		&validationLayerCount/*count aanel*/,
		NULL/*instance extension chi null karan property cha count mahiti nahi to second parameter mhadhun milnar*/);

	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\n");
		fprintf(gpFile, " fillValidationLayerNames() - vkEnumerateInstanceLayerProperties function failed");
		fprintf(gpFile, "Fist call to vkEnumerateInstanceLayerProperties failed");
	}
	else
	{
		fprintf(gpFile, "\fillValidationLayerNames() - vkEnumerateInstanceLayerProperties function Successed");
		fprintf(gpFile, "\nFirst call to vkEnumerateInstanceLayerProperties Successed");
	}

	VkLayerProperties* vkLayerProperties_array = NULL;

	vkLayerProperties_array = (VkLayerProperties*)malloc(sizeof(VkLayerProperties) * validationLayerCount);
	vkResult = vkEnumerateInstanceLayerProperties(
		&validationLayerCount,
		vkLayerProperties_array/*ethe ha array barala*/);

	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\n");
		fprintf(gpFile, " fillValidationLayerNames() - vkEnumerateInstanceLayerProperties function failed\n");
		fprintf(gpFile, "Second call to vkEnumerateInstanceLayerProperties failed\n");
	}
	else
	{
		fprintf(gpFile, "fillValidationLayerNames() - vkEnumerateInstanceLayerProperties function Successed\n");
		fprintf(gpFile, "Second call to vkEnumerateInstanceLayerProperties Successed\n");
	}

	char** validationLayersNames_array = NULL;
	validationLayersNames_array = (char**)malloc(sizeof(char*) * validationLayerCount);
	for (uint32_t i = 0; i < validationLayerCount; i++)
	{
		//atta je pn nav(string) rahil tevdhi memory dyaychi
		validationLayersNames_array[i] = (char*)malloc(sizeof(char) * strlen(vkLayerProperties_array[i].layerName) + 1); // +1 means array for the null terminator (\0) 
		memcpy(validationLayersNames_array[i], vkLayerProperties_array[i].layerName, strlen(vkLayerProperties_array[i].layerName) + 1);
		fprintf(gpFile, "fillValidationLayersNames() - Vulkan instance Layer Name = %s\n", validationLayersNames_array[i]);

	}

	free(vkLayerProperties_array);
	vkLayerProperties_array = NULL;

	VkBool32 validationLayerFound = VK_FALSE;
	for (uint32_t i = 0; i < validationLayerCount; i++)
	{
		if (strcmp(validationLayersNames_array[i], "VK_LAYER_KHRONOS_validation") == 0)
		{
			validationLayerFound = VK_TRUE;
			enabledValidationLayersNames_array[enabledValidationLayerCount++] = "VK_LAYER_KHRONOS_validation";
		}
	}

	for (uint32_t i = 0; i < validationLayerCount; i++)
	{
		free(validationLayersNames_array[i]);
	}
	free(validationLayersNames_array);

	if (validationLayerFound == VK_FALSE)
	{
		vkResult = VK_ERROR_INITIALIZATION_FAILED; //return hardcoded failuare
		fprintf(gpFile, "\nIn fillValidationLayersNames(),VK_LAYER_KHRONOS_validation not found");
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nIn fillValidationLayersNames(),VK_LAYER_KHRONOS_validation found");

	}

	//print layer name
	for (uint32_t i = 0; i < enabledValidationLayerCount; i++)
	{
		fprintf(gpFile, "\nIn fillValidationLayersNames(),supported layer names = %s\n", enabledValidationLayersNames_array[i]);

	}
	return vkResult;
}

VkResult createValidationCallback(void) {
	//functional declaration
	VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(VkDebugReportFlagsEXT,VkDebugReportObjectTypeEXT,uint64_t,size_t,int32_t,
														const char*, const char* ,void*); //call back function
	

	//variable declarations
	VkResult vkResult = VK_SUCCESS;
	PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT_fnptr = NULL;

	//code
	//get the required function pointer
	vkCreateDebugReportCallbackEXT_fnptr = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(vkInstance, "vkCreateDebugReportCallbackEXT");
	if (vkCreateDebugReportCallbackEXT_fnptr == NULL)
	{
		vkResult = VK_ERROR_INITIALIZATION_FAILED; //return hardcoded failuare
		fprintf(gpFile, "In createValidationCallback(),VkGetInstanceProcAddr failed to get function pointer for vkCreateDebugReportCallbackEXT_fnptr\n");
		return vkResult;
	}
	else {
		fprintf(gpFile, "In createValidationCallback(),VkGetInstanceProcAddr successed to get function pointer for vkCreateDebugReportCallbackEXT_fnptr\n");

	}

	//destroy the required function pointer
	vkDestroyDebugReportCallbackEXT_fnptr = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(vkInstance, "vkDestroyDebugReportCallbackEXT");
	if (vkDestroyDebugReportCallbackEXT_fnptr == NULL)
	{
		vkResult = VK_ERROR_INITIALIZATION_FAILED; //return hardcoded failuare
		fprintf(gpFile, "In createValidationCallback(),VkGetInstanceProcAddr failed to get function pointer for vkDestroyDebugReportCallbackEXT_fnptr\n");
		return vkResult;
	}
	else {
		fprintf(gpFile, "In createValidationCallback(),VkGetInstanceProcAddr successed to get function pointer for vkDestroyDebugReportCallbackEXT_fnptr\n");

	}

	//get the vulkan report callback object
	VkDebugReportCallbackCreateInfoEXT vkDebugReportCallbackCreateInfoEXT;
	memset((void*)&vkDebugReportCallbackCreateInfoEXT,0,sizeof(VkDebugReportCallbackCreateInfoEXT));
	vkDebugReportCallbackCreateInfoEXT.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	vkDebugReportCallbackCreateInfoEXT.pNext = NULL;
	vkDebugReportCallbackCreateInfoEXT.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	vkDebugReportCallbackCreateInfoEXT.pfnCallback = debugReportCallback;
	vkDebugReportCallbackCreateInfoEXT.pUserData = NULL;

	vkResult = vkCreateDebugReportCallbackEXT_fnptr(vkInstance , &vkDebugReportCallbackCreateInfoEXT,NULL,&vkDebugReportCallbackEXT);
	if (vkResult != VK_SUCCESS)
	{
		vkResult = VK_ERROR_INITIALIZATION_FAILED; //return hardcoded failuare
		fprintf(gpFile, "In createValidationCallback(),vkCreateDebugReportCallbackEXT_fnptr failed \n");
		return vkResult;
	}
	else {
		fprintf(gpFile, "In createValidationCallback(),vkCreateDebugReportCallbackEXT_fnptr successed\n");

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
		vkFormate_color = VK_FORMAT_R8G8B8A8_UNORM;
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
		vkExtent2D_swapchain.width = glm::max(vkSurfaceCapabilitiesKHR.minImageExtent.width, glm::min(vkSurfaceCapabilitiesKHR.maxImageExtent.width, vkExtent2D.width));
		vkExtent2D_swapchain.height = glm::max(vkSurfaceCapabilitiesKHR.minImageExtent.height, glm::min(vkSurfaceCapabilitiesKHR.maxImageExtent.height, vkExtent2D.height));
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

VkResult createCommandBuffers(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//code
	//initializationof vkcommandBufferallocateInfo
	VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo;
	memset((void*)&vkCommandBufferAllocateInfo, 0, sizeof(VkCommandBufferAllocateInfo));

	vkCommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	vkCommandBufferAllocateInfo.pNext = NULL;
	vkCommandBufferAllocateInfo.commandPool = vkCommandPool;
	vkCommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	vkCommandBufferAllocateInfo.commandBufferCount = 1;

	vkCommandBuffer_array = (VkCommandBuffer*)malloc(sizeof(VkCommandBuffer) * swapchainImageCount);

	//allocate command buffers
	for (uint32_t i = 0; i < swapchainImageCount; i++)
	{
		vkResult = vkAllocateCommandBuffers(vkDevice, &vkCommandBufferAllocateInfo,&vkCommandBuffer_array[i]);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "createCommandBuffers() - vkAllocateCommandBuffers function failed for iteration %d .(%d)\n", i, vkResult);
		}
		else
		{
			fprintf(gpFile, "createCommandBuffers() - vkAllocateCommandBuffers function Successed for iteration %d\n", i);
		}
	}

	return vkResult;
}

VkResult createVertexBuffer(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//code
	float triangle_position[] =
	{ 0.0f,1.0f,0.0f,
	-1.0f,-1.0f,0.0f,
	 1.0f,-1.0f,0.0f
	};

	memset((void*)&vertexData_position,0,sizeof(VertexData));

	VkBufferCreateInfo vkBufferCreateInfo;
	memset((void*)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));
	vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vkBufferCreateInfo.pNext = NULL;
	vkBufferCreateInfo.flags = 0; //valid flags use in sparc buffer
	vkBufferCreateInfo.size = sizeof(triangle_position);
	vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

	vkResult = vkCreateBuffer(vkDevice,&vkBufferCreateInfo,NULL,&vertexData_position.vkBuffer);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createVertexBuffer() - vkCreateBuffer function failed  %d \n",vkResult);
	}
	else
	{
		fprintf(gpFile, "createVertexBuffer() - vkCreateBuffer function Successed  \n");
	}

	VkMemoryRequirements vkMemoryRequirements;
	memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));

	vkGetBufferMemoryRequirements(vkDevice, vertexData_position.vkBuffer, &vkMemoryRequirements);

	VkMemoryAllocateInfo vkMemoryAllocateInfo;
	memset((void*)&vkMemoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));
	vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	vkMemoryAllocateInfo.pNext = NULL;
	vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
	vkMemoryAllocateInfo.memoryTypeIndex = 0; // initial value before entrying in the loop

	for (uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if ((vkMemoryRequirements.memoryTypeBits & 1) == 1)
		{
			if (vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
			{
				vkMemoryAllocateInfo.memoryTypeIndex = i;
				break;
			}
		}
		vkMemoryRequirements.memoryTypeBits >>= 1;
	}
	vkResult = vkAllocateMemory(vkDevice,&vkMemoryAllocateInfo,NULL,&vertexData_position.vkDeviceMemory);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createVertexBuffer() - vkAllocateMemory function failed  %d \n", vkResult);
	}
	else
	{
		fprintf(gpFile, "createVertexBuffer() - vkAllocateMemory function Successed  \n");
	}

	vkResult = vkBindBufferMemory(vkDevice, vertexData_position.vkBuffer, vertexData_position.vkDeviceMemory,0);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createVertexBuffer() - vkBindBufferMemory function failed  %d \n", vkResult);
	}
	else
	{
		fprintf(gpFile, "createVertexBuffer() - vkBindBufferMemory function Successed  \n");
	}

	void* data = NULL;
	vkResult = vkMapMemory(vkDevice, vertexData_position.vkDeviceMemory,0, vkMemoryAllocateInfo.allocationSize,0,&data);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createVertexBuffer() - vkMapMemory function failed  %d \n", vkResult);
	}
	else
	{
		fprintf(gpFile, "createVertexBuffer() - vkMapMemory function Successed  \n");
	}

	//actual memory mapped IO
	memcpy(data, triangle_position,sizeof(triangle_position));

	//
	vkUnmapMemory(vkDevice, vertexData_position.vkDeviceMemory);

	return vkResult;
}
VkResult createUniformBuffer(void)
{
	//function declarations
	VkResult updateUniformBuffer(void);

	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//code
	VkBufferCreateInfo vkBufferCreateInfo;
	memset((void*)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));
	vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vkBufferCreateInfo.pNext = NULL;
	vkBufferCreateInfo.flags = 0; //valid flags use in sparc buffer
	vkBufferCreateInfo.size = sizeof(MyUniformData);
	vkBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

	memset((void*)&uniformData, 0, sizeof(UniformData));

	vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &uniformData.vkBuffer);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createUniformBuffer() - vkCreateBuffer function failed  %d \n", vkResult);
	}
	else
	{
		fprintf(gpFile, "createUniformBuffer() - vkCreateBuffer function Successed  \n");
	}

	VkMemoryRequirements vkMemoryRequirements;
	memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));

	vkGetBufferMemoryRequirements(vkDevice, uniformData.vkBuffer, &vkMemoryRequirements);

	VkMemoryAllocateInfo vkMemoryAllocateInfo;
	memset((void*)&vkMemoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));
	vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	vkMemoryAllocateInfo.pNext = NULL;
	vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
	vkMemoryAllocateInfo.memoryTypeIndex = 0; // initial value before entrying in the loop

	for (uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if ((vkMemoryRequirements.memoryTypeBits & 1) == 1)
		{
			if (vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
			{
				vkMemoryAllocateInfo.memoryTypeIndex = i;
				break;
			}
		}
		vkMemoryRequirements.memoryTypeBits >>= 1;
	}
	vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &uniformData.vkDeviceMemory);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createUniformBuffer() - vkAllocateMemory function failed  %d \n", vkResult);
	}
	else
	{
		fprintf(gpFile, "createUniformBuffer() - vkAllocateMemory function Successed  \n");
	}

	vkResult = vkBindBufferMemory(vkDevice, uniformData.vkBuffer, uniformData.vkDeviceMemory, 0);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createUniformBuffer() - vkBindBufferMemory function failed  %d \n", vkResult);
	}
	else
	{
		fprintf(gpFile, "createUniformBuffer() - vkBindBufferMemory function Successed  \n");
	}

	//call updateUniform buffer
	vkResult = updateUniformBuffer();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createUniformBuffer() - updateUniformBuffer function failed  %d \n", vkResult);
	}
	else
	{
		fprintf(gpFile, "createUniformBuffer() - updateUniformBuffer function Successed  \n");
	}


	return vkResult;
}

VkResult updateUniformBuffer(void)
{  
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//code
	MyUniformData myUniformData;
	memset((void*)&myUniformData, 0, sizeof(MyUniformData));

	//update matrices
	myUniformData.modelMatrix = glm::mat4(1.0f);
	myUniformData.modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f));
	myUniformData.viewMatrix= glm::mat4(1.0f);
	glm::mat4 perspectiveProjectionMatrix = glm::mat4(1.0f);
	
	perspectiveProjectionMatrix = glm::perspective(glm::radians(45.0f), float(winWidth) / float(winHeight), 1.0f, 100.0f);
	myUniformData.projectionMatrix = perspectiveProjectionMatrix;

	//map uniform buffer
	void* data = NULL;
	vkResult = vkMapMemory(vkDevice, uniformData.vkDeviceMemory, 0, sizeof(MyUniformData), 0, &data);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "updateUniformBuffer() - vkMapMemory function failed  %d \n", vkResult);
	}

	//copy data to map local
	memcpy(data, &myUniformData, sizeof(MyUniformData));

	//unmap memory
	vkUnmapMemory(vkDevice, uniformData.vkDeviceMemory);

	return vkResult;
}

VkResult createShaders(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//code
	//For vertex Shader
	const char* szFileName = "Shader.vert.spv";
	FILE* fp = NULL;
	size_t size;

	fp = fopen(szFileName,"rb"); //"rb" = read binary format
	if (fp == NULL)
	{
		fprintf(gpFile, "createShaders() - createShaders function failed to open vertex shader sprvi file  \n" );
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "createShaders() - createShaders function Successed to open vertex shader sprvi file  \n");
	}

	fseek(fp, 0L, SEEK_END); // file chya shevati ja
	size = ftell(fp); // file pointer aata jithe ahe ti jaga atta mala sang
	if (size == 0)
	{
		fprintf(gpFile, "createShaders() -Vertex createShaders gave sprvi file size 0 \n");
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}	

	fseek(fp, 0L, SEEK_SET); // reset kar starting la

	char* shaderData = (char*)malloc(sizeof(char) * size);
	size_t retVal = fread(shaderData, size, 1, fp);
	if (retVal != 1)
	{
		fprintf(gpFile, "createShaders() - createShaders function failed to read vertex shader sprvi file  \n");
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "createShaders() - vkAllocateMemory function Successed to read vertex shader sprvi file  \n");
	}
	fclose(fp);

	VkShaderModuleCreateInfo vkShaderModuleCreateInfo;
	memset((void*)&vkShaderModuleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));
	vkShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	vkShaderModuleCreateInfo.pNext = NULL;
	vkShaderModuleCreateInfo.flags = 0; // resereved for future use 
	vkShaderModuleCreateInfo.codeSize = size;
	vkShaderModuleCreateInfo.pCode = (uint32_t*)shaderData;

	vkResult = vkCreateShaderModule(vkDevice,&vkShaderModuleCreateInfo,NULL,&vkShaderModule_vertex_shader);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createShaders() - vkCreateShaderModule function failed for vertex shader %d \n", vkResult);
	}
	else
	{
		fprintf(gpFile, "createShaders() - vkCreateShaderModule function Successed for vertex shader \n");
	}

	if (shaderData)
	{
		free(shaderData);
		shaderData = NULL;
	}
	fprintf(gpFile, "createShaders() - Vertex shader module Successfully created  \n");

	//For Fragment Shader
	szFileName = "Shader.frag.spv";
	fp = NULL;
	size = 0;

	fp = fopen(szFileName, "rb"); //"rb" = read binary format
	if (fp == NULL)
	{
		fprintf(gpFile, "createShaders() - createShaders function failed to open fragment shader sprvi file  \n");
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "createShaders() - vkAllocateMemory function Successed to open fragment shader sprvi file  \n");
	}

	fseek(fp, 0L, SEEK_END); // file chya shevati ja
	size = ftell(fp); // file pointer aata jithe ahe ti jaga atta mala sang
	if (size == 0)
	{
		fprintf(gpFile, "createShaders() - fragment createShaders gave sprvi file size 0 \n");
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}

	fseek(fp, 0L, SEEK_SET); // reset kar starting la

	 shaderData = (char*)malloc(sizeof(char) * size);
	 retVal = fread(shaderData, size, 1, fp);
	if (retVal != 1)
	{
		fprintf(gpFile, "createShaders() - createShaders function failed to read fragment shader sprvi file  \n");
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "createShaders() - vkAllocateMemory function Successed to read fragment shader sprvi file  \n");
	}
	fclose(fp);

	memset((void*)&vkShaderModuleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));
	vkShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	vkShaderModuleCreateInfo.pNext = NULL;
	vkShaderModuleCreateInfo.flags = 0; // resereved for future use 
	vkShaderModuleCreateInfo.codeSize = size;
	vkShaderModuleCreateInfo.pCode = (uint32_t*)shaderData;

	vkResult = vkCreateShaderModule(vkDevice, &vkShaderModuleCreateInfo, NULL, &vkShaderModule_fragment_shader);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createShaders() - vkCreateShaderModule function failed for fragment shader %d \n", vkResult);
	}
	else
	{
		fprintf(gpFile, "createShaders() - vkCreateShaderModule function Successed for fragment shader \n");
	}

	if (shaderData)
	{
		free(shaderData);
		shaderData = NULL;
	}
	fprintf(gpFile, "createShaders() - fragment shader module Successfully created  \n");

	return vkResult;
}

VkResult createDescriptorSetLayout(void)
{
	// variable declarations
	VkResult vkResult = VK_SUCCESS;

	//Initialize descriptor set binding
	VkDescriptorSetLayoutBinding vkDescriptorSetLayoutBinding;
	memset((void*)&vkDescriptorSetLayoutBinding, 0, sizeof(VkDescriptorSetLayoutBinding));
	vkDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	vkDescriptorSetLayoutBinding.binding = 0; //this is related to 0 binding in vertex shader 
	vkDescriptorSetLayoutBinding.descriptorCount = 1;
	vkDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	vkDescriptorSetLayoutBinding.pImmutableSamplers = NULL;


	VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutCreateInfo;
	memset((void*)&vkDescriptorSetLayoutCreateInfo,0,sizeof(VkDescriptorSetLayoutCreateInfo));
	vkDescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	vkDescriptorSetLayoutCreateInfo.pNext = NULL;
	vkDescriptorSetLayoutCreateInfo.flags = 0;
	vkDescriptorSetLayoutCreateInfo.bindingCount = 1;
	vkDescriptorSetLayoutCreateInfo.pBindings = &vkDescriptorSetLayoutBinding; //pBindings is actual array of VkDescriptorSetLayoutBinding

	vkResult = vkCreateDescriptorSetLayout(vkDevice, &vkDescriptorSetLayoutCreateInfo, NULL, &vkDescriptorSetLayout);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createDescriptorSetLayout() - vkCreateDescriptorSetLayout function failed  %d\n", vkResult);
	}
	else
	{
		fprintf(gpFile, "createDescriptorSetLayout() - vkCreateDescriptorSetLayout function Successed \n");
	}
	return vkResult;
}

VkResult createPipelineLayout(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//code
	VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo;
	memset((void*)&vkPipelineLayoutCreateInfo, 0, sizeof(VkPipelineLayoutCreateInfo));
	vkPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	vkPipelineLayoutCreateInfo.pNext = NULL;
	vkPipelineLayoutCreateInfo.flags = 0;
	vkPipelineLayoutCreateInfo.setLayoutCount = 1;
	vkPipelineLayoutCreateInfo.pSetLayouts = &vkDescriptorSetLayout;
	vkPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	vkPipelineLayoutCreateInfo.pPushConstantRanges = NULL;

	vkResult = vkCreatePipelineLayout(vkDevice, &vkPipelineLayoutCreateInfo, NULL, &vkPipelineLayout);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createPipelineLayout() - vkCreatePipelineLayout function failed  %d\n", vkResult);
	}
	else
	{
		fprintf(gpFile, "createPipelineLayout() - vkCreatePipelineLayout function Successed \n");
	}

	return vkResult;
}

VkResult createDescriptorPool(void) {
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//code
	//before creating actual descriptor pool , vulkan expects descriptor pool size
	VkDescriptorPoolSize  vkDescriptorPoolSize;
	memset((void*)&vkDescriptorPoolSize, 0, sizeof(VkDescriptorPoolSize));
	vkDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	vkDescriptorPoolSize.descriptorCount = 1;

	//create the pool
	VkDescriptorPoolCreateInfo vkDescriptorPoolCreateInfo;
	memset((void*)&vkDescriptorPoolCreateInfo, 0, sizeof(VkDescriptorPoolCreateInfo));
	vkDescriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	vkDescriptorPoolCreateInfo.pNext = NULL;
	vkDescriptorPoolCreateInfo.flags = 0;
	vkDescriptorPoolCreateInfo.poolSizeCount = 1;
	vkDescriptorPoolCreateInfo.pPoolSizes = &vkDescriptorPoolSize;
	vkDescriptorPoolCreateInfo.maxSets = 1;

	vkResult = vkCreateDescriptorPool(vkDevice, &vkDescriptorPoolCreateInfo, NULL, &vkDescriptorPool);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createDescriptorPool() - vkCreateDescriptorPool function failed  %d\n", vkResult);
	}
	else
	{
		fprintf(gpFile, "createDescriptorPool() - vkCreateDescriptorPool function Successed \n");
	}
	return vkResult;
	
}

VkResult createDescriptorSet(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//Code
	//initialize descriptor set allocation 
	VkDescriptorSetAllocateInfo vkDescriptorSetAllocateInfo;
	memset((void*)&vkDescriptorSetAllocateInfo, 0, sizeof(VkDescriptorSetAllocateInfo));
	vkDescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	vkDescriptorSetAllocateInfo.pNext = NULL;
	vkDescriptorSetAllocateInfo.descriptorPool = vkDescriptorPool;
	vkDescriptorSetAllocateInfo.descriptorSetCount = 1;
	vkDescriptorSetAllocateInfo.pSetLayouts = &vkDescriptorSetLayout;

	vkResult = vkAllocateDescriptorSets(vkDevice,&vkDescriptorSetAllocateInfo,&vkDescriptorSet);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createDescriptorSet() - vkAllocateDescriptorSets function failed  %d\n", vkResult);
	}
	else
	{
		fprintf(gpFile, "createDescriptorSet() - vkAllocateDescriptorSets function Successed \n");
	}

	//describe we want buffer as a uniform or image as a uniform
	VkDescriptorBufferInfo vkDescriptorBufferInfo;
	memset((void*)&vkDescriptorBufferInfo, 0, sizeof(VkDescriptorBufferInfo));
	vkDescriptorBufferInfo.buffer = uniformData.vkBuffer;
	vkDescriptorBufferInfo.offset = 0;
	vkDescriptorBufferInfo.range = sizeof(MyUniformData);

	//now update the above descriptor set directly to the shader
	//there are two way 1.writing to the shader 2. copy form one shader to another
	//we will perfer writing to shader , this requires following struture

	VkWriteDescriptorSet vkWriteDescriptorSet;
	memset((void*)&vkWriteDescriptorSet, 0, sizeof(VkWriteDescriptorSet));
	vkWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	vkWriteDescriptorSet.dstSet = vkDescriptorSet;
	vkWriteDescriptorSet.dstArrayElement = 0;
	vkWriteDescriptorSet.descriptorCount = 1;
	vkWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	vkWriteDescriptorSet.pBufferInfo = &vkDescriptorBufferInfo;
	vkWriteDescriptorSet.pImageInfo = NULL;
	vkWriteDescriptorSet.pTexelBufferView = NULL;
	vkWriteDescriptorSet.dstBinding = 0; // uniform is at binding at 0 index

	vkUpdateDescriptorSets(vkDevice,1,&vkWriteDescriptorSet,0,NULL);

	fprintf(gpFile, "createDescriptorSet() - vkUpdateDescriptorSets function Successed \n");

	return vkResult;
}

VkResult createRenderPass(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//code
	//initialization of VkAttachmentDescription

	VkAttachmentDescription  vkAttachmentDescription_array[1];
	memset((void*)vkAttachmentDescription_array, 0, sizeof(VkAttachmentDescription)*_ARRAYSIZE(vkAttachmentDescription_array));
	vkAttachmentDescription_array[0].flags = 0; //
	vkAttachmentDescription_array[0].format = vkFormate_color;
	vkAttachmentDescription_array[0].samples = VK_SAMPLE_COUNT_1_BIT; // image multisampling wali nahi ahe mahnun 1 bit is enough
	vkAttachmentDescription_array[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; //att alay alay hi layer clear karaychi
	vkAttachmentDescription_array[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	vkAttachmentDescription_array[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	vkAttachmentDescription_array[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	vkAttachmentDescription_array[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	vkAttachmentDescription_array[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference  vkAttachmentReference;
	memset((void*)&vkAttachmentReference, 0, sizeof(VkAttachmentReference));
	vkAttachmentReference.attachment = 0; // this is index number
	vkAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription vkSubpassDescription;
	memset((void*)&vkSubpassDescription, 0, sizeof(VkSubpassDescription));
	vkSubpassDescription.flags = 0;
	vkSubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	vkSubpassDescription.inputAttachmentCount = 0;
	vkSubpassDescription.pInputAttachments = NULL;
	vkSubpassDescription.colorAttachmentCount = _ARRAYSIZE(vkAttachmentDescription_array);
	vkSubpassDescription.pColorAttachments = &vkAttachmentReference; //base address
	vkSubpassDescription.pResolveAttachments = NULL;
	vkSubpassDescription.pDepthStencilAttachment = NULL;
	vkSubpassDescription.preserveAttachmentCount = 0;
	vkSubpassDescription.pPreserveAttachments = NULL;

	//declare and initialize VkRenderpasscreateinfo structure
	VkRenderPassCreateInfo vkRenderPassCreateInfo;
	memset((void*)&vkRenderPassCreateInfo, 0, sizeof(VkRenderPassCreateInfo));
	vkRenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	vkRenderPassCreateInfo.pNext = NULL;
	vkRenderPassCreateInfo.flags = 0;
	vkRenderPassCreateInfo.attachmentCount = _ARRAYSIZE(vkAttachmentDescription_array);
	vkRenderPassCreateInfo.pAttachments = vkAttachmentDescription_array;
	vkRenderPassCreateInfo.subpassCount = 1;
	vkRenderPassCreateInfo.pSubpasses = &vkSubpassDescription;
	vkRenderPassCreateInfo.dependencyCount = 0;
	vkRenderPassCreateInfo.pDependencies = NULL;

	//create renderpass
	vkResult = vkCreateRenderPass(vkDevice,&vkRenderPassCreateInfo,NULL,&vkRenderPass);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createRenderPass() - vkCreateRenderPass function failed %d \n", vkResult);
	}
	else
	{
		fprintf(gpFile, "createRenderPass() - vkCreateRenderPass function Successed \n");
	}

	return vkResult;
}

VkResult createPipeline(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//code
	//vertex input state
	VkVertexInputBindingDescription vkVertexInputBindingDescription_array[1];
	memset((void*)vkVertexInputBindingDescription_array, 0, sizeof(VkVertexInputBindingDescription) * _ARRAYSIZE(vkVertexInputBindingDescription_array));
	vkVertexInputBindingDescription_array[0].binding = 0;
	vkVertexInputBindingDescription_array[0].stride = sizeof(float)*3;
	vkVertexInputBindingDescription_array[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkVertexInputAttributeDescription vkVertexInputAttributeDescription_array[1];
	memset((void*)vkVertexInputAttributeDescription_array, 0, sizeof(VkVertexInputAttributeDescription) * _ARRAYSIZE(vkVertexInputAttributeDescription_array));
	vkVertexInputAttributeDescription_array[0].binding = 0;
	vkVertexInputAttributeDescription_array[0].location = 0;
	vkVertexInputAttributeDescription_array[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	vkVertexInputAttributeDescription_array[0].offset = 0;

	VkPipelineVertexInputStateCreateInfo vkPipelineVertexInputStateCreateInfo;
	memset((void*)&vkPipelineVertexInputStateCreateInfo, 0, sizeof(VkPipelineVertexInputStateCreateInfo));
	vkPipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vkPipelineVertexInputStateCreateInfo.pNext = NULL;
	vkPipelineVertexInputStateCreateInfo.flags = 0;
	vkPipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = _ARRAYSIZE(vkVertexInputBindingDescription_array);
	vkPipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = vkVertexInputBindingDescription_array;
	vkPipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = _ARRAYSIZE(vkVertexInputAttributeDescription_array);
	vkPipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vkVertexInputAttributeDescription_array;

	//Input assembly state
	VkPipelineInputAssemblyStateCreateInfo vkPipelineInputAssemblyStateCreateInfo;
	memset((void*)&vkPipelineInputAssemblyStateCreateInfo, 0, sizeof(VkPipelineInputAssemblyStateCreateInfo));
	vkPipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	vkPipelineInputAssemblyStateCreateInfo.pNext = NULL;
	vkPipelineInputAssemblyStateCreateInfo.flags = 0;
	vkPipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	//vkPipelineInputAssemblyStateCreateInfo.primitiveRestartEnable <-- by default 0

	//rastrizer state
	VkPipelineRasterizationStateCreateInfo vkPipelineRasterizationStateCreateInfo;
	memset((void*)&vkPipelineRasterizationStateCreateInfo, 0, sizeof(VkPipelineRasterizationStateCreateInfo));
	vkPipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	vkPipelineRasterizationStateCreateInfo.pNext = NULL;
	vkPipelineRasterizationStateCreateInfo.flags = 0;
	vkPipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	vkPipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	vkPipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	vkPipelineRasterizationStateCreateInfo.lineWidth = 1.0f;

	//color blend state
	VkPipelineColorBlendAttachmentState vkPipelineColorBlendAttachmentState_array[1];
	memset((void*)vkPipelineColorBlendAttachmentState_array, 0, sizeof(VkPipelineColorBlendAttachmentState)* _ARRAYSIZE(vkPipelineColorBlendAttachmentState_array));
	vkPipelineColorBlendAttachmentState_array[0].colorWriteMask = 0XF;
	vkPipelineColorBlendAttachmentState_array[0].blendEnable = VK_FALSE;
	
	VkPipelineColorBlendStateCreateInfo vkPipelineColorBlendStateCreateInfo;
	memset((void*)&vkPipelineColorBlendStateCreateInfo, 0, sizeof(VkPipelineColorBlendStateCreateInfo));
	vkPipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	vkPipelineColorBlendStateCreateInfo.pNext = NULL;
	vkPipelineColorBlendStateCreateInfo.flags = 0;
	vkPipelineColorBlendStateCreateInfo.attachmentCount = _ARRAYSIZE(vkPipelineColorBlendAttachmentState_array);
	vkPipelineColorBlendStateCreateInfo.pAttachments = vkPipelineColorBlendAttachmentState_array;

	//viewport scissor state
	VkPipelineViewportStateCreateInfo vkPipelineViewportStateCreateInfo;
	memset((void*)&vkPipelineViewportStateCreateInfo, 0, sizeof(VkPipelineViewportStateCreateInfo));
	vkPipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	vkPipelineViewportStateCreateInfo.pNext = NULL;
	vkPipelineViewportStateCreateInfo.flags = 0;
	vkPipelineViewportStateCreateInfo.viewportCount = 1;  // we can give array also

	memset((void*)&vkViewport, 0, sizeof(VkViewport));
	vkViewport.x = 0;
	vkViewport.y = 0;
	vkViewport.width = (float)vkExtent2D_swapchain.width;
	vkViewport.height = (float)vkExtent2D_swapchain.height;
	vkViewport.minDepth = 0.0f;
	vkViewport.maxDepth = 1.0f;

	vkPipelineViewportStateCreateInfo.pViewports = &vkViewport;
	vkPipelineViewportStateCreateInfo.scissorCount = 1;

	memset((void*)&vkRect2D_scissor, 0, sizeof(VkRect2D));
	vkRect2D_scissor.offset.x = 0;
	vkRect2D_scissor.offset.y = 0;
	vkRect2D_scissor.extent.width = (float)vkExtent2D_swapchain.width;
	vkRect2D_scissor.extent.height = (float)vkExtent2D_swapchain.height;

	vkPipelineViewportStateCreateInfo.pScissors = &vkRect2D_scissor; // viewport count and  scissor count member of this strutcure same

	//depth stencil state
	//as we dont have depth yet we can omit this state

	//dynamic state 
	//viewport ,depth bias ,line width,blend constants , stensil mask etc.. he sarv apan dyanamic state karun karu shakto
	//now we dont have any dynamic state

	//multisample state
	VkPipelineMultisampleStateCreateInfo  vkPipelineMultisampleStateCreateInfo;
	memset((void*)&vkPipelineMultisampleStateCreateInfo, 0, sizeof(VkPipelineMultisampleStateCreateInfo));
	vkPipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	vkPipelineMultisampleStateCreateInfo.pNext = NULL;
	vkPipelineMultisampleStateCreateInfo.flags = 0;
	vkPipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; //ha specify nahi kela tr validation error yeto

	//shader state
	
	VkPipelineShaderStageCreateInfo vkPipelineShaderStageCreateInfo_array[2];
	memset((void*)vkPipelineShaderStageCreateInfo_array, 0, sizeof(VkPipelineShaderStageCreateInfo)* _ARRAYSIZE(vkPipelineShaderStageCreateInfo_array));
	//vertex shader
	vkPipelineShaderStageCreateInfo_array[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vkPipelineShaderStageCreateInfo_array[0].pNext = NULL;
	vkPipelineShaderStageCreateInfo_array[0].flags = 0;
	vkPipelineShaderStageCreateInfo_array[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	vkPipelineShaderStageCreateInfo_array[0].module = vkShaderModule_vertex_shader;
	vkPipelineShaderStageCreateInfo_array[0].pName = "main";
	vkPipelineShaderStageCreateInfo_array[0].pSpecializationInfo = NULL;

	//fragment shader	
	vkPipelineShaderStageCreateInfo_array[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vkPipelineShaderStageCreateInfo_array[1].pNext = NULL;
	vkPipelineShaderStageCreateInfo_array[1].flags = 0;
	vkPipelineShaderStageCreateInfo_array[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	vkPipelineShaderStageCreateInfo_array[1].module = vkShaderModule_fragment_shader;
	vkPipelineShaderStageCreateInfo_array[1].pName = "main";
	vkPipelineShaderStageCreateInfo_array[1].pSpecializationInfo = NULL;

	//tesellation state
	// we dont have tessellation shader , so we can omit this state

	//as pipeline created from pipeline cache , now we will create pipeline cache
	VkPipelineCacheCreateInfo vkPipelineCacheCreateInfo;
	memset((void*)&vkPipelineCacheCreateInfo, 0, sizeof(VkPipelineCacheCreateInfo));
	vkPipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	vkPipelineCacheCreateInfo.pNext = NULL;
	vkPipelineCacheCreateInfo.flags = 0;

	VkPipelineCache vkPipelineCache = VK_NULL_HANDLE;
	vkResult = vkCreatePipelineCache(vkDevice, &vkPipelineCacheCreateInfo, NULL, &vkPipelineCache);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\n In createPipeline() ,vkCreatePipelineCache function failed");
	}
	else
	{
		fprintf(gpFile, "\n In createPipeline(), vkCreatePipelineCache function Successed");
	}

	//create the actual graphics pipeline
	VkGraphicsPipelineCreateInfo vkGraphicsPipelineCreateInfo;
	memset((void*)&vkGraphicsPipelineCreateInfo, 0, sizeof(VkGraphicsPipelineCreateInfo));
	vkGraphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	vkGraphicsPipelineCreateInfo.pNext = NULL;
	vkGraphicsPipelineCreateInfo.flags = 0;
	vkGraphicsPipelineCreateInfo.pVertexInputState = &vkPipelineVertexInputStateCreateInfo;
	vkGraphicsPipelineCreateInfo.pInputAssemblyState = &vkPipelineInputAssemblyStateCreateInfo;
	vkGraphicsPipelineCreateInfo.pRasterizationState = &vkPipelineRasterizationStateCreateInfo;
	vkGraphicsPipelineCreateInfo.pColorBlendState = &vkPipelineColorBlendStateCreateInfo;
	vkGraphicsPipelineCreateInfo.pViewportState = &vkPipelineViewportStateCreateInfo;
	vkGraphicsPipelineCreateInfo.pDepthStencilState = NULL;
	vkGraphicsPipelineCreateInfo.pDynamicState = NULL;
	vkGraphicsPipelineCreateInfo.pMultisampleState = &vkPipelineMultisampleStateCreateInfo;
	vkGraphicsPipelineCreateInfo.stageCount = _ARRAYSIZE(vkPipelineShaderStageCreateInfo_array);
	vkGraphicsPipelineCreateInfo.pStages = vkPipelineShaderStageCreateInfo_array;
	vkGraphicsPipelineCreateInfo.pTessellationState = NULL;
	vkGraphicsPipelineCreateInfo.layout = vkPipelineLayout;
	vkGraphicsPipelineCreateInfo.renderPass = vkRenderPass;
	vkGraphicsPipelineCreateInfo.subpass = 0;
	vkGraphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	vkGraphicsPipelineCreateInfo.basePipelineIndex = 0;

	//now create the pipeline
	vkResult = vkCreateGraphicsPipelines(vkDevice, vkPipelineCache,1,&vkGraphicsPipelineCreateInfo,NULL,&vkPipeline);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\n In createPipeline() ,vkCreateGraphicsPipelines function failed");
	}
	else
	{
		fprintf(gpFile, "\n In createPipeline(), vkCreateGraphicsPipelines function Successed");
	}

	//we are done with pipeline cache so destroy it
	if (vkPipelineCache) {
		vkDestroyPipelineCache(vkDevice, vkPipelineCache, NULL);
		vkPipelineCache = VK_NULL_HANDLE;
	}

	return vkResult;


}

VkResult createFramebuffers(void)
{
	// variable declarations
	VkResult vkResult = VK_SUCCESS;

	//code
	//step 1- declare an array 
	VkImageView vkImageView_attachments_array[1];
	memset((void*)vkImageView_attachments_array, 0, sizeof(VkImageView) * _ARRAYSIZE(vkImageView_attachments_array));

	//step 2 - declare and initialize VkFramebufferCreateInfo structure
	VkFramebufferCreateInfo vkFramebufferCreateInfo;
	memset((void*)&vkFramebufferCreateInfo, 0, sizeof(VkFramebufferCreateInfo));
	vkFramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	vkFramebufferCreateInfo.pNext = NULL;
	vkFramebufferCreateInfo.flags = 0;
	vkFramebufferCreateInfo.renderPass = vkRenderPass;
	vkFramebufferCreateInfo.attachmentCount = _ARRAYSIZE(vkImageView_attachments_array);
	vkFramebufferCreateInfo.pAttachments = vkImageView_attachments_array;
	vkFramebufferCreateInfo.width = vkExtent2D_swapchain.width;
	vkFramebufferCreateInfo.height = vkExtent2D_swapchain.height;
	vkFramebufferCreateInfo.layers = 1;


	//step 3- 
	vkFramebuffer_array = (VkFramebuffer*)malloc(sizeof(VkFramebuffer) * swapchainImageCount);

	for (uint32_t i = 0; i < swapchainImageCount; i++)
	{
		vkImageView_attachments_array[0] = swapchainImageView_array[i];
		vkResult = vkCreateFramebuffer(vkDevice, &vkFramebufferCreateInfo, NULL, &vkFramebuffer_array[i]);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "createFramebuffers() - createFramebuffers function failed for iteration %d .(%d)\n", i, vkResult);
		}
		else
		{
			fprintf(gpFile, "createFramebuffers() - createFramebuffers function Successed for iteration %d\n", i);
		}
	}

	return vkResult;
}

VkResult createSemaPhores(void)
{
	// variable declarations
	VkResult vkResult = VK_SUCCESS;

	//code
	VkSemaphoreCreateInfo vkSemaphoreCreateInfo;
	memset((void*)&vkSemaphoreCreateInfo, 0, sizeof(VkSemaphoreCreateInfo));
	vkSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	vkSemaphoreCreateInfo.pNext = NULL;
	vkSemaphoreCreateInfo.flags = 0; //reserved must be 0

	vkResult = vkCreateSemaphore(vkDevice, &vkSemaphoreCreateInfo, NULL, &vkSemaphore_backBuffer);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createSemaPhores() - vkCreateSemaPhore function failed  %d\n",vkResult);
	}
	else
	{
		fprintf(gpFile, "createSemaPhores() - vkCreateSemaPhore function Successed \n");
	}


	vkResult = vkCreateSemaphore(vkDevice, &vkSemaphoreCreateInfo, NULL, &vkSemaphore_renderComplete);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "createSemaPhores() - 2 vkCreateSemaPhore function failed  %d\n", vkResult);
	}
	else
	{
		fprintf(gpFile, "createSemaPhores() - 2 vkCreateSemaPhore function Successed \n");
	}

	return vkResult;
}

VkResult createFences(void)
{
	// variable declarations
	VkResult vkResult = VK_SUCCESS;

	//code
	VkFenceCreateInfo vkFenceCreateInfo;
	memset((void*)&vkFenceCreateInfo, 0, sizeof(VkFenceCreateInfo));
	vkFenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	vkFenceCreateInfo.pNext = NULL;
	vkFenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	vkFence_array = (VkFence*)malloc(sizeof(VkFence) * swapchainImageCount);
	
	for (uint32_t i = 0; i < swapchainImageCount; i++)
	{
		vkResult = vkCreateFence(vkDevice, &vkFenceCreateInfo, NULL, &vkFence_array[i]);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "createFences() - vkCreateFence function failed for iteration %d .(%d)\n", i, vkResult);
		}
		else
		{
			fprintf(gpFile, "createFences() - vkCreateFence function Successed for iteration %d\n", i);
		}
	}
	return vkResult;
}

VkResult buildCommandBuffers(void)
{
	// variable declarations
	VkResult vkResult = VK_SUCCESS;

	//code
	//loop per swapChain Image

	for (uint32_t i = 0; i < swapchainImageCount; i++)
	{
		//reset commandBuffers
		vkResult = vkResetCommandBuffer(vkCommandBuffer_array[i],0); // 0 means dont release resource created by command pull for this command buffer 
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "buildCommandBuffers() - vkResetCommandBuffer function failed for iteration %d .(%d)\n", i, vkResult);
		}
		else
		{
			fprintf(gpFile, "buildCommandBuffers() - vkResetCommandBuffer function Successed for iteration %d\n", i);
		}

		VkCommandBufferBeginInfo  vkCommandBufferBeginInfo;
		memset((void*)&vkCommandBufferBeginInfo, 0, sizeof(VkCommandBufferBeginInfo));
		vkCommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		vkCommandBufferBeginInfo.pNext = NULL;
		vkCommandBufferBeginInfo.flags = 0;

		vkResult = vkBeginCommandBuffer(vkCommandBuffer_array[i], &vkCommandBufferBeginInfo);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "buildCommandBuffers() - vkBeginCommandBuffer function failed for iteration %d .(%d)\n", i, vkResult);
		}
		else
		{
			fprintf(gpFile, "buildCommandBuffers() - vkBeginCommandBuffer function Successed for iteration %d\n", i);
		}

		//set clear value
		VkClearValue vkClearValue_array[1];
		memset((void*)vkClearValue_array, 0, sizeof(VkClearValue)*_ARRAYSIZE(vkClearValue_array));
		vkClearValue_array[0].color = vkClearColorValue;

		VkRenderPassBeginInfo vkRenderPassBeginInfo;
		memset((void*)&vkRenderPassBeginInfo, 0, sizeof(VkRenderPassBeginInfo));
		vkRenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		vkRenderPassBeginInfo.pNext = NULL;
		vkRenderPassBeginInfo.renderPass = vkRenderPass;
		vkRenderPassBeginInfo.renderArea.offset.x = 0;
		vkRenderPassBeginInfo.renderArea.offset.y = 0;
		vkRenderPassBeginInfo.renderArea.extent.width = vkExtent2D_swapchain.width;
		vkRenderPassBeginInfo.renderArea.extent.height = vkExtent2D_swapchain.height;
		vkRenderPassBeginInfo.clearValueCount = _ARRAYSIZE(vkClearValue_array);
		vkRenderPassBeginInfo.pClearValues = vkClearValue_array;
		vkRenderPassBeginInfo.framebuffer = vkFramebuffer_array[i];

		vkCmdBeginRenderPass(vkCommandBuffer_array[i],&vkRenderPassBeginInfo,VK_SUBPASS_CONTENTS_INLINE);

		
		//Bind with the pipeline
		vkCmdBindPipeline(vkCommandBuffer_array[i],VK_PIPELINE_BIND_POINT_GRAPHICS,vkPipeline);

		//bind with descriptor set to pipeline
		vkCmdBindDescriptorSets(vkCommandBuffer_array[i],VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipelineLayout,0, 1,&vkDescriptorSet,0,NULL);


		//bind with the vertex buffer
		VkDeviceSize vkDeviceSize_offset_array[1];
		memset((void*)vkDeviceSize_offset_array, 0, sizeof(VkDeviceSize)*_ARRAYSIZE(vkDeviceSize_offset_array));

		vkCmdBindVertexBuffers(vkCommandBuffer_array[i],0,1,&vertexData_position.vkBuffer, vkDeviceSize_offset_array);
		
		//here we should call vulkan drawing function
		vkCmdDraw(vkCommandBuffer_array[i],3,1,0,0);

		//End render pass
		vkCmdEndRenderPass(vkCommandBuffer_array[i]);

		//End commandbuffer recording
		vkResult = vkEndCommandBuffer(vkCommandBuffer_array[i]);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "buildCommandBuffers() - vkEndCommandBuffer function failed for iteration %d .(%d)\n", i, vkResult);
		}
		else
		{
			fprintf(gpFile, "buildCommandBuffers() - vkEndCommandBuffer function Successed for iteration %d\n", i);
		}

	}

	return vkResult;
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(VkDebugReportFlagsEXT vkDebugReportFlagsEXT,
													VkDebugReportObjectTypeEXT vkDebugReportObjectTypeEXT,
													uint64_t object,
													size_t location,
													int32_t messageCode,
													const char* pLayerPrefix,
													const char* pMessage,
													void* pUserData)
{
	//code
	fprintf(gpFile, "\n AST Validation :debugReportCallback - %s (%d) = %s\n ", pLayerPrefix, messageCode, pMessage);

	return VK_FALSE;
}