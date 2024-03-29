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
	windowData->windowBitmap = Bitmap(NULL, { -1, -1 });

	windowData->backgroundDC = 0;
	windowData->backgroundBmp = 0;

	// works correctly up to 32000x32000 pixels
	windowData->canvasBitmap = Bitmap(NULL, { 800, 800 });

	windowData->drawingZone = { 35, 35, 35, 35 };

	windowData->drawingZoneCornerResize = { 0, 0, 10, 10 };

	windowData->drawingZoomLevel = 1;
	windowData->drawingOffset = { 0, 0 };

	windowData->lastMouseCanvasPosition = { 0,0 };

	windowData->isRightButtonHold = false;
	windowData->wasRightButtonPressed = false;
	windowData->wasRightButtonReleased = false;
	windowData->wasMouseDoubleClick = false;
	windowData->mousePositionChanged = false;

	windowData->hotUi = UI_ELEMENT::NONE;
	windowData->activeUiOffset = { -1, -1 };
	windowData->activeUi = UI_ELEMENT::NONE;
	windowData->sumbitedUi = UI_ELEMENT::NONE;
	windowData->prevMousePosition = { 0,0 };
	windowData->mousePosition = { 0,0 };

	windowData->selectedColorBruchTile = UI_ELEMENT::COLOR_BRUCH_1;
	windowData->selectedTool = DRAW_TOOL::PENCIL;

	windowData->isTextEnteringMode = false;
	windowData->isValidVirtualKeycodeForText = false;
	windowData->textBuffer = WideString(L"");
	windowData->textBlockOnClient = { -1,-1,-1,-1 };
	windowData->textBlockButtonsSize = { 10, 10 };
	windowData->cursorPosition = -1;
	windowData->topLineIndexToShow = 0;
	windowData->selectedTextStartIndex = -1;
	windowData->glyphsLayout = NULL;

	windowData->initClickOnCanvasPosition = { -1, -1 };
	windowData->eraserBoxSize = 50;

	windowData->canvasSizeLabelBox = { 200, 0, 350, 30 };
	windowData->mouseCanvasPositionLabelBox = { 500, 0, 350, 30 };

	windowData->dialogType = DialogWindowType::NONE;
	windowData->dialogHwnd = 0;
	windowData->dialogDC = 0;

	windowData->selectedColorBrushForColorPicker = UI_ELEMENT::NONE;
	windowData->selectedColorInColorPicker = {0,0,0};
}

ubyte3 GetSelectedColor(WindowData* windowData)
{
	for (int i = 0; i < windowData->brushColorTiles.length; i++)
	{
		BrushColorTile colorTile = windowData->brushColorTiles.get(i);
		if (colorTile.uiElement == windowData->selectedColorBruchTile)
		{
			return colorTile.color;
		}
	}

	return { 0,0,0 };
}
