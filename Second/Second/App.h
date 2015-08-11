#pragma once

#include "Resource.h"
#include <d2d1.h>

#define MAX_LOADSTRING 100
class App
{
public:
	App();
	~App();
	ATOM		RegisterClass();
	BOOL		Init(HINSTANCE, int);
	BOOL		InitInstance(int);
	int			RunMessageLoop();
	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
	static INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

	HINSTANCE	getInstance() { return hInst; }

private:
	// Global Variables:
	HINSTANCE hInst;                                // current instance
	WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
	WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

	// Set up Direct2D
	ID2D1Factory* m_pD2DFactory;
	ID2D1EllipseGeometry* m_pEllipseGeometry;
	HRESULT CreateDeviceIndependentResources();
	HRESULT CreateDeviceResources(HWND hwnd);
};

