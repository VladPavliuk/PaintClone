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

	int2 clientSize;

	SimpleDynamicArray<int2> pixelsToDraw;
	int2 fillFrom;
};

LRESULT WINAPI WindowCallback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);