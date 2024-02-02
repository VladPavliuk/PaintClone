#pragma once

#include <Windows.h>

#include "custom_types.h"

struct WindowData
{
	HDC deviceContext;
	BITMAPINFO bitmapInfo;
	char* bitmap;

	int2 clientSize;
};

LRESULT WINAPI WindowCallback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);