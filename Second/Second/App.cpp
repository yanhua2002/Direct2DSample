#include "stdafx.h"
#include "App.h"


App::App() : m_pRenderTarget(NULL)
{
}


App::~App()
{
}

ATOM App::RegisterClass()
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInst;
	wcex.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_SECOND));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_SECOND);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

BOOL App::Init(HINSTANCE instance, int cmd)
{
	hInst = instance;

	// Initialize global strings
	LoadStringW(hInst, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInst, IDC_SECOND, szWindowClass, MAX_LOADSTRING);
	RegisterClass();

	// Perform application initialization
	if (!InitInstance(cmd))
		return FALSE;

	return TRUE;
}

BOOL App::InitInstance(int nCmdShow)
{
	HRESULT hr = CreateDeviceIndependentResources();
	if (FAILED(hr))
		return FALSE;

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInst, this);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

int App::RunMessageLoop()
{
	HACCEL hAccelTable = LoadAccelerators(hInst, MAKEINTRESOURCE(IDC_SECOND));

	// Main message loop
	MSG msg;
	while (GetMessage(&msg,NULL,0,0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return (int)msg.wParam;
}

LRESULT App::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	App* pApp;
	if (message == WM_CREATE)
	{
		LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
		pApp = (App*)pcs->lpCreateParams;

		::SetWindowLongPtrW(hWnd, GWLP_USERDATA, PtrToUlong(pApp));

		CreateWindow(L"button", L"Clear", WS_VISIBLE | WS_CHILD, 20, 50, 80, 25,
			hWnd, (HMENU)IDB_CLEAR, NULL, NULL);
		return TRUE;
	}
	else
	{
		pApp = reinterpret_cast<App*>(static_cast<LONG_PTR>(::GetWindowLongPtrW(hWnd, GWLP_USERDATA)));
		if (!pApp)
			return DefWindowProc(hWnd, message, wParam, lParam);
	}

	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	switch (message)
	{
		case WM_COMMAND:
			{
				wmId = LOWORD(wParam);
				wmEvent = HIWORD(wParam);
				// Parse the menu selections
				switch (wmId)
				{
					case IDM_ABOUT:
						DialogBox(pApp->getInstance(), MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, pApp->About);
						break;
					case IDM_EXIT:
						DestroyWindow(hWnd);
						break;
					case IDB_CLEAR:
						{
							int mbResult = MessageBox(hWnd, L"Are you sure you want to clear all the points?",
								L"Clear Points", MB_OKCANCEL | MB_ICONWARNING);
							if (mbResult == IDOK)
							{
								pApp->ClearPoints();
								InvalidateRect(hWnd, NULL, true);
							}
						}
						break;
					default:
						return DefWindowProc(hWnd, message, wParam, lParam);
				}
			}
			break;
		case WM_PAINT:
			{
				hdc = BeginPaint(hWnd, &ps);
				pApp->OnRender(hWnd);
				EndPaint(hWnd, &ps);
			}
			break;
		case WM_SIZE:
			{
				UINT width = LOWORD(lParam);
				UINT height = HIWORD(lParam);
				pApp->OnResize(width, height);
			}
			break;
		case WM_LBUTTONDOWN:
			{
				int x = LOWORD(lParam);
				int y = HIWORD(lParam);
				pApp->AddPoint((float)x / pApp->getXScale(), (float)y / pApp->getYScale());
				InvalidateRect(hWnd, NULL, true);
			}
			break;
		case WM_CHAR:
			{
				char c = (TCHAR)wParam;
				if (c == 'c')
				{
					pApp->ClearPoints();
					InvalidateRect(hWnd, NULL, true);
				}
			}
			break;
		case WM_DISPLAYCHANGE:
			pApp->OnRender(hWnd);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

INT_PTR App::About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

void App::AddPoint(float x, float y)
{
	points.push_back(std::pair<float, float>(x, y));
}

void App::ClearPoints()
{
	points.clear();
}

HRESULT App::CreateDeviceIndependentResources()
{
	HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);
	if (SUCCEEDED(hr))
	{
		float dpiX, dpiY;
		m_pD2DFactory->GetDesktopDpi(&dpiX, &dpiY);
		xScale = dpiX / 96.0f;
		yScale = dpiY / 96.0f;
	}
	//if (SUCCEEDED(hr))
	//{
	//	// Create an ellipse geometry
	//	D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F(105.0f, 105.0f), 25.0f, 25.0f);
	//	hr = m_pD2DFactory->CreateEllipseGeometry(ellipse, &m_pEllipseGeometry);
	//}
	return hr;
}

HRESULT App::CreateDeviceResources(HWND hwnd)
{
	HRESULT hr = S_OK;
	if (!m_pRenderTarget)
	{
		RECT rc;
		GetClientRect(hwnd, &rc);
		D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

		// Create a Direct2D render target
		hr = m_pD2DFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(hwnd, size),
			&m_pRenderTarget);
		if (SUCCEEDED(hr))
		{
			// Create a black brush
			hr = m_pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::Black),
				&m_pBlackBrush);
		}
	}
	return hr;
}

HRESULT App::OnRender(HWND hwnd)
{
	HRESULT hr = CreateDeviceResources(hwnd);
	if (SUCCEEDED(hr))
	{
		if (!(m_pRenderTarget->CheckWindowState()&D2D1_WINDOW_STATE_OCCLUDED))
		{
			m_pRenderTarget->BeginDraw();
			m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));
			//m_pRenderTarget->FillGeometry(m_pEllipseGeometry, m_pBlackBrush);
			for (std::vector<std::pair<float,float>>::iterator i = points.begin(); i < points.end(); i++)
			{
				D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F(i->first, i->second), 25.f, 25.f);
				m_pRenderTarget->FillEllipse(ellipse, m_pBlackBrush);
			}
			hr = m_pRenderTarget->EndDraw();

			if (hr == D2DERR_RECREATE_TARGET)
			{
				hr = S_OK;
				m_pRenderTarget->Release();
				m_pRenderTarget = NULL;
				m_pBlackBrush->Release();
				m_pBlackBrush = NULL;
			}
		}
	}
	return hr;
}

void App::OnResize(UINT width, UINT height)
{
	if (m_pRenderTarget)
	{
		D2D1_SIZE_U size;
		size.width = width;
		size.height = height;
		m_pRenderTarget->Resize(size);
	}
}
