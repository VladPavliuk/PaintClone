//#include "renderer.h"
#include "ui.h"
#include <stdio.h>

//TODO
// Render bitmap into rect
// Load bmp
// Store bmp
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
	windowData.oneTimeClick = { -1, -1 };
	windowData.isRightButtonHold = false;
	windowData.mousePosition = { 0,0 };
	windowData.selectedColor = { 0,0,0 };
	windowData.selectedTool = DRAW_TOOL::PENCIL;

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

	/*ubyte4* bitmapToCopy = (ubyte4*)malloc(120*120*4);

	bitmapToCopy[0] = ubyte4(120,0,0,0);
	bitmapToCopy[1] = ubyte4(120,0,0,0);
	bitmapToCopy[2] = ubyte4(120,0,0,0);
	bitmapToCopy[3] = ubyte4(120,0,0,0);
	DrawBitmap(&windowData, bitmapToCopy, { 300,300 }, { 120, 120 });*/
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

		if (windowData.isDrawing)
		{
			switch (windowData.selectedTool)
			{
			case DRAW_TOOL::PENCIL:
			{
				//> pencil drawing
				if (windowData.pixelsToDraw.length > 1)
				{
					int2 fromPixel = windowData.pixelsToDraw.get(0);
					int2 toPixel = windowData.pixelsToDraw.get(1);

					DrawLine(&windowData, fromPixel, toPixel, windowData.selectedColor);

					//windowData.pixelsToDraw.remove(1);
					windowData.pixelsToDraw.remove(0);
				}

				//<

				break;
			}
			case DRAW_TOOL::FILL:
			{
				//> filling
				if (windowData.oneTimeClick.x != -1)
				{
					FillFromPixel(&windowData, windowData.oneTimeClick, windowData.selectedColor);
					windowData.oneTimeClick = {-1,-1};
				}
				//<
				break;
			}
			}
		}

		if (!windowData.isRightButtonHold)
		{
			windowData.pixelsToDraw.clean();
		}

		// ui
		DrawColorsBrush(&windowData, &colors, { 5, 5 }, { 15,15 }, 5);

		DrawToolsPanel(&windowData, { 5, 40 }, { 15,15 }, 10);

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