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

		windowData->wasMouseDoubleClick = true;
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

		free(windowData->colorPickerBitmap.pixels);
		windowData->wasColorPickerPreRendered = false;

		// clear data related to specific dialog type
		windowData->selectedColorBrushForColorPicker = UI_ELEMENT::NONE;
		ReleaseCapture();
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
		//childWc.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);
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
		childWindowSize = { 600, 400 };
		windowData->wasColorPickerPreRendered = false;
		windowData->colorPickerRect = { 350, 150, 550, 350 };
		windowData->colorPickerBitmap.size = windowData->colorPickerRect.size();

		windowData->colorPickerBitmap.pixels = (ubyte4*)malloc(4 * windowData->colorPickerBitmap.size.x * windowData->colorPickerBitmap.size.y);

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
	//ZeroMemory(windowData->dialogBitmap.pixels, 4 * windowData->dialogBitmap.size.x * windowData->dialogBitmap.size.y);
	//FillBitmapWithWhite(windowData->canvasBitmap.pixels, windowData->canvasBitmap.size);

	windowData->dialogHwnd = childHwnd;
	windowData->dialogType = dialogWindowType;
}


// Formula for caluculating hsl was taken from here
// https://www.niwa.nu/2013/05/math-behind-colorspace-conversions-rgb-hsl/
float _chanelToHslFormat(float tmp1, float tmp2, float chanelTmp)
{
	if (chanelTmp < 0) chanelTmp += 1.0f;
	else if (chanelTmp > 1.0f) chanelTmp -= 1.0f;

	// red tests
	if (6.0f * chanelTmp < 1.0f)
	{
		return tmp2 + (tmp1 - tmp2) * 6.0f * chanelTmp;
	}

	if (2.0f * chanelTmp < 1.0f)
	{
		return tmp1;
	}

	if (3.0f * chanelTmp < 2.0f)
	{
		return tmp2 + (tmp1 - tmp2) * (0.666f - chanelTmp) * 6.0f;
	}

	return tmp2;
}

int2 GetLocationOnColorPickerByColor(ubyte3 color, int2 colorPickerSize)
{
	float red = (float)color.x / 255.0f;
	float green = (float)color.y / 255.0f;
	float blue = (float)color.z / 255.0f;

	float max = maxFloat(red, maxFloat(green, blue));
	float min = minFloat(red, minFloat(green, blue));

	float lum = (max + min) / 2.0f;

	float hue = 0;
	float sat = 0;

	if (lum <= 0.5f)
	{
		sat = (max - min) / (max + min);
	}
	else
	{
		sat = (max - min) / (2.0f - max - min);
	}

	if (max == min)
	{
		hue = 0.0f;
	}
	else if (red >= green && red >= blue)
	{
		hue = (green - blue) / (max - min);
	}
	else if (green >= red && green >= blue)
	{
		hue = 2.0f + (blue - red) / (max - min);
	}
	else if (blue >= red && blue >= green)
	{
		hue = 4.0f + (red - green) / (max - min);
	}
	else
	{
		assert(false);
	}

	if (hue < 0)
	{
		hue += 6.0f;
	}

	hue /= 6.0f;

	return {
		(int)(hue * (float)colorPickerSize.x),
		(int)(sat * (float)colorPickerSize.y)
	};
}

ubyte3 GetColorFromColorPicker(int2 mousePosition, int2 colorPickerSize, int lum)
{
	float xPoint = (float)mousePosition.x / (float)colorPickerSize.x;
	float yPoint = (float)mousePosition.y / (float)colorPickerSize.y;

	float lumF = (float)lum / (float)100;

	ubyte greyShade = (ubyte)(255.0f * (float)lum / 100.0f);

	if (mousePosition.y == 0)
	{
		return { greyShade, greyShade, greyShade };
	}

	float hueF = (float)mousePosition.x / (float)colorPickerSize.x;
	float satF = (float)mousePosition.y / (float)colorPickerSize.y;

	float tmp1 = lumF < 0.5f
		? lumF * (1.0f + satF)
		: lumF + satF - lumF * satF;

	float tmp2 = 2.0f * lumF - tmp1;

	float tmpRed = hueF + 0.333f;
	float tmpGreen = hueF;
	float tmpBlue = hueF - 0.333f;

	float redF = _chanelToHslFormat(tmp1, tmp2, tmpRed);
	float greenF = _chanelToHslFormat(tmp1, tmp2, tmpGreen);
	float blueF = _chanelToHslFormat(tmp1, tmp2, tmpBlue);

	ubyte red = (ubyte)(redF * 255.0f);
	ubyte green = (ubyte)(greenF * 255.0f);
	ubyte blue = (ubyte)(blueF * 255.0f);

	return { red, green, blue };
}

// That's a simpler approach rather then trying to create some custom solution of mapping colors to a rectangular
// even though it's slower around 4 times
void DrawColorPicker(Bitmap bitmap, int2 colorPickerSize, int lum)
{
	assert(lum >= 0);
	assert(lum < 100);

	float lumF = (float)lum / (float)100;

	ubyte greyShade = (ubyte)(255.0f * (float)lum / 100.0f);

	for (int x = 0; x < colorPickerSize.x; x++)
	{
		bitmap.pixels[x] = { greyShade, greyShade, greyShade, 255 };
	}

	for (int y = 1; y < colorPickerSize.y; y++)
	{
		for (int x = 0; x < colorPickerSize.x; x++)
		{
			ubyte3 color = GetColorFromColorPicker({ x, y }, colorPickerSize, lum);
			bitmap.pixels[x + (y * bitmap.size.x)] = { color.z, color.y, color.x, 255 };
		}
	}
}

void RenderDialog(WindowData* windowData)
{
	FillBitmapWithWhite(windowData->dialogBitmap.pixels, windowData->dialogBitmap.size);

	switch (windowData->dialogType)
	{
	case DialogWindowType::COLOR_PICKER:
	{
		int4 colorPickerRect = windowData->colorPickerRect;
		int2 colorPickerSize = windowData->colorPickerRect.size();

		int lum = 50;
		// Update selected color
		if (windowData->isRightButtonHold && IsInRect(colorPickerRect, windowData->mousePosition))
		{
			int2 mousePositionOnColorPicker = windowData->mousePosition - colorPickerRect.xy();

			windowData->selectedColorInColorPicker = GetColorFromColorPicker(mousePositionOnColorPicker, colorPickerSize, lum);
		}

		// Draw selected color
		DrawRect(windowData->dialogBitmap, { 30,30 }, { 60,60 }, windowData->selectedColorInColorPicker);
		DrawTextLine(L"Color|Solid", { 30,0 }, &windowData->fontData, windowData->dialogBitmap);

		// TODO: draw it once and cache the bitmap
		if (!windowData->wasColorPickerPreRendered)
		{
			DrawColorPicker(windowData->colorPickerBitmap, colorPickerSize, lum);

			windowData->wasColorPickerPreRendered = true;
		}

		CopyBitmapToBitmap(windowData->colorPickerBitmap.pixels, windowData->colorPickerBitmap.size,
			windowData->dialogBitmap.pixels, windowData->colorPickerRect.xy(), windowData->dialogBitmap.size);
		DrawBorderRect(windowData->dialogBitmap, windowData->colorPickerRect.xy() - 1, windowData->colorPickerRect.size() + 1, 1, { 0,0,0 });

		int2 selectedColorsPoint = GetLocationOnColorPickerByColor(windowData->selectedColorInColorPicker, colorPickerSize);
		selectedColorsPoint += colorPickerRect.xy();

		// Draw point that indicates which color is selected
		int4 topArrow = ClipRect(colorPickerRect, {
			selectedColorsPoint.x - 1, selectedColorsPoint.y + 2,
			selectedColorsPoint.x + 1, selectedColorsPoint.y + 10
			});

		int4 bottomArrow = ClipRect(colorPickerRect, {
			selectedColorsPoint.x - 1, selectedColorsPoint.y - 12,
			selectedColorsPoint.x + 1, selectedColorsPoint.y - 2
			});

		int4 leftArrow = ClipRect(colorPickerRect, {
			selectedColorsPoint.x - 12, selectedColorsPoint.y - 1,
			selectedColorsPoint.x - 2, selectedColorsPoint.y + 1
			});

		int4 rightArrow = ClipRect(colorPickerRect, {
			selectedColorsPoint.x + 2, selectedColorsPoint.y - 1,
			selectedColorsPoint.x + 10, selectedColorsPoint.y + 1
			});

		DrawRect(windowData->dialogBitmap, topArrow.xy(), topArrow.size(), { 0, 0, 0 });
		DrawRect(windowData->dialogBitmap, bottomArrow.xy(), bottomArrow.size(), { 0, 0, 0 });
		DrawRect(windowData->dialogBitmap, leftArrow.xy(), leftArrow.size(), { 0, 0, 0 });
		DrawRect(windowData->dialogBitmap, rightArrow.xy(), rightArrow.size(), { 0, 0, 0 });

		if (windowData->wasMouseDoubleClick && IsInRect(windowData->colorPickerRect, windowData->mousePosition))
		{
			int2 mousePositionOnColorPicker = windowData->mousePosition - colorPickerRect.xy();

			ubyte3 selectedColor = GetColorFromColorPicker(mousePositionOnColorPicker, colorPickerSize, lum);

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