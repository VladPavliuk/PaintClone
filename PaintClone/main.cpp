#include "ui.h"

#include <stdio.h>
#include <dwmapi.h>
#include "fonts.h"
#include "hash_table.h"

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

		// NOTE: We have to release previously capture, because we won't be able to use windws default buttons on the window
		ReleaseCapture();
		break;
	}
	case WM_LBUTTONDOWN:
	{
		WindowData* windowData = (WindowData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

		windowData->isRightButtonHold = true;
		windowData->wasRightButtonPressed = true;

		SetCapture(hwnd);
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
	case WM_MOUSELEAVE:
	{
		WindowData* windowData = (WindowData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

		windowData->prevHotUi = UI_ELEMENT::NONE;
		windowData->hotUi = UI_ELEMENT::NONE;
		windowData->activeUiOffset = { -1,-1 };
		// NOTE: we don't reset active ui after the mouse left the window
		// because it would prevent us from having some behaviour, like holding a slider and move it even if the mouse is outside of the window
		//windowData->activeUi = UI_ELEMENT::NONE;
		windowData->sumbitedUi = UI_ELEMENT::NONE;
		windowData->sumbitedOnAnyHotUi = UI_ELEMENT::NONE;
		break;
	}
	case WM_MOUSEMOVE:
	{
		//> It's required for WM_MOUSELEAVE
		TRACKMOUSEEVENT trackMouseEvent;

		trackMouseEvent.cbSize = sizeof(TRACKMOUSEEVENT);
		trackMouseEvent.hwndTrack = hwnd;
		trackMouseEvent.dwHoverTime = 0;
		trackMouseEvent.dwFlags = TME_LEAVE;

		TrackMouseEvent(&trackMouseEvent);
		//<

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
		CalculateDrawingZoneSize(windowData);
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

	//> testing fonts
	//wchar_t alphabetStr[] = L"АБВГҐДЕЄЖЗИІЇЙКЛМНОПРСТУФХЦЧШЩЬЮЯабвгґдеєжзиіїйклмнопрстуфхцчшщьюя !\"#$%&'()*+,-./0123456789:;<=>?@[\\]^_`{|}~ABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnoprstuvwxyz";
	wchar_t alphabetStr[] = L"!\"#$%&'()*+,-./0123456789:;<=>?@[\\]^_`{|}~ABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnoprstuvwxyz";
	//wchar_t alphabetStr[] = L"ABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnoprstuvwxyz";
	//wchar_t alphabetStr[] = L"~";
	
	SimpleDynamicArray<wchar_t> alphabet = SimpleDynamicArray<wchar_t>(10);
	for (int i = 0; i < wcslen(alphabetStr); i++)
	{
		alphabet.add(alphabetStr[i]);
	}

	double timeDelta2 = GetCurrentTimestamp(&windowData);
	HashTable<FontGlyph> glyphs = ReadGlyphsFromTTF(L"C:\\Windows\\Fonts\\arial.ttf", &alphabet);
	//HashTable<FontGlyph> glyphs = ReadGlyphsFromTTF(L"C:\\Windows\\Fonts\\calibri.ttf", &alphabet);
	//HashTable<FontGlyph> glyphs = ReadGlyphsFromTTF(L"C:\\Windows\\Fonts\\Candarai.ttf", &alphabet);
	//HashTable<FontGlyph> glyphs = ReadGlyphsFromTTF(L"C:\\Windows\\Fonts\\corbel.ttf", &alphabet);
	//HashTable<FontGlyph> glyphs = ReadGlyphsFromTTF(L"C:\\Windows\\Fonts\\comicbd.ttf", &alphabet);
	//HashTable<FontGlyph> glyphs = ReadGlyphsFromTTF(L"C:\\Windows\\Fonts\\segoeuiz.ttf", &alphabet);
	//HashTable<FontGlyph> glyphs = ReadGlyphsFromTTF(L"Envy Code R.ttf", &alphabet);
	//HashTable<FontGlyph> glyphs = ReadGlyphsFromTTF(L"ShadeBlue-2OozX.ttf", &alphabet);
	timeDelta2 = GetCurrentTimestamp(&windowData) - timeDelta2;
	char buff[100];
	sprintf_s(buff, "frame time: %f ml sec.\n", timeDelta2 * 1000.0f);
	OutputDebugStringA(buff);

	HashTable<RasterizedGlyph> rasterizedGlyphs = RasterizeFontGlyphs(&glyphs);

	RasterizedGlyph glyph = rasterizedGlyphs.get(L'~');

	CopyBitmapToBitmap(glyph.bitmap, glyph.bitmapSize, windowData.drawingBitmap, { 10,10 }, windowData.drawingBitmapSize);
	
	//>testing draiwing points
	//for (int i = 0; i < glyph.contours.length; i++)
	//{
	//	SimpleDynamicArray<int2> points = glyph.contours.get(i);
	//	for (int j = 0; j < points.length - 1; j++)
	//	{
	//		int2 currentPoint = points.get(j);
	//		int2 nextPoint = points.get(j + 1);
	//		//int2 nextPoint = glyph.points.get((i + 1) % (glyph.points.length - 1));

	//		/*currentPoint.x = currentPoint.x / 5.0f;
	//		currentPoint.y = currentPoint.y / 5.0f;*/

	//		currentPoint.x += 100;
	//		currentPoint.y += 100;

	//		//nextPoint.x = nextPoint.x / 5.0f;
	//		//nextPoint.y = nextPoint.y / 5.0f;

	//		nextPoint.x += 100;
	//		nextPoint.y += 100;

	//		int4 drawingRect;
	//		drawingRect.x = 0;
	//		drawingRect.y = 0;
	//		drawingRect.z = windowData.drawingZone.size.x;
	//		drawingRect.w = windowData.drawingZone.size.y;

	//		currentPoint = ConvertFromScreenToDrawingCoords(&windowData, currentPoint);
	//		nextPoint = ConvertFromScreenToDrawingCoords(&windowData, nextPoint);

	//		//ubyte testColor = j * 255.0f / points.length;
	//		ubyte testColor = 0;
	//		DrawLine(windowData.drawingBitmap, windowData.drawingBitmapSize, drawingRect, currentPoint, nextPoint, { testColor,testColor,testColor });
	//	}
	//}
	//<

	//ReadGlyphsFromTTF(L"Envy Code R.ttf", &alphabet);
	//<

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

		/*POINT mousePosition;
		GetCursorPos(&mousePosition);
		ScreenToClient(hwnd, &mousePosition);
		mousePosition.y = windowData.windowClientSize.y - mousePosition.y;
		windowData.mousePosition = { mousePosition.x, mousePosition.y };*/

		double timeDelta = GetCurrentTimestamp(&windowData);

		FillBitmapWithWhite(windowData.windowBitmap, windowData.windowClientSize);

		// ui
		DrawColorsBrush(&windowData, &windowData.brushColorTiles, { 5, 5 }, { 15, 15 }, 5);
		DrawToolsPanel(&windowData, { 5, 30 }, { 15, 15 }, 5);

		StretchDIBits(
			windowData.backgroundDC,
			windowData.drawingZone.x, windowData.windowClientSize.y - windowData.drawingZone.y - windowData.drawingZone.size.y,
			windowData.drawingZone.size.x, windowData.drawingZone.size.y,

			(int)((float)windowData.drawingOffset.x / (float)windowData.drawingZoomLevel), (int)((float)windowData.drawingOffset.y / (float)windowData.drawingZoomLevel),
			(int)((float)windowData.drawingZone.size.x / (float)windowData.drawingZoomLevel), (int)((float)windowData.drawingZone.size.y / (float)windowData.drawingZoomLevel),

			windowData.drawingBitmap,
			&windowData.drawingBitmapInfo,
			DIB_RGB_COLORS, SRCCOPY
		);

		DrawDrawingCanvas(&windowData);
		DrawDraggableCornerOfDrawingZone(&windowData);

		// NOTE: when charger is not connected to the laptop, it has around 10x slower performance!
		BitBlt(windowData.windowDC,
			0, 0, windowData.windowClientSize.x, windowData.windowClientSize.y,
			windowData.backgroundDC, 0, 0, SRCCOPY);

		// NOTE: at the end of frame we should clean mouse buttons state
		// otherwise we might get overspaming behaviour
		windowData.wasRightButtonPressed = false;
		windowData.wasRightButtonReleased = false;

		HandleUiElements(&windowData);
		windowData.prevMousePosition = windowData.mousePosition;
		windowData.prevHotUi = windowData.hotUi;
		windowData.hotUi = UI_ELEMENT::NONE;

		timeDelta = GetCurrentTimestamp(&windowData) - timeDelta;
		char buff[100];
		sprintf_s(buff, "frame time: %f ml sec.\n", timeDelta * 1000.0f);
		OutputDebugStringA(buff);
		deltasSum += timeDelta;
		framesCount++;
	}

	// 08.02.2024 average frame time - 2.21 ml sec. without charger connected to the laptop, 0.34 ml sec. with charger connected to the laptop
	double averageDelta = deltasSum / framesCount;
	char buff2[100];
	sprintf_s(buff2, "AVERAGE FRAME TIME: %f ml sec.\n", averageDelta * 1000.0f);
	OutputDebugStringA(buff2);

	DeleteDC(windowData.backgroundDC);
	ReleaseDC(hwnd, windowData.windowDC);
	//SelectObject(windowData.backgroundDC, objectHandler);
	windowData.brushColorTiles.freeMemory();
	windowData.toolTiles.freeMemory();

	return 0;
}