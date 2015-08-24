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
		WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = DemoApp::WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = sizeof(LONG_PTR);
		wcex.hInstance = HINST_THISCOMPONENT;
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = NULL;
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = L"D2DDemoApp";

		RegisterClassEx(&wcex);

		// Create the application window
		float dpiX, dpiY;
		m_pD2DFactory->GetDesktopDpi(&dpiX, &dpiY);

		m_hwnd = CreateWindow(
			L"D2DDemApp", L"D2D Demo App", WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, static_cast<UINT>(ceil(640.f*dpiX / 96.f)), static_cast<UINT>(ceil(480.f*dpiY / 96.f)),
			NULL, NULL, HINST_THISCOMPONENT, this);
		hr = m_hwnd ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			m_fRunning = true;

			ShowWindow(m_hwnd, SW_SHOWNORMAL);
			UpdateWindow(m_hwnd);
		}
	}

	return hr;
}

// Create resources which are not bound to any device
HRESULT DemoApp::CreateDeviceIndependentResources()
{
	static const wchar_t msc_fontName[] = L"Gabriola";
	static const float msc_fontSize = 50;
	static const wchar_t sc_helloWorld[] = L"The quick brown fox jumped over the lazy dog!";
	static const UINT stringLength = ARRAYSIZE(sc_helloWorld) - 1;

	// Create D2D factory
	HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);
	if (SUCCEEDED(hr))
	{
		// Create DWrite factory
		hr = DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_ISOLATED,
			__uuidof(m_pDWriteFactory),
			reinterpret_cast<IUnknown **>(&m_pDWriteFactory));
	}
	if (SUCCEEDED(hr))
	{
		// Create DWrite text format object
		hr = m_pDWriteFactory->CreateTextFormat(
			msc_fontName,
			NULL,
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			msc_fontSize,
			L"",
			&m_pTextFormat);
	}
	if (SUCCEEDED(hr))
	{
		// Center the text horizontally
		m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

		hr = m_pDWriteFactory->CreateTextLayout(
			&sc_helloWorld[0],
			stringLength,
			m_pTextFormat,
			300,
			1000,
			&m_pTextLayout);
	}
	if (SUCCEEDED(hr))
	{
		// We use typographic features here to show how to account for the
		// overhangs that these features will produce. See the code in
		// ResetAnimation that calls GetOverhangMetrics(). Note that there are
		// fonts that can produce overhangs even without the use of typographic
		// features - this is just one example
		IDWriteTypography *pTypography = NULL;
		hr = m_pDWriteFactory->CreateTypography(&pTypography);
		if (SUCCEEDED(hr))
		{
			DWRITE_FONT_FEATURE fontFeature = { DWRITE_FONT_FEATURE_TAG_STYLISTIC_SET_7,1 };
			hr = pTypography->AddFontFeature(fontFeature);
			if (SUCCEEDED(hr))
			{
				DWRITE_TEXT_RANGE textRange = { 0,stringLength };
				hr = m_pTextLayout->SetTypography(pTypography, textRange);
			}

			pTypography->Release();
		}
	}

	return hr;
}

// Create device resources
HRESULT DemoApp::CreateDeviceResources()
{
	HRESULT hr = S_OK;
	if (!m_pRT)
	{
		RECT rc;
		GetClientRect(m_hwnd, &rc);
		D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

		// Create a D2D render target
		hr = m_pD2DFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(m_hwnd, size, D2D1_PRESENT_OPTIONS_IMMEDIATELY),
			&m_pRT);
		if (SUCCEEDED(hr))
		{
			// Nothing in this sample requires antialiasing so we set the antialias mode to
			// aliased up front
			m_pRT->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);

			// Create a blank brush
			hr = m_pRT->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::Black),
				&m_pBlackBrush);
		}
		if (SUCCEEDED(hr))
			hr = ResetAnimation(true);
	}

	return hr;
}