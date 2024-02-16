#include "window_data.h"

double GetCurrentTimestamp(WindowData* windowData)
{
	LARGE_INTEGER currentTimestamp;
	QueryPerformanceCounter(&currentTimestamp);
	return (double)currentTimestamp.QuadPart / (double)windowData->perfomanceCounterFreq.QuadPart;
}

void InitWindowData(WindowData* windowData)
{
	windowData->windowDC = 0;
	//windowData->windowBitmapInfo = 0;
	windowData->windowBitmap = NULL;

	windowData->backgroundDC = 0;
	windowData->backgroundBmp = 0;

	windowData->drawingBitmap = NULL;
	// works correctly up to 32000x32000 pixels
	//windowData.drawingBitmapSize = { 3840, 2160 };
	windowData->drawingBitmapSize = { 800, 800 };

	windowData->drawingZone = { 35, 35, 35, 35 };
	windowData->drawingZone.UpdateSize();

	windowData->drawingZoneCornerResize = { 0, 0, 0, 0 };
	windowData->drawingZoneCornerResize.size = { 10, 10 };
	windowData->drawingZoneCornerResize.UpdateTopRight();

	windowData->drawingZoomLevel = 1;
	windowData->drawingOffset = { 0, 0 };

	windowData->isRightButtonHold = false;
	windowData->wasRightButtonPressed = false;
	windowData->wasRightButtonPressed = false;

	windowData->hotUi = UI_ELEMENT::NONE;
	windowData->activeUiOffset = { -1, -1 };
	windowData->activeUi = UI_ELEMENT::NONE;
	windowData->sumbitedUi = UI_ELEMENT::NONE;
	windowData->prevMousePosition = { 0,0 };
	windowData->mousePosition = { 0,0 };
	windowData->selectedColor = { 0,0,0 };
	windowData->selectedTool = DRAW_TOOL::PENCIL;
}
