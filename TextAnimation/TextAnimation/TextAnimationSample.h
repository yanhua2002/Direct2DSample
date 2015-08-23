#pragma once

#ifndef WINVER
#define WINVER 0x0501
#endif // !WINVER

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif // !_WIN32_WINNT

#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0410
#endif // !_WIN32_WINDOWS

#ifndef _WIN32_IE
#define _WIN32_IE 0x0600
#endif // !_WIN32_IE

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
#include <math.h>

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>

#pragma comment(lib,"d2d1")
#pragma comment(lib,"dwrite")
#pragma comment(lib,"windowscodecs")

#define Assert(a)

#include "RingBuffer.h"

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
#define Assert(b) do {if (!(b)) {OutputDebugStringA("Assert: " #b "\n");}} while(0)
#else
#define Assert(b)
#endif //DEBUG || _DEBUG
#endif

#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif

// DemoApp
namespace AnimationStyle
{
	enum Enum
	{
		None = 0,
		Translation = 1,
		Rotation = 2,
		Scaling = 4
	};
}

DEFINE_ENUM_FLAG_OPERATORS(AnimationStyle::Enum);

namespace TextRenderingMethod
{
	enum Enum
	{
		Default,
		Outline,
		UseA8Target,
		NumValues
	};
}

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
	HRESULT OnChar(SHORT key);
	void OnDestroy();

	static LRESULT CALLBACK WndProc(
		HWND hWnd,
		UINT message,
		WPARAM wParam,
		LPARAM lParam);

	bool IsRunning()
	{
		return m_fRunning;
	}

	void UpdateWindowText();

	HRESULT ResetAnimation(bool resetClock);

	void CalculateTransform(D2D1_MATRIX_3X2_F *pTransform);

private:
	DWORD m_startTime;
	AnimationStyle::Enum m_animationStyle;
	TextRenderingMethod::Enum m_renderingMethod;
	D2D1_POINT_2F m_overhangOffset;

	HWND m_hwnd;
	ID2D1Factory *m_pD2DFactory;
	IDWriteFactory *m_pDWriteFactory;
	ID2D1HwndRenderTarget *m_pRT;
	IDWriteTextFormat *m_pTextFormat;
	IDWriteTextLayout *m_pTextLayout;
	ID2D1SolidColorBrush *m_pBlackBrush;
	ID2D1BitmapRenderTarget *m_pOpacityRT;
	bool m_fRunning;

	RingBuffer<LONG, 10> m_times;
};

