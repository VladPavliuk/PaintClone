//#include "renderer.h"
#include "ui.h"
#include <stdio.h>

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
	case WM_RBUTTONUP:
	{
		/*WindowData* windowData = (WindowData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

		int xMouse = GET_X_LPARAM(lParam);
		int yMouse = GET_Y_LPARAM(lParam);

		yMouse = windowData->clientSize.y - yMouse;

		windowData->fillFrom = { xMouse, yMouse };*/
		break;
	}
	case WM_LBUTTONUP:
	{
		WindowData* windowData = (WindowData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

		windowData->isRightButtonHold = false;
		windowData->isDrawing = false;
		break;
	}
	case WM_LBUTTONDOWN:
	{
		WindowData* windowData = (WindowData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

		windowData->isRightButtonHold = true;

		ubyte2 zAndId = *(windowData->zAndIdBuffer + (windowData->mousePosition.x + windowData->clientSize.x * windowData->mousePosition.y));

		// NOTE: only drawing canvas has zero id in z buffer, so only when we have initial click on actual canvas, we allow drawing
		if (zAndId.y == 0)
		{
			windowData->isDrawing = true;

			switch (windowData->selectedTool)
			{
			case DRAW_TOOL::FILL:
			{
				windowData->oneTimeClick = windowData->mousePosition;
				break;
			}
			}
		}
		break;
	}
	case WM_PAINT:
	{
		/*WindowData* windowData = (WindowData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

		if (windowData->colorsInBrush.length > 0)
		{
			DrawColorsBrush(windowData, &windowData->colorsInBrush, { 5, 5 }, { 15, 15 }, 5);

			DrawToolsPanel(windowData, { 5, 30 }, { 15, 15 }, 5);
		}

		StretchDIBits(
			windowData->deviceContext,
			0, 0, windowData->clientSize.x, windowData->clientSize.y,
			0, 0, windowData->clientSize.x, windowData->clientSize.y,
			windowData->bitmap,
			&windowData->bitmapInfo,
			DIB_RGB_COLORS, SRCCOPY
		);*/

		break;
	}
	case WM_MOUSEMOVE:
	{
		WindowData* windowData = (WindowData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

		int xMouse = GET_X_LPARAM(lParam);
		int yMouse = GET_Y_LPARAM(lParam);

		yMouse = windowData->clientSize.y - yMouse;

		windowData->mousePosition = { xMouse, yMouse };

		if (wParam == MK_LBUTTON && windowData->isDrawing)
		{
			switch (windowData->selectedTool)
			{
			case DRAW_TOOL::PENCIL:
			{
				windowData->pixelsToDraw.add(windowData->mousePosition);
				break;
			}
			}
		}

		break;
	}
	case WM_SIZE:
	{
		WindowData* windowData = (WindowData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

		RECT clientRect;
		GetClientRect(hwnd, &clientRect);
		windowData->clientSize.x = clientRect.right - clientRect.left;
		windowData->clientSize.y = clientRect.bottom - clientRect.top;

		if (windowData->uiBitmap)
		{
			/*free(windowData->bitmap);
			free(windowData->zAndIdBuffer);*/
			windowData->uiBitmap = (ubyte4*)realloc(windowData->uiBitmap, 4 * windowData->clientSize.x * windowData->clientSize.y);

			windowData->zAndIdBuffer = (ubyte2*)realloc(windowData->zAndIdBuffer, 2 * windowData->clientSize.x * windowData->clientSize.y);
			ZeroMemory(windowData->zAndIdBuffer, 2 * windowData->clientSize.x * windowData->clientSize.y);

		}
		else
		{
			windowData->uiBitmap = (ubyte4*)malloc(4 * windowData->clientSize.x * windowData->clientSize.y);

			windowData->zAndIdBuffer = (ubyte2*)malloc(2 * windowData->clientSize.x * windowData->clientSize.y);
			ZeroMemory(windowData->zAndIdBuffer, 2 * windowData->clientSize.x * windowData->clientSize.y);
		}

		windowData->bitmapInfo.bmiHeader.biWidth = windowData->clientSize.x;
		windowData->bitmapInfo.bmiHeader.biHeight = windowData->clientSize.y;

		if (windowData->mousePosition.x >= windowData->clientSize.x)
		{
			windowData->mousePosition.x = windowData->mousePosition.x;
		}
		if (windowData->mousePosition.y >= windowData->clientSize.y)
		{
			windowData->mousePosition.y = windowData->mousePosition.y;
		}

		if (windowData->colorsInBrush.length > 0)
		{
			DrawColorsBrush(windowData, &windowData->colorsInBrush, { 5, 5 }, { 15, 15 }, 5);

			DrawToolsPanel(windowData, { 5, 30 }, { 15, 15 }, 5);
		}

		StretchDIBits(
			windowData->deviceContext,
			0, 0, windowData->clientSize.x, windowData->clientSize.y,
			0, 0, windowData->clientSize.x, windowData->clientSize.y,
			windowData->uiBitmap,
			&windowData->bitmapInfo,
			DIB_RGB_COLORS, SRCCOPY
		);
		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}


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

	windowData.uiBitmap = NULL;
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

	ShowWindow(hwnd, windowMode);
	UpdateWindow(hwnd);
	InitRenderer(&windowData, hwnd);

	FillWindowClientWithWhite(&windowData);

	windowData.toolsImages = SimpleDynamicArray<BmpImage>(10);
	windowData.toolsImages.set((int)DRAW_TOOL::PENCIL, LoadBmpFile(L"./pencil.bmp"));
	windowData.toolsImages.set((int)DRAW_TOOL::FILL, LoadBmpFile(L"./fill.bmp"));

	//BmpImage bmpTestImage = LoadBmpFile(L"./pencil.bmp");
	//BmpImage bmpTestImage = LoadBmpFile(L"./fill.bmp");
	//BmpImage bmpTestImage = LoadBmpFile(L"./testing.bmp");
	//BmpImage bmpTestImage = LoadBmpFile(L"./testing_2.bmp");
	//BmpImage bmpTestImage = LoadBmpFile(L"./testing_3.bmp");

	windowData.colorsInBrush = SimpleDynamicArray<ubyte3>(10);
	windowData.colorsInBrush.add({ 0, 0, 0 });
	windowData.colorsInBrush.add({ 255, 255, 255 });
	windowData.colorsInBrush.add({ 255, 0, 0 });
	windowData.colorsInBrush.add({ 0, 255, 0 });
	windowData.colorsInBrush.add({ 0, 0, 255 });
	windowData.colorsInBrush.add({ 255, 255, 0 });
	windowData.colorsInBrush.add({ 0, 255, 255 });
	windowData.colorsInBrush.add({ 255, 0, 255 });
	//DrawLine(&windowData, { 500,500 }, { 100,100 });
	//DrawRect(&windowData, 0, 0, 50, 30, { (ubyte)12,(ubyte)12,(ubyte)12 });

	//DrawBitmap(&windowData, bmpTestImage.rgbaBitmap, { 300,300 }, bmpTestImage.size);
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
					windowData.oneTimeClick = { -1,-1 };
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
		DrawColorsBrush(&windowData, &windowData.colorsInBrush, { 5, 5 }, { 15, 15 }, 5);

		DrawToolsPanel(&windowData, { 5, 30 }, { 15, 15 }, 5);

		//char buff[100];
		////sprintf_s(buff, "x: %i. y: %i\n", windowData.mousePosition.x, windowData.mousePosition.y);
		//sprintf_s(buff, "id: %i\n", windowData.uiIdClicked);
		//OutputDebugStringA(buff);

		StretchDIBits(
			windowData.deviceContext,
			0, 0, windowData.clientSize.x, windowData.clientSize.y,
			0, 0, windowData.clientSize.x, windowData.clientSize.y,
			windowData.uiBitmap,
			&windowData.bitmapInfo,
			DIB_RGB_COLORS, SRCCOPY
		);

	}

	windowData.colorsInBrush.freeMemory();
	//windowData.toolsImages.freeMemory();
	ReleaseDC(hwnd, windowData.deviceContext);

	return 0;
}