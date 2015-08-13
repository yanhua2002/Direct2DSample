#pragma once
#pragma comment(lib,"d2d1.lib")

#include "Resource.h"
#include <d2d1.h>
#include <vector>

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
	float getXScale() { return xScale; }
	float getYScale() { return yScale; }

	void AddPoint(float, float);
	void ClearPoints();

private:
	// Global Variables:
	HINSTANCE hInst;                                // current instance
	WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
	WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

	float xScale;
	float yScale;

	// Set up Direct2D
	ID2D1Factory* m_pD2DFactory;
	//ID2D1EllipseGeometry* m_pEllipseGeometry;
	ID2D1HwndRenderTarget* m_pRenderTarget;
	ID2D1SolidColorBrush* m_pBlackBrush;
	std::vector<std::pair<float, float>> points;
	HRESULT CreateDeviceIndependentResources();
	HRESULT CreateDeviceResources(HWND hwnd);
	HRESULT OnRender(HWND hwnd);
	void OnResize(UINT width, UINT height);
};

