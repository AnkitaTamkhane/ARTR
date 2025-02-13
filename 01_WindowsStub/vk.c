// Header Files
#include <windows.h>
#include "vk.h"
#include<stdio.h> //for File IO functions
#include<stdlib.h> //for exit()

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

// Global function declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

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
	int inlitialize(void);
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
	iRateVal = inlitialize();

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

int inlitialize(void)
{
	//function declarations

	//variable declarations

	//code

	return(0);
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