#include "ui.h"

void HandleUiElements(WindowData* windowData)
{
	switch (windowData->sumbitedUi)
	{
	case UI_ELEMENT::NONE:
	{
		break;
	}
	case UI_ELEMENT::DRAWING_CANVAS:
	{
		switch (windowData->selectedTool)
		{
		case DRAW_TOOL::FILL:
		{
			int2 pixelToStart = windowData->mousePosition;

			pixelToStart = ConvertFromScreenToDrawingCoords(windowData, pixelToStart);

			FillFromPixel(windowData, pixelToStart, GetSelectedColor(windowData));
			break;
		}
		case DRAW_TOOL::TEXT:
		{
			if (windowData->isTextEnteringMode)
			{
				//> clear previuos state of text block
				if (windowData->textBuffer.length > 0)
				{
					CopyTextBufferToCanvas(windowData);
				}

				windowData->textBlockOnClient = { -1,-1,-1,-1 };
				windowData->cursorPosition = -1;
				windowData->topLineIndexToShow = 0;
				windowData->selectedTextStartIndex = -1;
				windowData->textBuffer.clear();
				if (windowData->glyphsLayout != NULL)
				{
					for (int i = 0; i < windowData->glyphsLayout->length; i++)
					{
						windowData->glyphsLayout->get(i).freeMemory();
					}
					windowData->glyphsLayout->freeMemory();
					free(windowData->glyphsLayout);
					windowData->glyphsLayout = NULL;
				}
				windowData->isTextEnteringMode = false;
				break;
				//<
			}

			// create text block state
			int defaultLinesPerBlock = 3;
			int defaultTextBlockWidth = 100 * windowData->drawingZoomLevel;
			int defaultTextBlockHeight = defaultLinesPerBlock * windowData->fontData.lineHeight * windowData->drawingZoomLevel;

			int4 textBlockRect = int4(windowData->mousePosition,
				{ windowData->mousePosition.x + defaultTextBlockWidth, windowData->mousePosition.y + defaultTextBlockHeight });

			textBlockRect = ClipRect(textBlockRect, int4(windowData->drawingZone.xy() + windowData->textBlockButtonsSize,
				windowData->drawingZone.zw() - windowData->textBlockButtonsSize));

			int test = (textBlockRect.x - windowData->drawingZone.x) % windowData->drawingZoomLevel;
			int test2 = (textBlockRect.y - windowData->drawingZone.y) % windowData->drawingZoomLevel;

			textBlockRect.x -= test;
			textBlockRect.y -= test2;
			textBlockRect.z -= test;
			textBlockRect.w -= test2;

			windowData->textBlockOnClient = textBlockRect;

			RecreateGlyphsLayout(windowData, windowData->textBuffer, textBlockRect.size().x);

			windowData->cursorPosition = 0;

			windowData->isTextEnteringMode = true;
			break;
		}
		case DRAW_TOOL::ERASER:
		{
			int2 centerEraserPosition = windowData->mousePosition;
			centerEraserPosition = ConvertFromScreenToDrawingCoords(windowData, centerEraserPosition);

			centerEraserPosition -= (int)((float)windowData->eraserBoxSize / 2.0f);

			int4 rectToErase = { centerEraserPosition.x, centerEraserPosition.y,
				centerEraserPosition.x + windowData->eraserBoxSize, centerEraserPosition.y + windowData->eraserBoxSize };

			rectToErase = ClipRect(rectToErase, windowData->canvasBitmap.size);

			for (int i = rectToErase.y; i < rectToErase.w; i++)
			{
				for (int j = rectToErase.x; j < rectToErase.z; j++)
				{
					int2 pixelToErase = { j, i };

					DrawPixel(windowData->canvasBitmap.pixels, windowData->canvasBitmap.size, pixelToErase, { 255, 255, 255 });
				}
			}
			break;
		}
		}
		break;
	}
	case UI_ELEMENT::PENCIL_TOOL:
	{

		windowData->selectedTool = DRAW_TOOL::PENCIL;
		break;
	}
	case UI_ELEMENT::FILL_TOOL:
	{
		windowData->selectedTool = DRAW_TOOL::FILL;
		break;
	}
	case UI_ELEMENT::ZOOM_TOOL:
	{
		windowData->drawingZoomLevel *= 2;

		if (windowData->drawingZoomLevel >= 16)
		{
			windowData->drawingZoomLevel = 1;
		}

		ValidateDrawingOffset(windowData);
		break;
	}
	case UI_ELEMENT::TEXT_TOOL:
	{
		windowData->selectedTool = DRAW_TOOL::TEXT;
		break;
	}
	case UI_ELEMENT::ERASER_TOOL:
	{
		windowData->selectedTool = DRAW_TOOL::ERASER;
		break;
	}
	case UI_ELEMENT::LINE_TOOL:
	{
		windowData->selectedTool = DRAW_TOOL::LINE;
		break;
	}
	case UI_ELEMENT::RECTANGLE_TOOL:
	{
		windowData->selectedTool = DRAW_TOOL::RECTANGLE;
		break;
	}
	case UI_ELEMENT::COLOR_BRUCH_1:
	case UI_ELEMENT::COLOR_BRUCH_2:
	case UI_ELEMENT::COLOR_BRUCH_3:
	case UI_ELEMENT::COLOR_BRUCH_4:
	case UI_ELEMENT::COLOR_BRUCH_5:
	case UI_ELEMENT::COLOR_BRUCH_6:
	case UI_ELEMENT::COLOR_BRUCH_7:
	case UI_ELEMENT::COLOR_BRUCH_8:
	{
		for (int i = 0; i < windowData->brushColorTiles.length; i++)
		{
			BrushColorTile brushColorTile = windowData->brushColorTiles.get(i);
			if (brushColorTile.uiElement == windowData->sumbitedUi)
			{
				windowData->selectedColorBruchTile = brushColorTile.uiElement;
				break;
			}
		}

		if (windowData->wasMouseDoubleClick)
		{
			windowData->selectedColorBrushForColorPicker = windowData->selectedColorBruchTile;
			windowData->selectedColorInColorPicker = GetSelectedColor(windowData);

			ShowDialogWindow(windowData, DialogWindowType::COLOR_PICKER);
		}
		break;
	}
	default:
	{
		//OutputDebugString(L"YEAH\n");
		break;
	}
	}

	switch (windowData->sumbitedOnAnyHotUi)
	{
	case UI_ELEMENT::NONE:
	{
		break;
	}
	case UI_ELEMENT::DRAWING_CANVAS:
	{
		switch (windowData->selectedTool)
		{
		case DRAW_TOOL::LINE:
		{
			int4 drawingRect;

			drawingRect.x = 0;
			drawingRect.y = 0;
			drawingRect.z = windowData->drawingZone.size().x;
			drawingRect.w = windowData->drawingZone.size().y;

			drawingRect.x += windowData->drawingOffset.x;
			drawingRect.y += windowData->drawingOffset.y;
			drawingRect.z += windowData->drawingOffset.x;
			drawingRect.w += windowData->drawingOffset.y;

			drawingRect.x = (int)((float)drawingRect.x / (float)windowData->drawingZoomLevel);
			drawingRect.y = (int)((float)drawingRect.y / (float)windowData->drawingZoomLevel);
			drawingRect.z = (int)((float)drawingRect.z / (float)windowData->drawingZoomLevel);
			drawingRect.w = (int)((float)drawingRect.w / (float)windowData->drawingZoomLevel);

			int2 fromPixelToErase = ConvertFromScreenToDrawingCoords(windowData, windowData->initClickOnCanvasPosition);
			int2 toPixelToErase = ConvertFromScreenToDrawingCoords(windowData, windowData->mousePosition);

			DrawLine(windowData->canvasBitmap.pixels, windowData->canvasBitmap.size,
				drawingRect, fromPixelToErase, toPixelToErase, GetSelectedColor(windowData));

			windowData->initClickOnCanvasPosition = { -1,-1 };
			break;
		}
		case DRAW_TOOL::RECTANGLE:
		{
			int4 drawingRect;

			drawingRect.x = 0;
			drawingRect.y = 0;
			drawingRect.z = windowData->drawingZone.size().x;
			drawingRect.w = windowData->drawingZone.size().y;

			drawingRect.x += windowData->drawingOffset.x;
			drawingRect.y += windowData->drawingOffset.y;
			drawingRect.z += windowData->drawingOffset.x;
			drawingRect.w += windowData->drawingOffset.y;

			drawingRect.x = (int)((float)drawingRect.x / (float)windowData->drawingZoomLevel);
			drawingRect.y = (int)((float)drawingRect.y / (float)windowData->drawingZoomLevel);
			drawingRect.z = (int)((float)drawingRect.z / (float)windowData->drawingZoomLevel);
			drawingRect.w = (int)((float)drawingRect.w / (float)windowData->drawingZoomLevel);

			int2 fromPixel = ConvertFromScreenToDrawingCoords(windowData, windowData->initClickOnCanvasPosition);
			int2 toPixel = ConvertFromScreenToDrawingCoords(windowData, windowData->mousePosition);

			int4 rect = {
				minInt(fromPixel.x, toPixel.x),
				minInt(fromPixel.y, toPixel.y),
				maxInt(fromPixel.x, toPixel.x),
				maxInt(fromPixel.y, toPixel.y)
			};

			rect = ClipRect(rect, drawingRect);

			int lineWidth = 1;
			DrawBorderRect(windowData->canvasBitmap, rect.xy(), rect.size(),
				lineWidth, GetSelectedColor(windowData));

			/*DrawLine(windowData->canvasBitmap.pixels, windowData->canvasBitmap.size,
				drawingRect, fromPixel, toPixel, GetSelectedColor(windowData));*/

			windowData->initClickOnCanvasPosition = { -1,-1 };
			break;
		}
		}
		break;
	}
	case UI_ELEMENT::CANVAS_CORNER_RESIZE:
	{
		// recalculate new drawing bitmap size
		int2 previousBitmapSize = windowData->canvasBitmap.size;
		windowData->canvasBitmap.size = windowData->drawingZoneCornerResize.xy() - windowData->drawingZone.xy();

		CalculateDrawingZoneSize(windowData);

		// reallocate drawing bitmap for new drawing size
		ubyte4* previousBitmap = windowData->canvasBitmap.pixels;

		windowData->drawingBitmapInfo.bmiHeader.biWidth = windowData->canvasBitmap.size.x;
		windowData->drawingBitmapInfo.bmiHeader.biHeight = windowData->canvasBitmap.size.y;

		windowData->canvasBitmap.pixels = (ubyte4*)malloc(4 * windowData->canvasBitmap.size.x * windowData->canvasBitmap.size.y);
		FillBitmapWithWhite(windowData->canvasBitmap.pixels, windowData->canvasBitmap.size);

		CopyBitmapToBitmap(previousBitmap, previousBitmapSize,
			windowData->canvasBitmap.pixels, { 0, 0 }, windowData->canvasBitmap.size);

		free(previousBitmap);

		// adjust canvas scrolls
		ValidateDrawingOffset(windowData);
		break;
	}
	}

	switch (windowData->activeUi)
	{
	case UI_ELEMENT::NONE:
	{
		break;
	}
	case UI_ELEMENT::CANVAS_CORNER_RESIZE:
	{
		int2 drawingZoneCornerResizeSize = windowData->drawingZoneCornerResize.size();

		int2 canvasCornerPosition = windowData->mousePosition - windowData->activeUiOffset;

		canvasCornerPosition = ClipPoint(canvasCornerPosition, int4(windowData->drawingZone.xy() + 10, windowData->windowBitmap.size - 10));

		windowData->drawingZoneCornerResize.xy(canvasCornerPosition);
		windowData->drawingZoneCornerResize.zw(canvasCornerPosition + 10);

		// Draw a border rectangle, to better show how current canvas will look after resize
		DrawBorderRect(windowData->windowBitmap, windowData->drawingZone.xy(),
			canvasCornerPosition - windowData->drawingZone.xy(), 1, { 0, 255, 0 });
		break;
	}
	case UI_ELEMENT::CANVAS_VERTICAL_SCROLL:
	{
		windowData->drawingOffset.y += windowData->drawingZoomLevel * (windowData->mousePosition.y - windowData->prevMousePosition.y);

		ValidateDrawingOffset(windowData);
		break;
	}
	case UI_ELEMENT::CANVAS_HORIZONTAL_SCROLL:
	{
		windowData->drawingOffset.x += windowData->drawingZoomLevel * (windowData->mousePosition.x - windowData->prevMousePosition.x);

		ValidateDrawingOffset(windowData);
		break;
	}
	case UI_ELEMENT::DRAWING_CANVAS:
	{
		if (windowData->selectedTool == DRAW_TOOL::PENCIL)
		{
			int2 fromPixel = windowData->prevMousePosition;
			int2 toPixel = windowData->mousePosition;

			if (!IsInRect(windowData->drawingZone, fromPixel) && !IsInRect(windowData->drawingZone, toPixel))
			{
				break;
			}

			fromPixel = ConvertFromScreenToDrawingCoords(windowData, fromPixel);
			toPixel = ConvertFromScreenToDrawingCoords(windowData, toPixel);

			int4 drawingRect;

			drawingRect.x = 0;
			drawingRect.y = 0;
			drawingRect.z = windowData->drawingZone.size().x;
			drawingRect.w = windowData->drawingZone.size().y;

			drawingRect.x += windowData->drawingOffset.x;
			drawingRect.y += windowData->drawingOffset.y;
			drawingRect.z += windowData->drawingOffset.x;
			drawingRect.w += windowData->drawingOffset.y;

			drawingRect.x = (int)((float)drawingRect.x / (float)windowData->drawingZoomLevel);
			drawingRect.y = (int)((float)drawingRect.y / (float)windowData->drawingZoomLevel);
			drawingRect.z = (int)((float)drawingRect.z / (float)windowData->drawingZoomLevel);
			drawingRect.w = (int)((float)drawingRect.w / (float)windowData->drawingZoomLevel);

			if (fromPixel.x == toPixel.x && fromPixel.y == toPixel.y)
			{
				DrawPixel(windowData->canvasBitmap.pixels, windowData->canvasBitmap.size, toPixel, GetSelectedColor(windowData));
			}
			else
			{
				DrawLine(windowData->canvasBitmap.pixels, windowData->canvasBitmap.size, drawingRect, fromPixel, toPixel, GetSelectedColor(windowData));
			}
		}
		else if (windowData->selectedTool == DRAW_TOOL::ERASER)
		{
			int2 mouse = windowData->mousePosition;
			int2 prevMouse = windowData->prevMousePosition;

			if (!IsInRect(windowData->drawingZone, mouse))
			{
				break;
			}

			int4 drawingRect;

			drawingRect.x = 0;
			drawingRect.y = 0;
			drawingRect.z = windowData->drawingZone.size().x;
			drawingRect.w = windowData->drawingZone.size().y;

			drawingRect.x += windowData->drawingOffset.x;
			drawingRect.y += windowData->drawingOffset.y;
			drawingRect.z += windowData->drawingOffset.x;
			drawingRect.w += windowData->drawingOffset.y;

			drawingRect.x = (int)((float)drawingRect.x / (float)windowData->drawingZoomLevel);
			drawingRect.y = (int)((float)drawingRect.y / (float)windowData->drawingZoomLevel);
			drawingRect.z = (int)((float)drawingRect.z / (float)windowData->drawingZoomLevel);
			drawingRect.w = (int)((float)drawingRect.w / (float)windowData->drawingZoomLevel);

			mouse = ConvertFromScreenToDrawingCoords(windowData, mouse);
			prevMouse = ConvertFromScreenToDrawingCoords(windowData, prevMouse);

			mouse.x -= (int)((float)windowData->eraserBoxSize / 2.0f);
			mouse.y -= (int)((float)windowData->eraserBoxSize / 2.0f);

			prevMouse.x -= (int)((float)windowData->eraserBoxSize / 2.0f);
			prevMouse.y -= (int)((float)windowData->eraserBoxSize / 2.0f);
			// square eraser
			for (int i = 0; i < windowData->eraserBoxSize; i++)
			{
				for (int j = 0; j < windowData->eraserBoxSize; j++)
				{
					int2 fromPixelToErase = { mouse.x + i, mouse.y + j };
					int2 toPixelToErase = { prevMouse.x + i, prevMouse.y + j };

					DrawLine(windowData->canvasBitmap.pixels, windowData->canvasBitmap.size,
						drawingRect, fromPixelToErase, toPixelToErase, { 255, 255, 255 });
				}
			}

			// circle eraser
			/*float radius = (float)windowData->eraserBoxSize / 2.0f;
			for (int i = 0; i < windowData->eraserBoxSize; i++)
			{
				for (int j = 0; j < windowData->eraserBoxSize; j++)
				{
					float x = i - radius;
					float y = j - radius;

					float test = sqrt(y * y + x * x);

					if (test > radius)
					{
						continue;
					}

					int2 fromPixelToErase = { mouse.x + i, mouse.y + j };
					int2 toPixelToErase = { prevMouse.x + i, prevMouse.y + j };

					DrawLine(windowData->drawingBitmap, windowData->drawingBitmapSize,
						drawingRect, fromPixelToErase, toPixelToErase, { 255, 255, 255 });
				}
			}*/
		}
		else if (windowData->selectedTool == DRAW_TOOL::LINE)
		{
			if (windowData->wasRightButtonPressed)
			{
				windowData->initClickOnCanvasPosition = windowData->mousePosition;
			}

			int2 fromPixelToErase = windowData->initClickOnCanvasPosition;
			int2 toPixelToErase = windowData->mousePosition;

			DrawLine(windowData->windowBitmap.pixels, windowData->windowBitmap.size,
				windowData->drawingZone, fromPixelToErase, toPixelToErase, GetSelectedColor(windowData));
		}
		else if (windowData->selectedTool == DRAW_TOOL::RECTANGLE)
		{
			if (windowData->wasRightButtonPressed)
			{
				windowData->initClickOnCanvasPosition = windowData->mousePosition;
			}

			int2 fromPixel = windowData->initClickOnCanvasPosition;
			int2 toPixel = windowData->mousePosition;

			int4 rect = {
				minInt(fromPixel.x, toPixel.x),
				minInt(fromPixel.y, toPixel.y),
				maxInt(fromPixel.x, toPixel.x),
				maxInt(fromPixel.y, toPixel.y)
			};

			rect = ClipRect(rect, windowData->drawingZone);

			int lineWidth = 1;
			DrawBorderRect(windowData->windowBitmap, rect.xy(), rect.size(), 
				lineWidth * windowData->drawingZoomLevel, GetSelectedColor(windowData));
		}
		break;
	}
	case UI_ELEMENT::TEXT_BLOCK:
	{
		int oldCursorPosition = windowData->cursorPosition;
		windowData->cursorPosition = GetCursorPositionByMousePosition(windowData);

		if (windowData->wasRightButtonPressed)
		{
			if (GetKeyState(VK_SHIFT) & 0x8000)
			{
				windowData->selectedTextStartIndex = oldCursorPosition;
			}
			else
			{
				windowData->selectedTextStartIndex = windowData->cursorPosition;
			}
		}
		break;
	}
	case UI_ELEMENT::TEXT_BLOCK_TOP_LEFT_CORNER_RESIZE:
	{
		if (!windowData->mousePositionChanged) break;

		int2 topLeftCorner = {
			windowData->mousePosition.x + windowData->activeUiOffset.x,
			windowData->mousePosition.y - windowData->activeUiOffset.y
		};

		int4 possiblePositionsZone = {
			windowData->drawingZone.x + windowData->textBlockButtonsSize.x,
			windowData->textBlockOnClient.y + windowData->textBlockButtonsSize.y,
			windowData->textBlockOnClient.z - windowData->textBlockButtonsSize.x,
			windowData->drawingZone.w - windowData->textBlockButtonsSize.y
		};

		topLeftCorner = ClipPoint(topLeftCorner, possiblePositionsZone);

		windowData->textBlockOnClient.x = topLeftCorner.x;
		windowData->textBlockOnClient.w = topLeftCorner.y;

		RecreateGlyphsLayout(windowData, windowData->textBuffer, windowData->textBlockOnClient.size().x);
		break;
	}
	case UI_ELEMENT::TEXT_BLOCK_TOP_RIGHT_CORNER_RESIZE:
	{
		if (!windowData->mousePositionChanged) break;

		int2 topRightCorner = {
			windowData->mousePosition.x - windowData->activeUiOffset.x,
			windowData->mousePosition.y - windowData->activeUiOffset.y
		};

		int4 possiblePositionsZone = {
			windowData->textBlockOnClient.x + windowData->textBlockButtonsSize.x,
			windowData->textBlockOnClient.y + windowData->textBlockButtonsSize.y,
			windowData->drawingZone.z - windowData->textBlockButtonsSize.x,
			windowData->drawingZone.w - windowData->textBlockButtonsSize.y
		};

		topRightCorner = ClipPoint(topRightCorner, possiblePositionsZone);

		windowData->textBlockOnClient.z = topRightCorner.x;
		windowData->textBlockOnClient.w = topRightCorner.y;

		RecreateGlyphsLayout(windowData, windowData->textBuffer, windowData->textBlockOnClient.size().x);
		break;
	}
	case UI_ELEMENT::TEXT_BLOCK_BOTTOM_RIGHT_CORNER_RESIZE:
	{
		if (!windowData->mousePositionChanged) break;

		int2 bottomRightCorner = {
			windowData->mousePosition.x - windowData->activeUiOffset.x,
			windowData->mousePosition.y + windowData->activeUiOffset.y
		};

		int4 possiblePositionsZone = {
			windowData->textBlockOnClient.x + windowData->textBlockButtonsSize.x,
			windowData->drawingZone.y + windowData->textBlockButtonsSize.y,
			windowData->drawingZone.z - windowData->textBlockButtonsSize.x,
			windowData->textBlockOnClient.w - windowData->textBlockButtonsSize.y
		};

		bottomRightCorner = ClipPoint(bottomRightCorner, possiblePositionsZone);

		windowData->textBlockOnClient.z = bottomRightCorner.x;
		windowData->textBlockOnClient.y = bottomRightCorner.y;

		RecreateGlyphsLayout(windowData, windowData->textBuffer, windowData->textBlockOnClient.size().x);
		break;
	}
	case UI_ELEMENT::TEXT_BLOCK_BOTTOM_LEFT_CORNER_RESIZE:
	{
		if (!windowData->mousePositionChanged) break;

		int2 bottomRightCorner = {
			windowData->mousePosition.x + windowData->activeUiOffset.x,
			windowData->mousePosition.y + windowData->activeUiOffset.y
		};

		int4 possiblePositionsZone = {
			windowData->drawingZone.x + windowData->textBlockButtonsSize.x,
			windowData->drawingZone.y + windowData->textBlockButtonsSize.y,
			windowData->textBlockOnClient.z - windowData->textBlockButtonsSize.x,
			windowData->textBlockOnClient.w - windowData->textBlockButtonsSize.y
		};

		bottomRightCorner = ClipPoint(bottomRightCorner, possiblePositionsZone);

		windowData->textBlockOnClient.x = bottomRightCorner.x;
		windowData->textBlockOnClient.y = bottomRightCorner.y;

		RecreateGlyphsLayout(windowData, windowData->textBuffer, windowData->textBlockOnClient.size().x);
		break;
	}
	case UI_ELEMENT::TEXT_BLOCK_TOP_RESIZE:
	{
		if (!windowData->mousePositionChanged) break;

		int topPosition = windowData->mousePosition.y - windowData->activeUiOffset.y;
		int2 possibleTopPositionsRange = {
			windowData->textBlockOnClient.y + windowData->textBlockButtonsSize.y,
			windowData->drawingZone.w - windowData->textBlockButtonsSize.y };

		windowData->textBlockOnClient.w = ClipPoint(topPosition, possibleTopPositionsRange);

		RecreateGlyphsLayout(windowData, windowData->textBuffer, windowData->textBlockOnClient.size().x);
		break;
	}
	case UI_ELEMENT::TEXT_BLOCK_RIGHT_RESIZE:
	{
		if (!windowData->mousePositionChanged) break;

		int rightPosition = windowData->mousePosition.x - windowData->activeUiOffset.x;

		int2 possibleRightPositionsRange = {
			windowData->textBlockOnClient.x + windowData->textBlockButtonsSize.x,
			windowData->drawingZone.z - windowData->textBlockButtonsSize.x };

		windowData->textBlockOnClient.z = ClipPoint(rightPosition, possibleRightPositionsRange);

		RecreateGlyphsLayout(windowData, windowData->textBuffer, windowData->textBlockOnClient.size().x);
		break;
	}
	case UI_ELEMENT::TEXT_BLOCK_BOTTOM_RESIZE:
	{
		if (!windowData->mousePositionChanged) break;

		int bottomPosition = windowData->mousePosition.y + windowData->activeUiOffset.y;

		int2 possibleBottomPositionsRange = {
			windowData->drawingZone.y + windowData->textBlockButtonsSize.y,
			windowData->textBlockOnClient.w - windowData->textBlockButtonsSize.y };

		windowData->textBlockOnClient.y = ClipPoint(bottomPosition, possibleBottomPositionsRange);

		RecreateGlyphsLayout(windowData, windowData->textBuffer, windowData->textBlockOnClient.size().x);
		break;
	}
	case UI_ELEMENT::TEXT_BLOCK_LEFT_RESIZE:
	{
		if (!windowData->mousePositionChanged) break;

		int leftPosition = windowData->mousePosition.x + windowData->activeUiOffset.x;

		int2 possibleBottomPositionsRange = {
			windowData->drawingZone.x + windowData->textBlockButtonsSize.y,
			windowData->textBlockOnClient.z - windowData->textBlockButtonsSize.y };

		windowData->textBlockOnClient.x = ClipPoint(leftPosition, possibleBottomPositionsRange);

		RecreateGlyphsLayout(windowData, windowData->textBuffer, windowData->textBlockOnClient.size().x);
		break;
	}
	}

	switch (windowData->hotUi)
	{
	case UI_ELEMENT::DRAWING_CANVAS:
	{
		switch (windowData->selectedTool)
		{
		case DRAW_TOOL::ERASER:
		{
			int erazerBoxSize = windowData->eraserBoxSize * windowData->drawingZoomLevel;
			int2 eraserCenter = windowData->mousePosition;
			eraserCenter -= erazerBoxSize / 2;

			int4 eraserRect = { eraserCenter.x, eraserCenter.y, eraserCenter.x + erazerBoxSize, eraserCenter.y + erazerBoxSize };

			eraserRect = ClipRect(eraserRect, windowData->drawingZone);
			int2 eraserRectSize = eraserRect.size();

			DrawRect(windowData->windowBitmap,
				eraserRect.xy(),
				eraserRectSize,
				{ 255,255,255 });

			DrawBorderRect(windowData->windowBitmap,
				eraserRect.xy(),
				eraserRectSize,
				1, { 0,0,0 });
			break;
		}
		}
		break;
	}
	}
}

void DrawDrawingCanvas(WindowData* windowData)
{
	DrawBorderRect(windowData->windowBitmap, { windowData->drawingZone.x, windowData->drawingZone.y },
		windowData->drawingZone.size(), 2, { 255, 0, 0 });
	DrawScrollsForDrawingZone(windowData);

	CheckHotActiveForUiElement(windowData, {
		windowData->drawingZone.x, windowData->drawingZone.y,
		windowData->drawingZone.z, windowData->drawingZone.w
		}, UI_ELEMENT::DRAWING_CANVAS);
}

void DrawScrollsForDrawingZone(WindowData* windowData)
{
	int scrollWidth = 10;
	ubyte3 scrollBgColor = { 150, 150, 150 };
	ubyte3 scrollHoveredBgColor = { 120, 120, 120 };

	// vertical bar
	float drawingZoneToImageHeightRatio = (float)windowData->drawingZone.size().y
		/ ((float)windowData->canvasBitmap.size.y * (float)windowData->drawingZoomLevel);

	if (drawingZoneToImageHeightRatio < 1.0f)
	{
		DrawButton(windowData,
			{
				windowData->drawingZone.x - scrollWidth,
				(int)(drawingZoneToImageHeightRatio * windowData->drawingOffset.y) + windowData->drawingZone.y
			},
			{ scrollWidth, (int)(drawingZoneToImageHeightRatio * windowData->drawingZone.size().y) },
			scrollBgColor, scrollHoveredBgColor, UI_ELEMENT::CANVAS_VERTICAL_SCROLL);
	}

	// horizontal bar
	float drawingZoneToImageWidthRatio = (float)windowData->drawingZone.size().x / ((float)windowData->canvasBitmap.size.x * (float)windowData->drawingZoomLevel);

	if (drawingZoneToImageWidthRatio < 1.0f)
	{
		DrawButton(windowData,
			{
				(int)(drawingZoneToImageWidthRatio * windowData->drawingOffset.x) + windowData->drawingZone.x,
				windowData->drawingZone.y - scrollWidth,
			},
			{ (int)(drawingZoneToImageWidthRatio * windowData->drawingZone.size().x), scrollWidth },
			scrollBgColor, scrollHoveredBgColor, UI_ELEMENT::CANVAS_HORIZONTAL_SCROLL);
	}
}

void DrawPanel(WindowData* windowData,
	int2 bottomLeft, int2 size,
	ubyte3 bgColor,
	ubyte zIndex, ubyte uiId)
{
	/*DrawPanel(windowData,
		bottomLeft, size,
		bgColor,
		zIndex, uiId);

	DrawRectToZAndIdBuffer(windowData,
		bottomLeft, size,
		zIndex, uiId);*/
}

void DrawBitmapButton(WindowData* windowData,
	int2 bottomLeft, int2 size,
	ubyte4* bitmap,
	ubyte3 hoveredBgColor,
	UI_ELEMENT uiElement)
{
	CheckHotActiveForUiElement(windowData, { bottomLeft.x, bottomLeft.y, bottomLeft.x + size.x, bottomLeft.y + size.y }, uiElement);

	//TODO: add some kind of image stretching because we might want to have buttons that dont have the same size as a bitmap
	DrawBitmap(windowData, bitmap, bottomLeft, size);

	DrawBorderRect(windowData->windowBitmap, bottomLeft, size, 1, { 0,0,0 });
}

void DrawButton(WindowData* windowData,
	int2 bottomLeft, int2 size,
	ubyte3 bgColor, ubyte3 hoveredBgColor,
	UI_ELEMENT uiElement)
{
	CheckHotActiveForUiElement(windowData, { bottomLeft.x, bottomLeft.y, bottomLeft.x + size.x, bottomLeft.y + size.y }, uiElement);

	if (windowData->activeUi == uiElement || (windowData->hotUi == uiElement && windowData->activeUi == UI_ELEMENT::NONE))
	{
		DrawRect(windowData->windowBitmap, bottomLeft, size, hoveredBgColor);
	}
	else
	{
		DrawRect(windowData->windowBitmap, bottomLeft, size, bgColor);
	}

	DrawBorderRect(windowData->windowBitmap, bottomLeft, size, 1, { 0,0,0 });
}

void DrawModalWindow(WindowData* windowData, WideString* title, int4 modalWindowBox, 
	UI_ELEMENT modalWindowUiId)
{
	CheckHotActiveForUiElement(windowData, modalWindowBox, modalWindowUiId);

	// background
	DrawRect(windowData->windowBitmap, modalWindowBox.xy(), modalWindowBox.size(), { 200,200,200 });

	// title background
	DrawRect(windowData->windowBitmap, { modalWindowBox.x, modalWindowBox.w - windowData->fontData.lineHeight }, 
		{ modalWindowBox.size().x, windowData->fontData.lineHeight }, { 255,255,255 });
	
	// title
	DrawTextLine(title, 
		{ modalWindowBox.x, modalWindowBox.w - windowData->fontData.lineHeight }, 
		&windowData->fontData, windowData->windowBitmap);

	// close button


	// modal window border
	DrawBorderRect(windowData->windowBitmap, modalWindowBox.xy(), modalWindowBox.size(), 1, { 0,0,0 });
}

void DrawColorsBrush(WindowData* windowData, SimpleDynamicArray<BrushColorTile>* brushColors, int2 bottomLeft,
	int2 singleColorTileSize, int xDistanceToNextColor)
{
	for (int i = 0; i < brushColors->length; i++)
	{
		BrushColorTile brushColor = brushColors->get(i);

		DrawButton(windowData,
			{ bottomLeft.x + (singleColorTileSize.x * i) + (xDistanceToNextColor * i), bottomLeft.y },
			singleColorTileSize, brushColor.color, brushColor.color, brushColor.uiElement);

		if (brushColor.uiElement == windowData->selectedColorBruchTile)
		{
			DrawBorderRect(windowData->windowBitmap,
				{ bottomLeft.x + (singleColorTileSize.x * i) + (xDistanceToNextColor * i) - 1, bottomLeft.y - 1 },
				{ singleColorTileSize.x + 1, singleColorTileSize.y + 1 }, 2, { 0,0,0 });
		}
	}
}

void DrawToolsPanel(WindowData* windowData, int2 bottomLeft,
	int2 singleToolTileSize, int yDistanceToNextToolTile)
{
	for (int i = 0; i < windowData->toolTiles.length; i++)
	{
		ToolTile tool = windowData->toolTiles.get(i);

		// TODO: create some of kind of a registry of ui ids to prevent collisions
		DrawBitmapButton(windowData,
			{ bottomLeft.x, bottomLeft.y + (i * singleToolTileSize.y) + (i * yDistanceToNextToolTile) },
			tool.image.size, tool.image.rgbaBitmap, { 20, 80, 200 }, tool.uiElement);

		if (tool.tool == windowData->selectedTool)
		{
			DrawBorderRect(windowData->windowBitmap, { bottomLeft.x - 1, bottomLeft.y + (i * singleToolTileSize.y) + (i * yDistanceToNextToolTile) - 1 },
				{ tool.image.size.x + 1, tool.image.size.y + 1 }, 2, { 0,0,0 });
		}
	}
}

void DrawCanvasSizeLabel(WindowData* windowData)
{
	WideString lable = WideString(L"");

	lable.append(windowData->canvasBitmap.size.x);
	lable.append(L" x ");
	lable.append(windowData->canvasBitmap.size.y);
	lable.append(L" px");

	DrawTextLine(&lable, windowData->canvasSizeLabelBox.xy(), &windowData->fontData, windowData->windowBitmap);

	lable.freeMemory();
}

void DrawMouseCanvasPositionLabel(WindowData* windowData)
{
	WideString lable = WideString(L"");
	int2 mouseCanvasPosition = ConvertFromScreenToDrawingCoords(windowData, windowData->lastMouseCanvasPosition);

	lable.append(mouseCanvasPosition.x);
	lable.append(L" x ");
	lable.append(mouseCanvasPosition.y);
	lable.append(L" px");

	DrawTextLine(&lable, windowData->mouseCanvasPositionLabelBox.xy(), &windowData->fontData, windowData->windowBitmap);

	lable.freeMemory();
}

void DrawDraggableCornerOfDrawingZone(WindowData* windowData)
{
	DrawButton(windowData,
		windowData->drawingZoneCornerResize.xy(),
		windowData->drawingZoneCornerResize.size(), { 100, 100, 100 }, { 150, 150, 150 }, UI_ELEMENT::CANVAS_CORNER_RESIZE);
}

void DrawTextBlock(WindowData* windowData)
{
	if (!windowData->isTextEnteringMode)
	{
		return;
	}

	float scale = (float)windowData->drawingZoomLevel;

	CheckHotActiveForUiElement(windowData, windowData->textBlockOnClient, UI_ELEMENT::TEXT_BLOCK);

	DrawBorderRect(windowData->windowBitmap,
		windowData->textBlockOnClient.xy(),
		windowData->textBlockOnClient.size(), 1, { 0, 255, 0 });

	int2 textSelectionRange = { -1, -1 };
	if (windowData->selectedTextStartIndex != -1)
	{
		textSelectionRange = GetSelectedTextRange(windowData);
	}

	// render text
	int textBlockLeftSide = windowData->textBlockOnClient.z;
	int maxLinesInTextBlock = windowData->textBlockOnClient.size().y / windowData->fontData.lineHeight;
	int topLineIndex = windowData->topLineIndexToShow;
	for (int layoutLineIndex = topLineIndex, lineIndex = 0; layoutLineIndex < windowData->glyphsLayout->length; layoutLineIndex++, lineIndex++)
	{
		if (lineIndex >= maxLinesInTextBlock)
		{
			break;
		}

		auto line = windowData->glyphsLayout->get(layoutLineIndex);
		int lineTopOffset = (int)(windowData->textBlockOnClient.w - (lineIndex + 1) * windowData->fontData.lineHeight * scale);
		for (int j = 0; j < line.length; j++)
		{
			int2 glyphData = line.get(j);
			int charIndex = glyphData.y;

			// check if text in selection if any
			bool isSymbolInSelection = (charIndex >= textSelectionRange.x && charIndex < textSelectionRange.y);

			wchar_t code = windowData->textBuffer.chars[charIndex];
			int lineLeftOffset = windowData->textBlockOnClient.x + glyphData.x;

			if (code != L'\n' && code != L'\0') // glyphs layout includes \0 at the end of the last line
			{
				RasterizedGlyph rasterizedGlyph = windowData->fontData.glyphs.get(code);
				int2 position = {
					lineLeftOffset + (int)((float)rasterizedGlyph.leftSideBearings * scale),
					lineTopOffset + (int)((float)(rasterizedGlyph.boundaries.y + -windowData->fontData.descent) * scale) };

				if (isSymbolInSelection)
				{
					DrawRect(windowData->windowBitmap, { lineLeftOffset, lineTopOffset },
						{ rasterizedGlyph.advanceWidth, windowData->fontData.lineHeight }, { 0,0,0 });
				}

				if (rasterizedGlyph.hasBitmap)
				{
					CopyMonochromicBitmapToBitmap(rasterizedGlyph.bitmap, rasterizedGlyph.bitmapSize,
						windowData->windowBitmap.pixels, position, windowData->windowBitmap.size,
						(int)scale, isSymbolInSelection);
				}
			}

			// NOTE: if new line symbol in the selection, we have to highlight it manually
			if (code == L'\n' && isSymbolInSelection)
			{
				int newLineSymbolSelectionWidth = 20;

				if (lineLeftOffset + newLineSymbolSelectionWidth > textBlockLeftSide)
				{
					newLineSymbolSelectionWidth = textBlockLeftSide - lineLeftOffset;
				}

				DrawRect(windowData->windowBitmap, { lineLeftOffset, lineTopOffset },
					{ newLineSymbolSelectionWidth, windowData->fontData.lineHeight }, { 0, 0, 0 });
			}

			if (charIndex == windowData->cursorPosition)
			{
				DrawRect(windowData->windowBitmap, { lineLeftOffset, lineTopOffset },
					{ 1, (int)((float)windowData->fontData.lineHeight * scale) }, { 0,0,0 });
			}
		}
	}
}

void DrawTextBlockResizeButtons(WindowData* windowData)
{
	if (!windowData->isTextEnteringMode) return;

	//> corner resize buttons
	DrawButton(windowData,
		{ windowData->textBlockOnClient.x - windowData->textBlockButtonsSize.x, windowData->textBlockOnClient.w },
		windowData->textBlockButtonsSize, { 100, 100, 100 }, { 150, 150, 150 }, UI_ELEMENT::TEXT_BLOCK_TOP_LEFT_CORNER_RESIZE);

	DrawButton(windowData,
		{ windowData->textBlockOnClient.z, windowData->textBlockOnClient.w },
		windowData->textBlockButtonsSize, { 100, 100, 100 }, { 150, 150, 150 }, UI_ELEMENT::TEXT_BLOCK_TOP_RIGHT_CORNER_RESIZE);

	DrawButton(windowData,
		{ windowData->textBlockOnClient.z, windowData->textBlockOnClient.y - windowData->textBlockButtonsSize.y },
		windowData->textBlockButtonsSize, { 100, 100, 100 }, { 150, 150, 150 }, UI_ELEMENT::TEXT_BLOCK_BOTTOM_RIGHT_CORNER_RESIZE);

	DrawButton(windowData,
		{ windowData->textBlockOnClient.x - windowData->textBlockButtonsSize.x, windowData->textBlockOnClient.y - windowData->textBlockButtonsSize.y },
		windowData->textBlockButtonsSize, { 100, 100, 100 }, { 150, 150, 150 }, UI_ELEMENT::TEXT_BLOCK_BOTTOM_LEFT_CORNER_RESIZE);
	//<

	//> edges resize buttons
	DrawButton(windowData,
		{
			windowData->textBlockOnClient.x + (windowData->textBlockOnClient.size().x - windowData->textBlockButtonsSize.x) / 2,
			windowData->textBlockOnClient.w
		},
		windowData->textBlockButtonsSize, { 100, 100, 100 }, { 150, 150, 150 }, UI_ELEMENT::TEXT_BLOCK_TOP_RESIZE);

	DrawButton(windowData,
		{
			windowData->textBlockOnClient.z,
			windowData->textBlockOnClient.y + (windowData->textBlockOnClient.size().y - windowData->textBlockButtonsSize.y) / 2
		},
		windowData->textBlockButtonsSize, { 100, 100, 100 }, { 150, 150, 150 }, UI_ELEMENT::TEXT_BLOCK_RIGHT_RESIZE);

	DrawButton(windowData,
		{
			windowData->textBlockOnClient.x - windowData->textBlockButtonsSize.x,
			windowData->textBlockOnClient.y + (windowData->textBlockOnClient.size().y - windowData->textBlockButtonsSize.y) / 2
		},
		windowData->textBlockButtonsSize, { 100, 100, 100 }, { 150, 150, 150 }, UI_ELEMENT::TEXT_BLOCK_LEFT_RESIZE);

	DrawButton(windowData,
		{
			windowData->textBlockOnClient.x + (windowData->textBlockOnClient.size().x - windowData->textBlockButtonsSize.x) / 2,
			windowData->textBlockOnClient.y - windowData->textBlockButtonsSize.y
		},
		windowData->textBlockButtonsSize, { 100, 100, 100 }, { 150, 150, 150 }, UI_ELEMENT::TEXT_BLOCK_BOTTOM_RESIZE);
	//<
}

void CheckHotActiveForUiElement(WindowData* windowData, int4 boundaries, UI_ELEMENT uiElement)
{
	bool isInRect = IsInRect(boundaries, windowData->mousePosition);

	if (isInRect)
	{
		windowData->hotUi = uiElement;

		if (windowData->wasMouseDoubleClick)
		{
			windowData->sumbitedUi = uiElement;
			windowData->sumbitedOnAnyHotUi = uiElement;
			windowData->activeUi = UI_ELEMENT::NONE;
			windowData->activeUiOffset = { -1, -1 };
			return;
		}
	}
	//windowData->hotUi = isInRect ? uiElement : UI_ELEMENT::NONE;

	if (windowData->activeUi == uiElement)
	{
		if (windowData->wasRightButtonReleased)
		{
			//OutputDebugString(L"released\n");
			if (windowData->hotUi == uiElement) windowData->sumbitedUi = uiElement;

			windowData->sumbitedOnAnyHotUi = uiElement;
			windowData->activeUi = UI_ELEMENT::NONE;
			windowData->activeUiOffset = { -1, -1 };
		}
	}
	else if (windowData->hotUi == uiElement)
	{
		if (windowData->wasRightButtonPressed)
		{
			//OutputDebugString(L"pressed\n");
			windowData->activeUiOffset = { windowData->mousePosition.x - boundaries.x, windowData->mousePosition.y - boundaries.y };
			windowData->activeUi = uiElement;
		}
	}
}
