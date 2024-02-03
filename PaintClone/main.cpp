//#include "renderer.h"
#include "ui.h"
#include <stdio.h>

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
	windowData.isRightButtonHold = false;
	windowData.mousePosition = { 0,0 };
	windowData.selectedColor = { 0,0,0 };

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

	SimpleDynamicArray<ubyte3> colors = SimpleDynamicArray<ubyte3>(10);
	colors.add({ 0, 0, 0 });
	colors.add({ 255, 255, 255 });
	colors.add({ 255, 0, 0 });
	colors.add({ 0, 255, 0 });
	colors.add({ 0, 0, 255 });
	colors.add({ 255, 255, 0 });
	colors.add({ 0, 255, 255 });
	colors.add({ 255, 0, 255 });
	//DrawLine(&windowData, { 500,500 }, { 100,100 });
	//DrawRect(&windowData, 0, 0, 50, 30, { (ubyte)12,(ubyte)12,(ubyte)12 });

	//DrawBorderRect(&windowData, {0,0}, { 100,100 }, 2, { 120,120,49 });
	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}

		//> pencil drawing
		if (windowData.pixelsToDraw.length > 1)
		{
			int2 fromPixel = windowData.pixelsToDraw.get(0);
			int2 toPixel = windowData.pixelsToDraw.get(1);

			DrawLine(&windowData, fromPixel, toPixel, windowData.selectedColor);

			//windowData.pixelsToDraw.remove(1);
			windowData.pixelsToDraw.remove(0);
		}

		if (!windowData.isRightButtonHold)
		{
			windowData.pixelsToDraw.clean();
		}
		//<

		//> filling
		if (windowData.fillFrom.x != -1)
		{
			FillFromPixel(&windowData, windowData.fillFrom, { 200,0,0 });
			windowData.fillFrom.x = -1;
		}
		//<

		// ui
		DrawColorsBrush(&windowData, &colors, { 5, 5 },
			{ 15,15 }, 5);
		/*if (DrawButton(&windowData, { 5,5 }, { 15,15 }, { 255,0,0 }, { 255,0,0 }, 1, 1))
		{
			windowData.selectedColor = { 255,0,0 };
		}
		if (DrawButton(&windowData, { 25,5 }, { 15,15 }, { 0,255,0 }, { 0,255,0 }, 1, 2))
		{
			windowData.selectedColor = { 0,255,0 };
		}
		if (DrawButton(&windowData, { 45,5 }, { 15,15 }, { 0,0,255 }, { 0,0,255 }, 1, 3))
		{
			windowData.selectedColor = { 0,0,255 };
		}*/

		//char buff[100];
		////sprintf_s(buff, "x: %i. y: %i\n", windowData.mousePosition.x, windowData.mousePosition.y);
		//sprintf_s(buff, "id: %i\n", windowData.uiIdClicked);
		//OutputDebugStringA(buff);

		StretchDIBits(
			windowData.deviceContext,
			0, 0, windowData.clientSize.x, windowData.clientSize.y,
			0, 0, windowData.clientSize.x, windowData.clientSize.y,
			windowData.bitmap,
			&windowData.bitmapInfo,
			DIB_RGB_COLORS, SRCCOPY
		);
	}

	colors.freeMemory();
	ReleaseDC(hwnd, windowData.deviceContext);

	return 0;
}