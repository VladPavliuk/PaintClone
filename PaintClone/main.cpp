#include "ui.h"

#include <stdio.h>
#include <dwmapi.h>

LRESULT WINAPI WindowCallback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_NCCREATE:
	{
		WindowData* windowData = (WindowData*)((CREATESTRUCTW*)lParam)->lpCreateParams;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)windowData);

		break;
	}
	case WM_LBUTTONUP:
	{
		WindowData* windowData = (WindowData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

		windowData->isRightButtonHold = false;
		windowData->wasRightButtonReleased = true;
		break;
	}
	case WM_LBUTTONDOWN:
	{
		WindowData* windowData = (WindowData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

		windowData->isRightButtonHold = true;
		windowData->wasRightButtonPressed = true;
		
		break;
	}
	case WM_KEYDOWN:
	{
		WindowData* windowData = (WindowData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

		int offsetPerMove = (int)(100.0f / (float)windowData->drawingZoomLevel);

		if (VK_UP == wParam)
		{
			windowData->drawingOffset.y += offsetPerMove;
		}

		if (VK_DOWN == wParam)
		{
			windowData->drawingOffset.y -= offsetPerMove;
		}

		if (VK_RIGHT == wParam)
		{
			windowData->drawingOffset.x += offsetPerMove;
		}

		if (VK_LEFT == wParam)
		{
			windowData->drawingOffset.x -= offsetPerMove;

		}

		ValidateDrawingOffset(windowData);
		break;
	}
	case WM_MOUSEMOVE:
	{
		WindowData* windowData = (WindowData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

		int xMouse = GET_X_LPARAM(lParam);
		int yMouse = GET_Y_LPARAM(lParam);

		yMouse = windowData->windowClientSize.y - yMouse;

		windowData->mousePosition = { xMouse, yMouse };
		break;
	}
	case WM_SIZE:
	{
		WindowData* windowData = (WindowData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

		// NOTE: after window is create WM_SIZE message is recevied, since we init rendering after, we should ignore the first call
		if (windowData->windowDC == 0)
		{
			break;
		}
		RECT clientRect;
		GetClientRect(hwnd, &clientRect);
		windowData->windowClientSize.x = clientRect.right - clientRect.left;
		windowData->windowClientSize.y = clientRect.bottom - clientRect.top;

		windowData->windowBitmapInfo.bmiHeader.biWidth = windowData->windowClientSize.x;
		windowData->windowBitmapInfo.bmiHeader.biHeight = windowData->windowClientSize.y;

		RecreateBackgroundBmp(windowData);
		CalculateDrawingZone(windowData);
		ValidateDrawingOffset(windowData);

		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

//TODO
// +Render bitmap into rect
// +Load bmp
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
	InitWindowData(&windowData);

	QueryPerformanceFrequency(&windowData.perfomanceCounterFreq);

	HWND hwnd = CreateWindowExW(
		0,
		windowClass.lpszClassName,
		L"Paint Clone",
		WS_OVERLAPPEDWINDOW /* | CS_VREDRAW | CS_HREDRAW*/,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		0, 0,
		hInstance,
		&windowData
	);

	if (hwnd == 0)
	{
		return -1;
	}

	//> Remove fade in animation when window is opened up
	BOOL attrib = TRUE;
	DwmSetWindowAttribute(hwnd, DWMWA_TRANSITIONS_FORCEDISABLED, &attrib, sizeof(attrib));
	//<

	ShowWindow(hwnd, windowMode);

	InitRenderer(&windowData, hwnd);

	windowData.toolTiles = SimpleDynamicArray<ToolTile>(10);
	windowData.toolTiles.add(ToolTile(UI_ELEMENT::PENCIL_TOOL, DRAW_TOOL::PENCIL, LoadBmpFile(L"./pencil.bmp")));
	windowData.toolTiles.add(ToolTile(UI_ELEMENT::FILL_TOOL, DRAW_TOOL::FILL, LoadBmpFile(L"./fill.bmp")));
	windowData.toolTiles.add(ToolTile(UI_ELEMENT::ZOOM_TOOL, DRAW_TOOL::ZOOM, LoadBmpFile(L"./zoom.bmp")));

	windowData.brushColorTiles = SimpleDynamicArray<BrushColorTile>(10);
	windowData.brushColorTiles.add(BrushColorTile({ 0, 0, 0 }, UI_ELEMENT::COLOR_BRUCH_1));
	windowData.brushColorTiles.add(BrushColorTile({ 255, 255, 255 }, UI_ELEMENT::COLOR_BRUCH_2));
	windowData.brushColorTiles.add(BrushColorTile({ 255, 0, 0 }, UI_ELEMENT::COLOR_BRUCH_3));
	windowData.brushColorTiles.add(BrushColorTile({ 0, 255, 0 }, UI_ELEMENT::COLOR_BRUCH_4));
	windowData.brushColorTiles.add(BrushColorTile({ 0, 0, 255 }, UI_ELEMENT::COLOR_BRUCH_5));
	windowData.brushColorTiles.add(BrushColorTile({ 255, 255, 0 }, UI_ELEMENT::COLOR_BRUCH_6));
	windowData.brushColorTiles.add(BrushColorTile({ 0, 255, 255 }, UI_ELEMENT::COLOR_BRUCH_7));
	windowData.brushColorTiles.add(BrushColorTile({ 255, 0, 255 }, UI_ELEMENT::COLOR_BRUCH_8));
	//DrawLine(&windowData, { 500,500 }, { 100,100 });
	//DrawRect(&windowData, 0, 0, 50, 30, { (ubyte)12,(ubyte)12,(ubyte)12 });

	//DrawBitmap(&windowData, bmpTestImage.rgbaBitmap, { 300,300 }, bmpTestImage.size);
	//DrawBorderRect(&windowData, {0,0}, { 100,100 }, 2, { 120,120,49 });
	//FillFromPixel(&windowData, { 10,10 }, { 255, 0, 255 });
	//DrawBorderRect(&windowData, { 500, 10 }, { 100, 10 }, 4, { 0,0,0 });
	double deltasSum = 0.0f;
	int framesCount = 1;
	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}

		//POINT mousePosition;
		//GetCursorPos(&mousePosition);
		//ScreenToClient(hwnd, &mousePosition);
		//mousePosition.y = windowData.windowClientSize.y - mousePosition.y;
		//windowData.mousePosition = { mousePosition.x, mousePosition.y };

		double timeDelta = GetCurrentTimestamp(&windowData);

		FillWindowClientWithWhite(windowData.windowBitmap, windowData.windowClientSize);

		timeDelta = GetCurrentTimestamp(&windowData) - timeDelta;

		char buff[100];
		sprintf_s(buff, "time: %f\n", timeDelta);
		//OutputDebugStringA(buff);
		
		// ui
		DrawColorsBrush(&windowData, &windowData.brushColorTiles, { 5, 5 }, { 15, 15 }, 5);

		DrawToolsPanel(&windowData, { 5, 30 }, { 15, 15 }, 5);

		StretchDIBits(
			windowData.backgroundDC,
			windowData.drawingZonePosition.x, windowData.windowClientSize.y - windowData.drawingZonePosition.y - windowData.drawingZoneSize.y,
			windowData.drawingZoneSize.x, windowData.drawingZoneSize.y,

			(int)((float)windowData.drawingOffset.x / (float)windowData.drawingZoomLevel), (int)((float)windowData.drawingOffset.y / (float)windowData.drawingZoomLevel),
			(int)((float)windowData.drawingZoneSize.x / (float)windowData.drawingZoomLevel), (int)((float)windowData.drawingZoneSize.y / (float)windowData.drawingZoomLevel),

			windowData.drawingBitmap,
			&windowData.drawingBitmapInfo,
			DIB_RGB_COLORS, SRCCOPY
		);

		DrawDrawingCanvas(&windowData);

		// NOTE: when charger is not connected to the laptop, it has around 10x slower performance!
		BitBlt(windowData.windowDC,
			0, 0, windowData.windowClientSize.x, windowData.windowClientSize.y,
			windowData.backgroundDC, 0, 0, SRCCOPY);
		deltasSum += timeDelta;
		framesCount++;

		// NOTE: at the end of frame we should clean mouse buttons state
		// otherwise we might get overspaming behaviour
		windowData.wasRightButtonPressed = false;
		windowData.wasRightButtonReleased = false;

		HandleUiElements(&windowData);
		windowData.prevMousePosition = windowData.mousePosition;
		windowData.prevHotUi = windowData.hotUi;
	}

	double averageDelta = deltasSum / framesCount;

	DeleteDC(windowData.backgroundDC);
	ReleaseDC(hwnd, windowData.windowDC);
	//SelectObject(windowData.backgroundDC, objectHandler);
	windowData.brushColorTiles.freeMemory();
	windowData.toolTiles.freeMemory();

	return 0;
}