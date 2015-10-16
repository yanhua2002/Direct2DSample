// FirstTry.cpp : Defines the FirstTry class.
//

#include "stdafx.h"
#include "FirstTry.h"

FirstTry::FirstTry() :
	m_hwnd(NULL),
	m_pD2DFactory(NULL),
	m_pRenderTarget(NULL),
	m_pWICFactory(NULL),
	m_pLightSlateGrayBrush(NULL),
	m_pCornflowerBlueBrush(NULL),
	m_pBitmap(NULL),
	m_pBitmap1(NULL),
	m_pAnimationManager(NULL),
	m_pAnimationTimer(NULL),
	m_pTransitionLibrary(NULL),
	m_pAnimationVariableAngle(NULL)
{

}

FirstTry::~FirstTry()
{
	// D2D
	SafeRelease(&m_pD2DFactory);
	SafeRelease(&m_pRenderTarget);
	SafeRelease(&m_pWICFactory);
	SafeRelease(&m_pLightSlateGrayBrush);
	SafeRelease(&m_pCornflowerBlueBrush);
	SafeRelease(&m_pBitmap);
	SafeRelease(&m_pBitmap1);

	// Animation
	SafeRelease(&m_pAnimationManager);
	SafeRelease(&m_pAnimationTimer);
	SafeRelease(&m_pTransitionLibrary);
	SafeRelease(&m_pAnimationVariableAngle);
}

// Create the window, show it.
HRESULT FirstTry::Initialize()
{
	HRESULT hr = CreateDeviceIndependentResources();

	if (SUCCEEDED(hr))
	{
		// Register the window class
		WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = FirstTry::WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = sizeof(LONG_PTR);
		wcex.hInstance = HINST_THISCOMPONENT;
		wcex.hbrBackground = NULL;
		wcex.lpszMenuName = NULL;
		wcex.hCursor = LoadCursor(NULL, IDI_APPLICATION);
		wcex.lpszClassName = L"FirstTry";

		RegisterClassEx(&wcex);

		// Obtain the system DPI
		float dpiX, dpiY;
		m_pD2DFactory->GetDesktopDpi(&dpiX, &dpiY);

		// Create the window
		m_hwnd = CreateWindow(L"FirstTry", L"First Try", WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, static_cast<UINT>(ceil(640.f*dpiX / 96.f)), static_cast<UINT>(ceil(480.f*dpiY / 96.f)),
			NULL, NULL, HINST_THISCOMPONENT, this);

		hr = m_hwnd ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			ShowWindow(m_hwnd, SW_SHOWNORMAL);
			UpdateWindow(m_hwnd);
		}
	}

	return hr;
}

// Translate and dispatch messages
void FirstTry::RunMessageLoop()
{
	MSG msg;

	while (GetMessage(&msg,NULL,0,0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

// Creates and initializes the main animation components.
HRESULT FirstTry::InitializeAnimation()
{
	// Create Animation Manager.
	HRESULT hr = CoCreateInstance(CLSID_UIAnimationManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pAnimationManager));

	// Create Animation Timer.
	if (SUCCEEDED(hr))
	{
		hr = CoCreateInstance(CLSID_UIAnimationTimer, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pAnimationTimer));

		// Create Animation Transition Library
		if (SUCCEEDED(hr))
		{
			hr = CoCreateInstance(CLSID_UIAnimationTransitionLibrary, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pTransitionLibrary));

			// Create and set the ManagerEventHandler to start updating when animations are scheduled
			if (SUCCEEDED(hr))
			{
				IUIAnimationManagerEventHandler *pManagerEventHandler;
			}
		}
	}
}

HRESULT FirstTry::CreateDeviceIndependentResources()
{
	HRESULT hr = S_OK;
	
	// Create a Direct2D factory.
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);

	if (SUCCEEDED(hr))
		hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pWICFactory));

	return hr;
}

HRESULT FirstTry::CreateDeviceResources()
{
	HRESULT hr = S_OK;
	if (!m_pRenderTarget)
	{
		RECT rc;
		GetClientRect(m_hwnd, &rc);

		D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

		// Create a Direct2D render target.
		hr = m_pD2DFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(m_hwnd, size),
			&m_pRenderTarget);

		if (SUCCEEDED(hr))
		{
			// Create a gray brush.
			hr = m_pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::LightSlateGray),
				&m_pLightSlateGrayBrush);
		}

		if (SUCCEEDED(hr))
		{
			// Create a blue brush.
			hr = m_pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::CornflowerBlue),
				&m_pCornflowerBlueBrush);
		}

		// Create a bitmap from an application resource.
		if (SUCCEEDED(hr))
		{
			hr = LoadResourceBitmap(
				m_pRenderTarget,
				m_pWICFactory,
				L"SAMPLEIMAGE",
				L"IMAGE",
				0,
				0,
				&m_pBitmap);
		}

		// Create a bitmap by loading it from a file.
		if (SUCCEEDED(hr))
		{
			hr = LoadBitmapFromFile(
				m_pRenderTarget,
				m_pWICFactory,
				L".\\sampleImage.jpg",
				0,
				0,
				&m_pBitmap1);
		}
	}

	return hr;
}

void FirstTry::DiscardDeviceResources()
{
	SafeRelease(&m_pRenderTarget);
	SafeRelease(&m_pLightSlateGrayBrush);
	SafeRelease(&m_pCornflowerBlueBrush);
}

LRESULT CALLBACK FirstTry::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;
	if (message == WM_CREATE)
	{
		LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
		FirstTry *pFirstTry = (FirstTry *)pcs->lpCreateParams;

		::SetWindowLongPtrW(hwnd, GWLP_USERDATA, PtrToUlong(pFirstTry));

		result = 1;
	}
	else
	{
		FirstTry *pFirstTry = reinterpret_cast<FirstTry *>(static_cast<LONG_PTR>(
			::GetWindowLongPtrW(hwnd, GWLP_USERDATA)));

		bool wasHandled = false;

		if (pFirstTry)
		{
			switch (message)
			{
			case WM_SIZE:
				{
					UINT width = LOWORD(lParam);
					UINT height = HIWORD(lParam);
					pFirstTry->OnResize(width, height);
				}
				result = 0;
				wasHandled = true;
				break;

			case WM_DISPLAYCHANGE:
				{
					InvalidateRect(hwnd, NULL, FALSE);
				}
				result = 0;
				wasHandled = true;
				break;

			case WM_PAINT:
				{
					pFirstTry->OnRender();
					ValidateRect(hwnd, NULL);
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

HRESULT FirstTry::OnRender()
{
	HRESULT hr = S_OK;
	hr = CreateDeviceResources();

	if (SUCCEEDED(hr))
	{
		m_pRenderTarget->BeginDraw();
		m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
		m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

		// Retrieve the size of the drawing area.
		D2D1_SIZE_F rtSize = m_pRenderTarget->GetSize();

		// Draw a grid background by using a for loop and the render target's DrawLine method.
		int width = static_cast<int>(rtSize.width);
		int height = static_cast<int>(rtSize.height);

		for (int x = 0; x < width; x+=10)
		{
			m_pRenderTarget->DrawLine(
				D2D1::Point2F(static_cast<FLOAT>(x), 0.f),
				D2D1::Point2F(static_cast<FLOAT>(x), rtSize.height),
				m_pLightSlateGrayBrush,
				0.5f);
		}
		for (int y = 0; y < height; y+=10)
		{
			m_pRenderTarget->DrawLine(
				D2D1::Point2F(0.f, static_cast<float>(y)),
				D2D1::Point2F(rtSize.width, static_cast<float>(y)),
				m_pLightSlateGrayBrush,
				0.5f);
		}

		// Draw a bitmap in the upper-left corner.
		D2D1_SIZE_F size = m_pBitmap->GetSize();
		m_pRenderTarget->DrawBitmap(m_pBitmap, D2D1::RectF(0.f, 0.f, size.width, size.height));

		// Draw a bitmap 10 pixels off right.
		size = m_pBitmap1->GetSize();
		m_pRenderTarget->DrawBitmap(m_pBitmap1, D2D1::RectF(10.f, 10.f, size.width + 10.f, size.height + 10.f));

		// Draw two rectangles
		D2D1_RECT_F rectangle1 = D2D1::RectF(
			rtSize.width / 2 - 50.f,
			rtSize.height / 2 - 50.f,
			rtSize.width / 2 + 50.f,
			rtSize.height / 2 + 50.f);
		D2D1_RECT_F rectangle2 = D2D1::RectF(
			rtSize.width / 2 - 100.f,
			rtSize.height / 2 - 100.f,
			rtSize.width / 2 + 100.f,
			rtSize.height / 2 + 100.f);

		m_pRenderTarget->FillRectangle(&rectangle1, m_pLightSlateGrayBrush);
		m_pRenderTarget->DrawRectangle(&rectangle2, m_pCornflowerBlueBrush);

		hr = m_pRenderTarget->EndDraw();
	}

	if (hr == D2DERR_RECREATE_TARGET)
	{
		hr = S_OK;
		DiscardDeviceResources();
	}

	return hr;
}

void FirstTry::OnResize(UINT width, UINT height)
{
	if (m_pRenderTarget)
	{
		m_pRenderTarget->Resize(D2D1::SizeU(width, height));
	}
}

// Create a Direct2D bitmap from a resource in the application resource file
HRESULT FirstTry::LoadResourceBitmap(
	ID2D1RenderTarget *pRenderTarget,
	IWICImagingFactory *pIWICFactory,
	PCWSTR resourceName,
	PCWSTR resourceType,
	UINT destinationWidth,
	UINT destinationHeight,
	ID2D1Bitmap **ppBitmap)
{
	HRESULT hr = S_OK;
	IWICBitmapDecoder *pDecoder = NULL;
	IWICBitmapFrameDecode *pSource = NULL;
	IWICStream *pStream = NULL;
	IWICFormatConverter *pConverter = NULL;
	IWICBitmapScaler *pScaler = NULL;

	HRSRC imageResHandle = NULL;
	HGLOBAL imageResDataHandle = NULL;
	void *pImageFile = NULL;
	DWORD imageFileSize = 0;

	// Locate the resource
	imageResHandle = FindResource(HINST_THISCOMPONENT, resourceName, resourceType);
	hr = imageResHandle ? S_OK : E_FAIL;

	if (SUCCEEDED(hr))
	{
		// Load the resource
		imageResDataHandle = LoadResource(HINST_THISCOMPONENT, imageResHandle);
		hr = imageResDataHandle ? S_OK : E_FAIL;

		if (SUCCEEDED(hr))
		{
			// Lock it to get a system memory pointer
			pImageFile = LockResource(imageResDataHandle);
			hr = pImageFile ? S_OK : E_FAIL;
		}

		if (SUCCEEDED(hr))
		{
			// Calculate the size
			imageFileSize = SizeofResource(HINST_THISCOMPONENT, imageResHandle);
			hr = imageFileSize ? S_OK : E_FAIL;
		}
	}
	if (SUCCEEDED(hr))
	{
		// Create a WIC stream to map onto the memory
		hr = pIWICFactory->CreateStream(&pStream);
		if (SUCCEEDED(hr))
		{
			// Initialize the stream with the memory pointer and size
			hr = pStream->InitializeFromMemory(
				reinterpret_cast<BYTE*>(pImageFile),
				imageFileSize);
		}
	}
	if (SUCCEEDED(hr))
	{
		hr = pIWICFactory->CreateDecoderFromStream(
			pStream,
			NULL,
			WICDecodeMetadataCacheOnLoad,
			&pDecoder);
		if (SUCCEEDED(hr))
		{
			// Create the initial frame
			hr = pDecoder->GetFrame(0, &pSource);
		}
	}
	if (SUCCEEDED(hr))
	{
		// Convert the image format to 32bppPBGRA
		hr = pIWICFactory->CreateFormatConverter(&pConverter);

		if (SUCCEEDED(hr))
		{
			// If a new width or height was specified, create an IWICBitmapScaler and
			// use it to resize the image
			if (destinationWidth != 0 || destinationHeight != 0)
			{
				UINT originalWidth, originalHeight;
				hr = pSource->GetSize(&originalWidth, &originalHeight);
				if (SUCCEEDED(hr))
				{
					if (destinationWidth == 0)
					{
						float scalar = static_cast<float>(destinationHeight) / static_cast<float>(originalHeight);
						destinationWidth = static_cast<UINT>(scalar*static_cast<float>(originalWidth));
					}
					else if (destinationHeight == 0)
					{
						float scalar = static_cast<float>(destinationWidth) / static_cast<float>(originalWidth);
						destinationHeight = static_cast<UINT>(scalar*static_cast<float>(originalHeight));
					}

					hr = pIWICFactory->CreateBitmapScaler(&pScaler);
					if (SUCCEEDED(hr))
					{
						hr = pScaler->Initialize(pSource, destinationWidth, destinationHeight, WICBitmapInterpolationModeCubic);

						if (SUCCEEDED(hr))
						{
							hr = pConverter->Initialize(pScaler, GUID_WICPixelFormat32bppBGRA, WICBitmapDitherTypeNone, NULL, 0.f, WICBitmapPaletteTypeMedianCut);
						}
					}
				}
			}
			else
			{
				hr = pConverter->Initialize(pSource, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.f, WICBitmapPaletteTypeMedianCut);
			}
		}
	}
	if (SUCCEEDED(hr))
	{
		// Create a Direct2D bitmap from the WIC bitmap
		hr = pRenderTarget->CreateBitmapFromWicBitmap(pConverter, NULL, ppBitmap);
	}

	SafeRelease(&pDecoder);
	SafeRelease(&pSource);
	SafeRelease(&pStream);
	SafeRelease(&pConverter);
	SafeRelease(&pScaler);

	return hr;
}

// Create a Direct2D bitmap from the specified file name
HRESULT FirstTry::LoadBitmapFromFile(
	ID2D1RenderTarget *pRenderTarget,
	IWICImagingFactory *pIWICFactory,
	PCWSTR uri,
	UINT destinationWidth,
	UINT destinationHeight,
	ID2D1Bitmap **ppBitmap)
{
	HRESULT hr = S_OK;

	IWICBitmapDecoder *pDecoder = NULL;
	IWICBitmapFrameDecode *pSource = NULL;
	IWICStream *pStream = NULL;
	IWICFormatConverter *pConverter = NULL;
	IWICBitmapScaler *pScaler = NULL;

	hr = pIWICFactory->CreateDecoderFromFilename(
		uri,
		NULL,
		GENERIC_READ,
		WICDecodeMetadataCacheOnLoad,
		&pDecoder);

	if (SUCCEEDED(hr))
	{
		// Create the initial frame
		hr = pDecoder->GetFrame(0, &pSource);
	}

	if (SUCCEEDED(hr))
	{
		// Convert the image format to 32bppPBGRA
		hr = pIWICFactory->CreateFormatConverter(&pConverter);

		if (SUCCEEDED(hr))
		{
			// If a new width or height was specified, create a IWICBitmapScaler and use it
			// to resize the image
			if (destinationWidth != 0 || destinationHeight != 0)
			{
				UINT originalWidth, originalHeight;
				hr = pSource->GetSize(&originalWidth, &originalHeight);
				if (SUCCEEDED(hr))
				{
					if (destinationWidth == 0)
					{
						float scalar = static_cast<float>(destinationHeight) / static_cast<float>(originalHeight);
						destinationWidth = static_cast<UINT>(scalar*static_cast<float>(originalWidth));
					}
					else if (destinationHeight==0)
					{
						float scalar = static_cast<float>(destinationWidth) / static_cast<float>(originalWidth);
						destinationHeight = static_cast<UINT>(scalar*static_cast<float>(originalHeight));
					}

					hr = pIWICFactory->CreateBitmapScaler(&pScaler);
					if (SUCCEEDED(hr))
					{
						hr = pScaler->Initialize(
							pSource,
							destinationWidth,
							destinationHeight,
							WICBitmapInterpolationModeCubic);

						if (SUCCEEDED(hr))
						{
							hr = pConverter->Initialize(
								pScaler,
								GUID_WICPixelFormat32bppPBGRA,
								WICBitmapDitherTypeNone,
								NULL,
								0.f,
								WICBitmapPaletteTypeMedianCut);
						}
					}
				}
			}
			else
			{
				hr = pConverter->Initialize(
					pSource,
					GUID_WICPixelFormat32bppPBGRA,
					WICBitmapDitherTypeNone,
					NULL,
					0.f,
					WICBitmapPaletteTypeMedianCut);
			}
		}
	}

	if (SUCCEEDED(hr))
	{
		// Create a Direct2D bitmap from the WIC bitmap
		hr = pRenderTarget->CreateBitmapFromWicBitmap(
			pConverter,
			NULL,
			ppBitmap);
	}

	SafeRelease(&pDecoder);
	SafeRelease(&pSource);
	SafeRelease(&pStream);
	SafeRelease(&pConverter);
	SafeRelease(&pScaler);

	return hr;
}
