#pragma once

#ifndef WINVER
#define WINVER 0x0700
#endif // !WINVER

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0700
#endif // !_WIN32_WINNT

#ifndef UNICODE
#define UNICODE
#endif // !UNICODE

#define WIN32_LEAN_AND_MEAN

// Windows header files
#include <Windows.h>

// C runtime header files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <wchar.h>

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>
#include <dwmapi.h>

#pragma comment(lib,"d2d1")
#pragma comment(lib,"dwrite")
#pragma comment(lib,"windowscodecs")
#pragma comment(lib,"dwmapi")

#include "Animation.h"

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
#if defined( DEBUG ) || defined( _DEBUG )
#define Assert(b) if (!(b)) {OutputDebugStringA("Assert: " #b "\n");}
#else
#define Assert(b)
#endif //DEBUG || _DEBUG
#endif

#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif

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
	void DiscardDeviceResources();

	HRESULT OnRender();
	void OnResize(UINT width, UINT height);
	void OnTimer();

	static LRESULT CALLBACK WndProc(
		HWND hWnd,
		UINT message,
		WPARAM wParam,
		LPARAM lParam);

private:
	HWND m_hwnd;
	ID2D1Factory *m_pD2DFactory;
	ID2D1HwndRenderTarget *m_pRT;
	ID2D1PathGeometry *m_pPathGeometry;
	ID2D1PathGeometry *m_pObjectGeometry;
	ID2D1SolidColorBrush *m_pRedBrush;
	ID2D1SolidColorBrush *m_pYellowBrush;

	EaseInOutExponentialAnimation<float> m_Animation;

	DWM_TIMING_INFO m_DwmTimingInfo;
};