#include "TextAnimationSample.h"

// WinMain Application entrypoint
int APIENTRY wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nCmdShow)
{
	// Ignoring the return value because we want to continue running even in the unlikely event that HeapSetInformation fails
	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

	if (SUCCEEDED(CoInitialize(NULL)))
	{
		{
			DemoApp app;
			if (SUCCEEDED(app.Initialize()))
				app.RunMessageLoop();
		}

		CoUninitialize();
	}

	return 0;
}

DemoApp::DemoApp():
	m_hwnd(NULL),
	m_pD2DFactory(NULL),
	m_pDWriteFactory(NULL),
	m_pRT(NULL),
	m_pTextFormat(NULL),
	m_pTextLayout(NULL),
	m_pBlackBrush(NULL),
	m_pOpacityRT(NULL)
{
	m_startTime = 0;
	m_animationStyle = AnimationStyle::Translation;
	m_renderingMethod = TextRenderingMethod::Default;
}

DemoApp::~DemoApp()
{
	SafeRelease(&m_pD2DFactory);
	SafeRelease(&m_pDWriteFactory);
	SafeRelease(&m_pRT);
	SafeRelease(&m_pTextFormat);
	SafeRelease(&m_pTextLayout);
	SafeRelease(&m_pBlackBrush);
	SafeRelease(&m_pOpacityRT);
}

// Create application window and device-independent resources
HRESULT DemoApp::Initialize()
{
	m_fRunning = false;
	HRESULT hr = CreateDeviceIndependentResources();

	if (SUCCEEDED(hr))
	{
		// Register window class

	}
}