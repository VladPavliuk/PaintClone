#include "dialog_window.h"

LRESULT WINAPI ChildWindowCallback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_NCCREATE:
	{
		WindowData* windowData = (WindowData*)((CREATESTRUCTW*)lParam)->lpCreateParams;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)windowData);

		break;
	}
	case WM_SYSCOMMAND:
	{
		//> NOTE: We should enable parent window right before child window is closed
		// otherwise parent window is hidden bellow other windows
		WindowData* windowData = (WindowData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

		if (SC_CLOSE == wParam)
		{
			EnableWindow(windowData->parentHwnd, TRUE);
		}
		//<

		break;
	}
	case WM_LBUTTONDBLCLK:
	{
		WindowData* windowData = (WindowData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

		/*int xMouse = GET_X_LPARAM(lParam);
		int yMouse = GET_Y_LPARAM(lParam);*/

		windowData->wasMouseDoubleClick = true;

		break;
	}
	case WM_MOUSEMOVE:
	{
		WindowData* windowData = (WindowData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

		int xMouse = GET_X_LPARAM(lParam);
		int yMouse = GET_Y_LPARAM(lParam);

		yMouse = windowData->dialogBitmap.size.y - yMouse;

		windowData->mousePosition = { xMouse, yMouse };

		break;
	}
	case WM_DESTROY:
	{
		WindowData* windowData = (WindowData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

		windowData->dialogType = DialogWindowType::NONE;
		windowData->dialogDC = 0;
		free(windowData->dialogBitmap.pixels);

		// clear data related to specific dialog type
		windowData->selectedColorBrushForColorPicker = UI_ELEMENT::NONE;
		break;
	}
	default:
		break;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

void ShowDialogWindow(WindowData* windowData, DialogWindowType dialogWindowType)
{
	//TODO: temporary solution
	windowData->wasRightButtonPressed = false;
	windowData->wasRightButtonReleased = false;
	windowData->wasMouseDoubleClick = false;
	windowData->mousePositionChanged = false;

	const wchar_t* childClassName = L"DialogClass";
	WNDCLASSEX s;
	if (!GetClassInfoEx(windowData->hInstance, childClassName, &s))
	{
		WNDCLASS childWc = {};
		childWc.lpfnWndProc = (WNDPROC)ChildWindowCallback;
		childWc.style = CS_DBLCLKS; // CS_DBLCLKS enables usage of WM_LBUTTONDBLCLK
		childWc.hInstance = windowData->hInstance;
		childWc.lpszClassName = childClassName;
		RegisterClass(&childWc);
	}

	int2 childWindowSize = { -1, -1 };
	WideString dialogTitle;
	switch (dialogWindowType)
	{
	case DialogWindowType::COLOR_PICKER:
	{
		dialogTitle = WideString(L"Color picker");
		childWindowSize = { 300, 300 };
		break;
	}
	default:
		assert(false);
		break;
	}


	int2 parentWindowSize = windowData->windowRect.size();

	int2 childWindowPosition = {
		windowData->windowRect.x + parentWindowSize.x / 2.0f - childWindowSize.x / 2.0f,
		windowData->windowRect.y + parentWindowSize.y / 2.0f - childWindowSize.y / 2.0f
	};

	//TODO: it's possible to create child window outside of screen boundaries, make sure that it's always IN screen boundaries

	EnableWindow(windowData->parentHwnd, FALSE);
	HWND childHwnd = CreateWindowExW(WS_EX_DLGMODALFRAME, childClassName, dialogTitle.chars,
		WS_SYSMENU | WS_CAPTION, childWindowPosition.x, childWindowPosition.y, childWindowSize.x, childWindowSize.y,
		windowData->parentHwnd, NULL, windowData->hInstance, windowData);
	dialogTitle.freeMemory();

	//> Remove fade in animation when window is opened up
	BOOL attrib = TRUE;
	DwmSetWindowAttribute(childHwnd, DWMWA_TRANSITIONS_FORCEDISABLED, &attrib, sizeof(attrib));
	//<

	ShowWindow(childHwnd, SW_SHOWDEFAULT);

	if (childHwnd == 0)
	{
		assert(false);
	}

	RECT clientRect;
	GetClientRect(childHwnd, &clientRect);
	windowData->dialogBitmap.size.x = clientRect.right - clientRect.left;
	windowData->dialogBitmap.size.y = clientRect.bottom - clientRect.top;

	windowData->dialogDC = GetDC(childHwnd);

	windowData->dialogBitmapInfo.bmiHeader.biSize = sizeof(windowData->dialogBitmapInfo.bmiHeader);
	windowData->dialogBitmapInfo.bmiHeader.biWidth = windowData->dialogBitmap.size.x;
	windowData->dialogBitmapInfo.bmiHeader.biHeight = windowData->dialogBitmap.size.y;
	windowData->dialogBitmapInfo.bmiHeader.biPlanes = 1;
	windowData->dialogBitmapInfo.bmiHeader.biBitCount = 32;
	windowData->dialogBitmapInfo.bmiHeader.biCompression = BI_RGB;
	windowData->dialogBitmapInfo.bmiHeader.biSizeImage = 0;
	windowData->dialogBitmapInfo.bmiHeader.biXPelsPerMeter = 0;
	windowData->dialogBitmapInfo.bmiHeader.biYPelsPerMeter = 0;
	windowData->dialogBitmapInfo.bmiHeader.biClrUsed = 0;
	windowData->dialogBitmapInfo.bmiHeader.biClrImportant = 0;

	windowData->dialogBitmap.pixels = (ubyte4*)malloc(4 * windowData->dialogBitmap.size.x * windowData->dialogBitmap.size.y);
	// TODO: use win32 styles for window creation, to create default window color
	ZeroMemory(windowData->dialogBitmap.pixels, 4 * windowData->dialogBitmap.size.x * windowData->dialogBitmap.size.y);
	//FillBitmapWithWhite(windowData->canvasBitmap.pixels, windowData->canvasBitmap.size);

	windowData->dialogHwnd = childHwnd;
	windowData->dialogType = dialogWindowType;
}

float _colorChanelFunction(float x)
{
	// y = max(min(-|x - 3| + 2, 1), 0)
	return maxFloat(minFloat(-absFloat(x - 3.0f) + 2.0f, 1.0f), 0.0f);
	//return maxFloat(minFloat(-absFloat(x - 0.5f) + 0.333333f, 0.166666f), 0.0f);
}

ubyte3 GetColorFromColorPicker(int2 mousePosition, int2 colorPickerSize)
{
	const float functionSegmentsCount = 6.0f;
	float xPoint = functionSegmentsCount * (float)mousePosition.x / (float)colorPickerSize.x;

	float xPointShiftedToRed = xPoint + 3.0f; // red
	float xPointShiftedToGreen = xPoint + 1.0f; // green
	float xPointShiftedToBlue = xPoint + 5.0f; // blue

	if (xPointShiftedToRed >= functionSegmentsCount) xPointShiftedToRed -= functionSegmentsCount;
	if (xPointShiftedToGreen >= functionSegmentsCount) xPointShiftedToGreen -= functionSegmentsCount;
	if (xPointShiftedToBlue >= functionSegmentsCount) xPointShiftedToBlue -= functionSegmentsCount;

	ubyte r = (ubyte)(255.0f * _colorChanelFunction(xPointShiftedToRed));
	ubyte g = (ubyte)(255.0f * _colorChanelFunction(xPointShiftedToBlue));
	ubyte b = (ubyte)(255.0f * _colorChanelFunction(xPointShiftedToGreen));

	return { r,g,b };
}

void RenderDialog(WindowData* windowData)
{
	switch (windowData->dialogType)
	{
	case DialogWindowType::COLOR_PICKER:
	{
		int2 colorPickerSize = {
			windowData->dialogBitmap.size.x,
			windowData->dialogBitmap.size.y
		};

		//TODO: draw it onec and cache the bitmap
		const float functionSegmentsCount = 6.0f;
		for (int i = 0; i < colorPickerSize.x; i++)
		{
			float xPoint = functionSegmentsCount * (float)i / (float)colorPickerSize.x;

			float xPointShiftedToRed = xPoint + 3.0f; // red
			float xPointShiftedToGreen = xPoint + 1.0f; // green
			float xPointShiftedToBlue = xPoint + 5.0f; // blue

			if (xPointShiftedToRed >= functionSegmentsCount) xPointShiftedToRed -= functionSegmentsCount;
			if (xPointShiftedToGreen >= functionSegmentsCount) xPointShiftedToGreen -= functionSegmentsCount;
			if (xPointShiftedToBlue >= functionSegmentsCount) xPointShiftedToBlue -= functionSegmentsCount;

			ubyte r = (ubyte)(255.0f * _colorChanelFunction(xPointShiftedToRed));
			ubyte g = (ubyte)(255.0f * _colorChanelFunction(xPointShiftedToBlue));
			ubyte b = (ubyte)(255.0f * _colorChanelFunction(xPointShiftedToGreen));

			for (int j = 0; j < colorPickerSize.y; j++)
			{
				windowData->dialogBitmap.pixels[i + j * colorPickerSize.x] = { b,g,r,255 };
			}
		}

		if (windowData->wasMouseDoubleClick)
		{
			ubyte3 selectedColor = GetColorFromColorPicker(windowData->mousePosition, colorPickerSize);
			
			BrushColorTile* testColorTile = nullptr;
			for (int i = 0; i < windowData->brushColorTiles.length; i++)
			{
				testColorTile = windowData->brushColorTiles.getPointer(i);

				if (testColorTile->uiElement == windowData->selectedColorBrushForColorPicker)
				{
					break;
				}
			}
			//assert(testColorTile != nullptr);

			testColorTile->color = selectedColor;

			PostMessage(windowData->dialogHwnd, WM_SYSCOMMAND, SC_CLOSE, 0);
		}
		// render to dialog bitmap
		break;
	}
	}

	StretchDIBits(
		windowData->dialogDC,
		0, 0, windowData->dialogBitmap.size.x, windowData->dialogBitmap.size.y,
		0, 0, windowData->dialogBitmap.size.x, windowData->dialogBitmap.size.y,
		windowData->dialogBitmap.pixels,
		&windowData->dialogBitmapInfo,
		DIB_RGB_COLORS, SRCCOPY
	);
}