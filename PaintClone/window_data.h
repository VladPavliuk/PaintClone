#pragma once

#include <Windows.h>
#include <windowsx.h>

#include "custom_types.h"
#include "dynamic_array.h"
#include "queue.h"
#include "bmp_images.h"

enum class DRAW_TOOL
{
	UNDEFINED,
	PENCIL,
	FILL
};

struct WindowData
{
	HDC deviceContext;
	BITMAPINFO bitmapInfo;
	ubyte4* uiBitmap;

	// (first byte for z index, second byte for id) of a ui element
	ubyte2* zAndIdBuffer;

	int2 clientSize;

	int2 mousePosition;

	bool isRightButtonHold;

	bool isDrawing;

	DRAW_TOOL selectedTool;
	SimpleDynamicArray<BmpImage> toolsImages;

	ubyte3 selectedColor;
	SimpleDynamicArray<ubyte3> colorsInBrush;

	SimpleDynamicArray<int2> pixelsToDraw;
	int2 oneTimeClick;
};

//LRESULT WINAPI WindowCallback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);