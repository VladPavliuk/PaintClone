#include "renderer.h"

void InitRenderer(WindowData* windowData, HWND hwnd)
{
	windowData->deviceContext = GetDC(hwnd);

	RECT clientRect;
	GetClientRect(hwnd, &clientRect);
	windowData->clientSize.x = clientRect.right - clientRect.left;
	windowData->clientSize.y = clientRect.bottom - clientRect.top;

	BITMAPINFOHEADER header;
	windowData->bitmapInfo.bmiHeader.biSize = sizeof(windowData->bitmapInfo.bmiHeader);
	windowData->bitmapInfo.bmiHeader.biWidth = windowData->clientSize.x;
	windowData->bitmapInfo.bmiHeader.biHeight = windowData->clientSize.y;
	windowData->bitmapInfo.bmiHeader.biPlanes = 1;
	windowData->bitmapInfo.bmiHeader.biBitCount = 32;
	windowData->bitmapInfo.bmiHeader.biCompression = BI_RGB;
	windowData->bitmapInfo.bmiHeader.biSizeImage = 0;
	windowData->bitmapInfo.bmiHeader.biXPelsPerMeter = 0;
	windowData->bitmapInfo.bmiHeader.biYPelsPerMeter = 0;
	windowData->bitmapInfo.bmiHeader.biClrUsed = 0;
	windowData->bitmapInfo.bmiHeader.biClrImportant = 0;
	//windowData->bitmapInfo.bmiColors = 0;

	// create actual bitmap with r,g,b,_
	windowData->bitmap = (char*)malloc(4 * windowData->clientSize.x * windowData->clientSize.y);
	//ZeroMemory(windowData->bitmap, 4 * windowData->clientSize.x * windowData->clientSize.y);
}

void FillWindowClientWithWhite(WindowData* windowData)
{
	int length = 4 * windowData->clientSize.x * windowData->clientSize.y;
	for (int i = 0; i < length; i++)
	{
		windowData->bitmap[i] = (char)255;
	}
}

void DrawRect(WindowData* windowData, int x, int y, int width, int height, char3 color)
{
	char* currentPixel = windowData->bitmap + 4 * (x + windowData->clientSize.x * y);
	int pitch = 4 * windowData->clientSize.x;

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			*currentPixel = color.z; // b
			currentPixel++;

			*currentPixel = color.y; //g
			currentPixel++;

			*currentPixel = color.x; // r
			currentPixel++;

			//*currentPixel = 0;
			currentPixel++;
		}

		currentPixel -= 4 * width;
		currentPixel += pitch;
	}
}