#include <locale.h>

#include <windows.h>
#include <commctrl.h>

#include "Globals.h"
#include "Main_window.h"
#include "Resource.h"
#include "Logger.h"

/* Global instance handle */
HINSTANCE g_hInstance = NULL;
static HFONT guiFont = NULL;

BOOL CALLBACK EnumChildProc(HWND hWnd, LPARAM lParam)
{
	NONCLIENTMETRICS metrics = {0};
	metrics.cbSize = sizeof(metrics);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, metrics.cbSize, &metrics, 0);

	guiFont = CreateFontIndirect(&metrics.lfCaptionFont);
	SendMessage(hWnd, WM_SETFONT, (WPARAM) guiFont, MAKELPARAM(TRUE, 0));
	return TRUE;
}

/* Our application entry point */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	LOGGER_init("tvshowtracker.log");
	INITCOMMONCONTROLSEX icc;
	HWND hWnd;
	HACCEL hAccelerators;
	MSG msg;

	/* Assign global HINSTANCE */
	g_hInstance = hInstance;

	/* Initialise common controls */
	icc.dwSize = sizeof(icc);
	icc.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&icc);

	/* Register our main window class, or error */
	if (!RegisterMainWindowClass()) {
		MessageBox(NULL, TEXT("Error registering main window class."), TEXT("Error"), MB_ICONERROR | MB_OK);
		return 0;
	}

  /* Create our main window, or error */
	if (!(hWnd = CreateMainWindow())) {
		MessageBox(NULL, TEXT("Error creating main window."), TEXT("Error"), MB_ICONERROR | MB_OK);
		return 0;
	}

	/* Load accelerators */
	hAccelerators = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR));

	/* Show main window and force a paint */
	EnumChildWindows(hWnd, EnumChildProc, 0);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

  /* Main message loop */
	while (GetMessage(&msg, NULL, 0, 0) > 0) {
		if (!TranslateAccelerator(hWnd, hAccelerators, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	if (guiFont != NULL) {
		DeleteObject(guiFont);
	}
	return (int)msg.wParam;
}
