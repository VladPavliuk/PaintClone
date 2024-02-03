#pragma once

#include <Windows.h>
#include <windowsx.h>

#include "custom_types.h"
#include "dynamic_array.h"
#include "queue.h"

struct WindowData
{
	HDC deviceContext;
	BITMAPINFO bitmapInfo;
	ubyte* bitmap;
	
	// (first byte for z index, second byte for id) of a ui element
	ubyte2* zAndIdBuffer;

	int2 clientSize;

	int2 mousePosition;
	bool isRightButtonHold;
	int isDrawing;

	ubyte3 selectedColor;
	SimpleDynamicArray<int2> pixelsToDraw;
	int2 fillFrom;
};

LRESULT WINAPI WindowCallback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);