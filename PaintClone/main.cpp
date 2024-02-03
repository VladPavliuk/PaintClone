#include "renderer.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPWSTR pCmd, int windowMode)
{
	WNDCLASS windowClass = {};
	windowClass.hInstance = hInstance;
	windowClass.lpszClassName = L"window class";
	windowClass.lpfnWndProc = WindowCallback;
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);

	RegisterClass(&windowClass);

	WindowData windowData;

	windowData.pixelsToDraw = SimpleDynamicArray<int2>(2);
	windowData.fillFrom = { -1, -1 };

	HWND hwnd = CreateWindowExW(
		0,
		windowClass.lpszClassName,
		L"Paint Clone",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		0, 0,
		hInstance,
		&windowData
	);

	if (hwnd == 0)
	{
		return -1;
	}

	ShowWindow(hwnd, windowMode);
	InitRenderer(&windowData, hwnd);

	FillWindowClientWithWhite(&windowData);

	//DrawLine(&windowData, { 500,500 }, { 100,100 });
	DrawRect(&windowData, 130, 100, 50, 30, { (ubyte)12,(ubyte)12,(ubyte)12 });
	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}

		if (windowData.pixelsToDraw.length > 1)
		{
			int2 fromPixel = windowData.pixelsToDraw.get(0);
			int2 toPixel = windowData.pixelsToDraw.get(1);

			DrawLine(&windowData, fromPixel, toPixel);

			//windowData.pixelsToDraw.remove(1);
			windowData.pixelsToDraw.remove(0);
		}

		if (windowData.fillFrom.x != -1)
		{
			FillFromPixel(&windowData, windowData.fillFrom, { (ubyte)200,0,0 });
			windowData.fillFrom.x = -1;
		}

		// drawing

		StretchDIBits(
			windowData.deviceContext,
			0, 0, windowData.clientSize.x, windowData.clientSize.y,
			0, 0, windowData.clientSize.x, windowData.clientSize.y,
			windowData.bitmap,
			&windowData.bitmapInfo,
			DIB_RGB_COLORS, SRCCOPY
		);
	}

	ReleaseDC(hwnd, windowData.deviceContext);

	return 0;
}