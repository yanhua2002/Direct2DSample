#include "SimplePathAnimationSample.h"

// Application entrypoint
int APIENTRY wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nCmdShow)
{
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

// DemoApp constructor, initialize member data
DemoApp::DemoApp() :
	m_hwnd(NULL),
	m_pD2DFactory(NULL),
	m_pRT(NULL),
	m_pPathGeometry(NULL),
	m_pObjectGeometry(NULL),
	m_pYellowBrush(NULL),
	m_pRedBrush(NULL),
	m_Animation()
{

}

// Destructor, release resources
DemoApp::~DemoApp()
{
	SafeRelease(&m_pD2DFactory);
	SafeRelease(&m_pRT);
	SafeRelease(&m_pPathGeometry);
	SafeRelease(&m_pObjectGeometry);
	SafeRelease(&m_pRedBrush);
	SafeRelease(&m_pYellowBrush);
}

// Create application window and device-independent resources
HRESULT DemoApp::Initialize()
{
	HRESULT hr;

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

	hr = CreateDeviceIndependentResources();
	if (SUCCEEDED(hr))
	{
		// Create the application window
		float dpiX, dpiY;
		m_pD2DFactory->GetDesktopDpi(&dpiX, &dpiY);

		m_hwnd = CreateWindow(
			L"D2DDemoApp", L"D2D Simple Path Animation Sample", WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, static_cast<UINT>(ceil(512.f*dpiX / 96.f)), static_cast<UINT>(ceil(512.f*dpiY / 96.f)),
			NULL, NULL, HINST_THISCOMPONENT, this);

		hr = m_hwnd ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			float length = 0;
			hr = m_pPathGeometry->ComputeLength(NULL, &length);
			if (SUCCEEDED(hr))
			{
				m_Animation.SetStart(0);
				m_Animation.SetEnd(length);
				m_Animation.SetDuration(5.f);

				ZeroMemory(&m_DwmTimingInfo, sizeof(m_DwmTimingInfo));
				m_DwmTimingInfo.cbSize = sizeof(m_DwmTimingInfo);

				// Get the composition refresh rate
				if (FAILED(DwmGetCompositionTimingInfo(NULL, &m_DwmTimingInfo)))
				{
					HDC hdc = GetDC(m_hwnd);
					m_DwmTimingInfo.rateCompose.uiDenominator = 1;
					m_DwmTimingInfo.rateCompose.uiNumerator = GetDeviceCaps(hdc, VREFRESH);
					ReleaseDC(m_hwnd, hdc);
				}

				ShowWindow(m_hwnd, SW_SHOWNORMAL);
				UpdateWindow(m_hwnd);
			}
		}
	}

	return hr;
}

// Create device independent resources
HRESULT DemoApp::CreateDeviceIndependentResources()
{
	HRESULT hr;
	ID2D1GeometrySink *pSink = NULL;

	// Create a Direct2D factory
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);
	if (SUCCEEDED(hr))
	{
		// Create the path geometry
		hr = m_pD2DFactory->CreatePathGeometry(&m_pPathGeometry);
	}
	if (SUCCEEDED(hr))
	{
		// Write to the path geometry using the geometry sink.
		// We are going to create a spiral
		hr = m_pPathGeometry->Open(&pSink);
	}
	if (SUCCEEDED(hr))
	{
		D2D1_POINT_2F currentLocation = { 0,0 };
		pSink->BeginFigure(currentLocation, D2D1_FIGURE_BEGIN_FILLED);
		D2D1_POINT_2F locDelta = { 2,2 };
		float radius = 3;

		for (UINT i = 0; i < 30; i++)
		{
			currentLocation.x += radius*locDelta.x;
			currentLocation.y += radius*locDelta.y;

			pSink->AddArc(
				D2D1::ArcSegment(
					currentLocation,
					D2D1::SizeF(2 * radius, 2 * radius),
					0.0f,
					D2D1_SWEEP_DIRECTION_CLOCKWISE,
					D2D1_ARC_SIZE_SMALL));

			locDelta = D2D1::Point2F(-locDelta.y, locDelta.x);
			radius += 3;
		}

		pSink->EndFigure(D2D1_FIGURE_END_OPEN);
		hr = pSink->Close();
	}
	if (SUCCEEDED(hr))
	{
		// Create the path geometry
		hr = m_pD2DFactory->CreatePathGeometry(&m_pObjectGeometry);
	}
	if (SUCCEEDED(hr))
	{
		// Write to the object geometry using the geometry sink
		// we are going to create a simple triangle
		hr = m_pObjectGeometry->Open(&pSink);
	}
	if (SUCCEEDED(hr))
	{
		pSink->BeginFigure(
			D2D1::Point2F(0.0f, 0.0f),
			D2D1_FIGURE_BEGIN_FILLED);

		const D2D1_POINT_2F ptTriangle[] = { {-10.f,-10.f},{-10.f,10.f},{0.f,0.f} };
		pSink->AddLines(ptTriangle, 3);

		pSink->EndFigure(D2D1_FIGURE_END_OPEN);
		hr = pSink->Close();
	}

	SafeRelease(&pSink);
	return hr;
}