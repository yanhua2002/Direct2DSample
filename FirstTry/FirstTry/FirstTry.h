#pragma once

#include "stdafx.h"
#include "resource.h"

#include <math.h>

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>

#include <UIAnimation.h>

#pragma comment(lib,"d2d1")
#pragma comment(lib,"dwrite")
#pragma comment(lib,"windowscodecs")

class FirstTry
{
public:
	FirstTry();
	~FirstTry();

	// Register the window class and call methods for instantiating drawing resources.
	HRESULT Initialize();
	// Process and dispatch messages.
	void RunMessageLoop();

	HRESULT Invalidate();

private:
	// Initialize device-independent resources.
	HRESULT CreateDeviceIndependentResources();

	// Initialize device-dependent resources.
	HRESULT CreateDeviceResources();

	// Release device-dependent resources.
	void DiscardDeviceResources();

	// Draw content.
	HRESULT OnPaint();
	HRESULT OnRender();

	// Resize the render target.
	void OnResize(UINT width, UINT height);

	// The windows procedure.
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	// Load resource bitmap
	HRESULT LoadResourceBitmap(
		ID2D1RenderTarget *pRenderTarget,
		IWICImagingFactory *pIWICFactory,
		PCWSTR resourceName,
		PCWSTR resourceType,
		UINT destinationWidth,
		UINT destinationHeight,
		ID2D1Bitmap **ppBitmap);

	// Load bitmap from file
	HRESULT LoadBitmapFromFile(
		ID2D1RenderTarget *pRenderTarget,
		IWICImagingFactory *pIWICFactory,
		PCWSTR uri,
		UINT destinationWidth,
		UINT destinationHeight,
		ID2D1Bitmap **ppBitmap);

	// Animation components
	HRESULT InitializeAnimation();
	HRESULT CreateAnimationVariables();
	HRESULT AcceleratingRotation();

private:
	HWND m_hwnd;
	DOUBLE currentAngle = 0;

	// D2D components
	ID2D1Factory* m_pD2DFactory;
	IWICImagingFactory *m_pWICFactory;
	IDWriteFactory *m_pDWriteFactory;
	ID2D1HwndRenderTarget* m_pRenderTarget;
	ID2D1SolidColorBrush* m_pLightSlateGrayBrush;
	ID2D1SolidColorBrush* m_pCornflowerBlueBrush;
	ID2D1Bitmap *m_pBitmap;
	ID2D1Bitmap *m_pBitmap1;
	IDWriteTextFormat *m_pTextFormat;

	// Animation components
	IUIAnimationManager *m_pAnimationManager;
	IUIAnimationTimer *m_pAnimationTimer;
	IUIAnimationTransitionLibrary *m_pTransitionLibrary;

	// Animated Variables
	IUIAnimationVariable *m_pAnimationVarAngle;
};