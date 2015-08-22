#pragma once

#ifndef WINVER
#define	WINVER	0x0700
#endif // !WINVER

#ifndef _WIN32_WINNT
#define	_WIN32_WINNT	0x0700
#endif // !_WIN32_WINNT

#ifndef UNICODE
#define	UNICODE
#endif // !UNICODE

#define	WIN32_LEAN_AND_MEAN

// Windows header files
#include <Windows.h>

// C runtime header files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <wchar.h>
#include <math.h>

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>

// Macros
template<class Interface>
inline void SafeRelease(Interface **ppInterfaceToRelease)
{
	if (*ppInterfaceToRelease != NULL)
	{
		(*ppInterfaceToRelease)->Release();
		(*ppInterfaceToRelease) = NULL;
	}
}

#ifndef Assert
#if defined(DEBUG)||defined(_DEBUG)
#define Assert(b) do {if(!(b)){OutputDebugStringA("Assert: " #b "\n");}}while(0)
#else
#define Assert(b)
#endif
#endif // !Assert

#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif // !HINST_THISCOMPONENT

// DemoApp
class DemoApp
{
public:
	DemoApp();
	~DemoApp();

	HRESULT Initialize();
	void RunMessageLoop();

private:
	HRESULT CreateDeviceIndependentResources();
	HRESULT CreateDeviceResources();

	HRESULT CreateGridPatternBrush(ID2D1RenderTarget *pRenderTarget, ID2D1BitmapBrush **ppBitmapBrush);

	void DiscardDeviceResources();

	HRESULT OnRender();
	void OnResize(UINT width, UINT height);

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	HRESULT LoadResourceBitmap(
		ID2D1RenderTarget *pRenderTarget,
		IWICImagingFactory *pIWICFactory,
		PCWSTR resourceName,
		PCWSTR resourceType,
		UINT destinationWidth,
		UINT destinationHeight,
		ID2D1Bitmap **ppBitmap);

	HRESULT LoadBitmapFromFile(
		ID2D1RenderTarget *pRenderTarget,
		IWICImagingFactory *pIWICFactory,
		PCWSTR uri,
		UINT destinationWidth,
		UINT destinationHeight,
		ID2D1Bitmap **ppBitmap);

private:
	HWND m_hwnd;
	ID2D1Factory *m_pD2DFactory;
	IWICImagingFactory *m_pWICFactory;
	IDWriteFactory *m_pDWriteFactory;
	ID2D1HwndRenderTarget *m_pRenderTarget;
	IDWriteTextFormat *m_pTextFormat;
	ID2D1PathGeometry *m_pPathGeometry;
	ID2D1LinearGradientBrush *m_pLinearGradientBrush;
	ID2D1SolidColorBrush *m_pBlackBrush;
	ID2D1BitmapBrush *m_pGridPatternBitmapBrush;
	ID2D1Bitmap *m_pBitmap;
	ID2D1Bitmap *m_pAnotherBitmap;
};