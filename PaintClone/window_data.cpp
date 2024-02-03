#include "window_data.h"

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
		WindowData* windowData = (WindowData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

		int xMouse = GET_X_LPARAM(lParam);
		int yMouse = GET_Y_LPARAM(lParam);

		yMouse = windowData->clientSize.y - yMouse;

		windowData->fillFrom = { xMouse, yMouse };
		break;
	}
	case WM_LBUTTONUP:
	{
		WindowData* windowData = (WindowData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

		windowData->isRightButtonHold = false;
		break;
	}
	case WM_LBUTTONDOWN:
	{
		WindowData* windowData = (WindowData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

		windowData->isRightButtonHold = true;
		break;
	}
	case WM_MOUSEMOVE:
	{
		if (wParam == 0x0001)
		{
			WindowData* windowData = (WindowData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

			int xMouse = GET_X_LPARAM(lParam);
			int yMouse = GET_Y_LPARAM(lParam);

			yMouse = windowData->clientSize.y - yMouse;

			windowData->pixelsToDraw.add({ xMouse, yMouse });

			//int pixelIndex = 4 * (xMouse + yMouse * windowData->clientSize.x);
			//windowData->bitmap[pixelIndex] = 130;
			//windowData->bitmap[pixelIndex + 1] = 130;
			//windowData->bitmap[pixelIndex + 2] = 130;
		}

		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}
