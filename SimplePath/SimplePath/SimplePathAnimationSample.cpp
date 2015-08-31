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

// Create Device Resources
HRESULT DemoApp::CreateDeviceResources()
{
	HRESULT hr = S_OK;
	if (!m_pRT)
	{
		RECT rc;
		GetClientRect(m_hwnd, &rc);

		D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

		// Create a Direct2D render target
		hr = m_pD2DFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(m_hwnd, size),
			&m_pRT);

		if (SUCCEEDED(hr))
		{
			// Create a red brush
			hr = m_pRT->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::Red),
				&m_pRedBrush);
		}
		if (SUCCEEDED(hr))
		{
			// Create a yellow brush
			hr = m_pRT->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::Yellow),
				&m_pYellowBrush);
		}
	}

	return hr;
}

// Discard device resources
void DemoApp::DiscardDeviceResources()
{
	SafeRelease(&m_pRT);
	SafeRelease(&m_pRedBrush);
	SafeRelease(&m_pYellowBrush);
}

// Main window message loop
void DemoApp::RunMessageLoop()
{
	MSG msg;
	while (GetMessage(&msg,NULL,0,0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

// OnRender
HRESULT DemoApp::OnRender()
{
	HRESULT hr = CreateDeviceResources();
	if (SUCCEEDED(hr) && !(m_pRT->CheckWindowState()&D2D1_WINDOW_STATE_OCCLUDED))
	{
		D2D1_POINT_2F point, tangent;
		D2D1_MATRIX_3X2_F triangleMatrix;
		D2D1_SIZE_F rtSize = m_pRT->GetSize();
		float minWidthHeightScale = min(rtSize.width, rtSize.height) / 512;

		D2D1::Matrix3x2F scale = D2D1::Matrix3x2F::Scale(minWidthHeightScale, minWidthHeightScale);

		D2D1::Matrix3x2F translation = D2D1::Matrix3x2F::Translation(rtSize.width / 2, rtSize.height / 2);

		// Prepare to draw
		m_pRT->BeginDraw();
		// Reset to identity transform
		m_pRT->SetTransform(D2D1::Matrix3x2F::Identity());
		// Clear the render target
		m_pRT->Clear(D2D1::ColorF(D2D1::ColorF::Black));
		// Center the path
		m_pRT->SetTransform(scale*translation);
		// Draw the path in red
		m_pRT->DrawGeometry(m_pPathGeometry, m_pRedBrush);

		static float float_time = 0.f;
		float length = m_Animation.GetValue(float_time);

		// Ask the geometry to give us the point that responds with the length at the current time
		hr = m_pPathGeometry->ComputePointAtLength(length, NULL, &point, &tangent);

		Assert(SUCCEEDED(hr));

		// Reorient the triangle so that it follows the direction of the path
		triangleMatrix = D2D1::Matrix3x2F(tangent.x, tangent.y, -tangent.y, tangent.x, point.x, point.y);

		m_pRT->SetTransform(triangleMatrix*scale*translation);

		// Draw the yellow triangle
		m_pRT->FillGeometry(m_pObjectGeometry, m_pYellowBrush);

		// Commit the drawing operations
		hr = m_pRT->EndDraw();

		if (hr == D2DERR_RECREATE_TARGET)
		{
			hr = S_OK;
			DiscardDeviceResources();
		}

		// When we reach the end of the animation, loop back to the beginning
		if (float_time >= m_Animation.GetDuration())
		{
			float_time = 0.f;
		}
		else
		{
			float_time += static_cast<float>(m_DwmTimingInfo.rateCompose.uiDenominator) /
				static_cast<float>(m_DwmTimingInfo.rateCompose.uiNumerator);
		}
	}

	InvalidateRect(m_hwnd, NULL, false);
	return hr;
}

// OnResize
void DemoApp::OnResize(UINT width, UINT height)
{
	if (m_pRT)
	{
		D2D1_SIZE_U size;
		size.width = width;
		size.height = height;

		m_pRT->Resize(size);
	}
}

// Window message handler
LRESULT CALLBACK DemoApp::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;
	if (message == WM_CREATE)
	{
		LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
		DemoApp *pDemoApp = (DemoApp *)pcs->lpCreateParams;

		::SetWindowLongPtrW(hwnd, GWLP_USERDATA, PtrToUlong(pDemoApp));

		result = 1;
	}
	else
	{
		DemoApp *pDemoApp = reinterpret_cast<DemoApp *>(static_cast<LONG_PTR>(
			::GetWindowLongPtrW(
				hwnd,
				GWLP_USERDATA)));

		bool wasHandled = false;

		if (pDemoApp)
		{
			switch (message)
			{
			case WM_SIZE:
				{
					UINT width = LOWORD(lParam);
					UINT height = HIWORD(lParam);
					pDemoApp->OnResize(width, height);
				}
				result = 0;
				wasHandled = true;
				break;
			case WM_PAINT:
			case WM_DISPLAYCHANGE:
				{
					PAINTSTRUCT ps;
					BeginPaint(hwnd, &ps);
					pDemoApp->OnRender();
					EndPaint(hwnd, &ps);
				}
				result = 0;
				wasHandled = true;
				break;
			case WM_DESTROY:
				{
					PostQuitMessage(0);
				}
				result = 1;
				wasHandled = true;
				break;
			}
		}

		if (!wasHandled)
			result = DefWindowProc(hwnd, message, wParam, lParam);
	}

	return result;
}