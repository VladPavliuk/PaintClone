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
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}
