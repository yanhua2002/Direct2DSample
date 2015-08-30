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

DemoApp::DemoApp() :
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
			L"D2DDemoApp", L"D2D Demo App", WS_OVERLAPPEDWINDOW,
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

void DemoApp::DiscardDeviceResources()
{
	SafeRelease(&m_pRT);
	SafeRelease(&m_pBlackBrush);
	SafeRelease(&m_pOpacityRT);
}

// Main window message loop
void DemoApp::RunMessageLoop()
{
	while (this->IsRunning())
	{
		MSG msg;
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

// Responds to input from the user
HRESULT DemoApp::OnChar(SHORT key)
{
	HRESULT hr = S_OK;

	bool resetAnimation = true;
	bool resetClock = true;

	switch (key)
	{
	case 't':
		if (m_animationStyle&AnimationStyle::Translation)
			m_animationStyle &= ~AnimationStyle::Translation;
		else
			m_animationStyle |= AnimationStyle::Translation;
		break;
	case 'r':
		if (m_animationStyle&AnimationStyle::Rotation)
			m_animationStyle &= ~AnimationStyle::Rotation;
		else
			m_animationStyle |= AnimationStyle::Rotation;
		break;
	case 's':
		if (m_animationStyle&AnimationStyle::Scaling)
			m_animationStyle &= ~AnimationStyle::Scaling;
		else
			m_animationStyle |= AnimationStyle::Scaling;
		break;

	case '1':
	case '2':
	case '3':
		m_renderingMethod = static_cast<TextRenderingMethod::Enum>(key - '1');
		resetClock = false;
		break;

	default:
		resetAnimation = false;
		resetClock = false;
	}

	if (resetAnimation)
		hr = ResetAnimation(resetClock);

	return hr;
}

// Updates the window title bar with info about the current
// animation style and rendering method. It also outputs the frame rate
void DemoApp::UpdateWindowText()
{
	static LONGLONG sc_lastTimeStatusShown = 0;

	// Update the window status no more than 10 times per second, without this check the performance bottleneck
	// could potentially be the time it takes for windows to update the title
	if (m_times.GetCount() > 0 && m_times.GetLast() > sc_lastTimeStatusShown + 1000000)
	{
		// Determine the frame rate by computing the difference in clock time between this frame and 
		// the frame we rendered 10 frames ago
		sc_lastTimeStatusShown = m_times.GetLast();

		LARGE_INTEGER frequency;
		QueryPerformanceFrequency(&frequency);

		float fps = 0.0f;
		if (m_times.GetCount() > 0)
			fps = (m_times.GetCount() - 1)*frequency.QuadPart / static_cast<float>((m_times.GetLast() - m_times.GetFirst()));

		// Add other useful information to the window title
		wchar_t *style = NULL;
		switch (m_animationStyle)
		{
		case AnimationStyle::None:
			style = L"None";
			break;
		case AnimationStyle::Translation:
			style = L"Translation";
			break;
		case AnimationStyle::Rotation:
			style = L"Rotation";
			break;
		case AnimationStyle::Scaling:
			style = L"Scale";
			break;
		}

		wchar_t *method = NULL;
		switch (m_renderingMethod)
		{
		case TextRenderingMethod::Default:
			method = L"Default";
			break;
		case TextRenderingMethod::Outline:
			method = L"Outline";
			break;
		case TextRenderingMethod::UseA8Target:
			method = L"UseA8Target";
			break;
		}

		wchar_t title[255];
		StringCchPrintf(
			title,
			ARRAYSIZE(title),
			L"AnimationStyle: %s%s%s, Method: %s, %#.1f fps",
			(m_animationStyle&AnimationStyle::Translation) ? L"+t" : L"-t",
			(m_animationStyle&AnimationStyle::Rotation) ? L"+r" : L"-r",
			(m_animationStyle&AnimationStyle::Scaling) ? L"+s" : L"-s",
			method,
			fps);
		SetWindowText(m_hwnd, title);
	}
}

// This method does the necessary work to change the current animation style
HRESULT DemoApp::ResetAnimation(bool resetClock)
{
	HRESULT hr = S_OK;
	if (resetClock)
		m_startTime = GetTickCount();

	// Release the opacity mask. We will regenerate it if the current animation style demands it
	SafeRelease(&m_pOpacityRT);

	if (m_renderingMethod == TextRenderingMethod::Outline)
	{
		// Set the rendering mode to OUTLINE mode. To do this we first create
		// a default params object and then make a copy with the given modification
		IDWriteRenderingParams *pDefaultParams = NULL;
		hr = m_pDWriteFactory->CreateRenderingParams(&pDefaultParams);

		if (SUCCEEDED(hr))
		{
			IDWriteRenderingParams *pRenderingParams = NULL;
			hr = m_pDWriteFactory->CreateCustomRenderingParams(
				pDefaultParams->GetGamma(),
				pDefaultParams->GetEnhancedContrast(),
				pDefaultParams->GetClearTypeLevel(),
				pDefaultParams->GetPixelGeometry(),
				DWRITE_RENDERING_MODE_OUTLINE,
				&pRenderingParams);
			if (SUCCEEDED(hr))
			{
				m_pRT->SetTextRenderingParams(pRenderingParams);
				pRenderingParams->Release();
			}
			pDefaultParams->Release();
		}
	}
	else
		// Reset the rendering mode to default
		m_pRT->SetTextRenderingParams(NULL);

	if (SUCCEEDED(hr) && m_renderingMethod == TextRenderingMethod::UseA8Target)
	{
		// Create a compatible A8 Target to store the text as an opacity mask
		// Note: To reduce sampling error in the scale animation, it might be preferable
		// to create multiple masks for the text at different resolutions

		float dpiX, dpiY;
		m_pRT->GetDpi(&dpiX, &dpiY);

		// It is important to obtain the overhang metrics here in case the text
		// extends beyond the layout max-width and max-height
		DWRITE_OVERHANG_METRICS overhangMetrics;
		m_pTextLayout->GetOverhangMetrics(&overhangMetrics);

		// Because the overhang metrics can be off slightly given that these
		// metrics do not account for antiliasing, we add an extra pixel for padding
		D2D1_SIZE_F padding = D2D1::SizeF(96.f / dpiX, 96.f / dpiY);
		m_overhangOffset = D2D1::Point2F(ceil(overhangMetrics.left + padding.width), ceil(overhangMetrics.top + padding.height));

		// The true width of the text is the max width + the overhang metrics + padding in each direction
		D2D1_SIZE_F maskSize = D2D1::SizeF(
			overhangMetrics.right + padding.width + m_overhangOffset.x + m_pTextLayout->GetMaxWidth(),
			overhangMetrics.bottom + padding.height + m_overhangOffset.y + m_pTextLayout->GetMaxHeight());

		// Round up to the nearest pixel
		D2D1_SIZE_U maskPixelSize = D2D1::SizeU(
			static_cast<UINT>(ceil(maskSize.width*dpiX / 96.f)),
			static_cast<UINT>(ceil(maskSize.height*dpiY / 96.f)));

		// Create the compatible render target using desirePixelSize to avoid
		// blurriness isssues caused by a fractional-pixel desiredSize
		D2D1_PIXEL_FORMAT alphaOnlyFormat = D2D1::PixelFormat(DXGI_FORMAT_A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED);
		hr = m_pRT->CreateCompatibleRenderTarget(
			NULL,
			&maskPixelSize,
			&alphaOnlyFormat,
			D2D1_COMPATIBLE_RENDER_TARGET_OPTIONS_NONE,
			&m_pOpacityRT);
		if (SUCCEEDED(hr))
		{
			// Draw the text to the opacity mask. Note that we can use pixel snapping now given
			// that subpixel translation can now happen during the FillOpacityMask method
			m_pOpacityRT->BeginDraw();
			m_pOpacityRT->Clear(D2D1::ColorF(D2D1::ColorF::Black, 0.f));
			m_pOpacityRT->DrawTextLayout(
				m_overhangOffset,
				m_pTextLayout,
				m_pBlackBrush,
				D2D1_DRAW_TEXT_OPTIONS_NO_SNAP);
			hr = m_pOpacityRT->EndDraw();
		}
	}

	return hr;
}

// Calculates the transform based on the current time
void DemoApp::CalculateTransform(D2D1_MATRIX_3X2_F *pTransform)
{
	// Calculate a 't' value that will linearly interpolate from 0 to 1 and back every 20s
	DWORD currentTime = GetTickCount();
	if (m_startTime == 0)
		m_startTime = currentTime;
	float t = 2 * ((currentTime - m_startTime) % 20000) / 20000.f;
	if (t > 1.f)
		t = 2 - t;

	// Calculate animation parameters
	float rotation = 0;
	float translationOffset = 0;
	float scaleMultiplier = 1.f;
	if (m_animationStyle&AnimationStyle::Translation)
	{
		// range from -100 to 100
		translationOffset = (t - 0.5f) * 200;
	}

	if (m_animationStyle&AnimationStyle::Rotation)
	{
		// range from 0 to 360
		rotation = t*360.f;
	}

	if (m_animationStyle&AnimationStyle::Scaling)
	{
		// range from 1/4 to 2x the normal size
		scaleMultiplier = t*1.75f + 0.25f;
	}

	D2D1_SIZE_F size = m_pRT->GetSize();
	*pTransform =
		D2D1::Matrix3x2F::Rotation(rotation)
		*D2D1::Matrix3x2F::Scale(scaleMultiplier, scaleMultiplier)
		*D2D1::Matrix3x2F::Translation(translationOffset + size.width / 2.f, translationOffset + size.height / 2.f);
}

// Called whenever the app needs to display the client window
HRESULT DemoApp::OnRender()
{
	HRESULT hr;
	// We use a ring buffer to store the clock time for the last 10 frames
	// This lets us eliminate a lot of noise when computing framerates
	LARGE_INTEGER time;
	QueryPerformanceCounter(&time);
	m_times.Add(time.QuadPart);

	hr = CreateDeviceResources();

	if (SUCCEEDED(hr) && !(m_pRT->CheckWindowState()&D2D1_WINDOW_STATE_OCCLUDED))
	{
		D2D1_MATRIX_3X2_F transform;
		CalculateTransform(&transform);

		m_pRT->BeginDraw();

		m_pRT->Clear(D2D1::ColorF(D2D1::ColorF::White));
		m_pRT->SetTransform(transform);

		DWRITE_TEXT_METRICS textMetrics;
		m_pTextLayout->GetMetrics(&textMetrics);

		if (m_renderingMethod == TextRenderingMethod::UseA8Target)
		{
			// Offset the destination rect such that the text will be centered on the render target
			D2D1_SIZE_F opacityRTSize = m_pOpacityRT->GetSize();
			D2D1_POINT_2F offset = D2D1::Point2F(
				-textMetrics.width / 2.f - m_overhangOffset.x,
				-textMetrics.height / 2.f - m_overhangOffset.y);

			// Round the offset to the nearest pixel. Note that the rounding done here is unnecessary,
			// but it causes the test to be less blurry
			float dpiX, dpiY;
			m_pRT->GetDpi(&dpiX, &dpiY);
			D2D1_POINT_2F roundedOffset = D2D1::Point2F(
				floor(offset.x*dpiX / 96.f + 0.5f)*96.f / dpiX,
				floor(offset.y*dpiY / 96.f + 0.5f)*96.f / dpiY);

			D2D1_RECT_F destinationRect = D2D1::RectF(
				roundedOffset.x,
				roundedOffset.y,
				roundedOffset.x + opacityRTSize.width,
				roundedOffset.y + opacityRTSize.height);

			ID2D1Bitmap *pBitmap = NULL;
			m_pOpacityRT->GetBitmap(&pBitmap);

			pBitmap->GetDpi(&dpiX, &dpiY);

			// The antilias mode must be set to D2D1_ANTILIAS_MODE_ALIASED
			// for this method to succeed. We've set this mode already though
			// so no need to do it again
			m_pRT->FillOpacityMask(
				pBitmap,
				m_pBlackBrush,
				D2D1_OPACITY_MASK_CONTENT_TEXT_NATURAL,
				&destinationRect);

			pBitmap->Release();
		}
		else
		{
			// Disable pixel snapping to get a smoother animation
			m_pRT->DrawTextLayout(
				D2D1::Point2F(-textMetrics.width / 2.f, -textMetrics.height / 2.f),
				m_pTextLayout,
				m_pBlackBrush,
				D2D1_DRAW_TEXT_OPTIONS_NO_SNAP);
		}

		hr = m_pRT->EndDraw();

		if (hr == D2DERR_RECREATE_TARGET)
		{
			hr = S_OK;
			DiscardDeviceResources();
		}

		// To animate as quickly as possible, we request another WM_PAINT immediately
		InvalidateRect(m_hwnd, NULL, false);
	}

	UpdateWindowText();
	return hr;
}

// OnResize, resizes the render target appropriately
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

// OnDestroy
void DemoApp::OnDestroy()
{
	m_fRunning = false;
}

// Window message handler
LRESULT CALLBACK DemoApp::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;
	if (message == WM_CREATE)
	{
		LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
		DemoApp *pDemoApp = (DemoApp *)pcs->lpCreateParams;

		::SetWindowLongPtrW(
			hwnd,
			GWLP_USERDATA,
			PtrToUlong(pDemoApp));

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

			case WM_CHAR:
				{
					pDemoApp->OnChar(static_cast<short>(wParam));
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
					pDemoApp->OnDestroy();
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