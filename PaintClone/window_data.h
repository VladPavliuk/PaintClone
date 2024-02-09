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
	FILL,
	ZOOM
};

enum class UI_ELEMENT
{
	NONE,

	DRAWING_CANVAS,

	CANVAS_VERTICAL_SCROLL,
	CANVAS_HORIZONTAL_SCROLL,
	CANVAS_CORNER_RESIZE,

	PENCIL_TOOL,
	FILL_TOOL,
	ZOOM_TOOL,

	COLOR_BRUCH_1,
	COLOR_BRUCH_2,
	COLOR_BRUCH_3,
	COLOR_BRUCH_4,
	COLOR_BRUCH_5,
	COLOR_BRUCH_6,
	COLOR_BRUCH_7,
	COLOR_BRUCH_8,
};

struct BrushColorTile
{
	BrushColorTile(ubyte3 color, UI_ELEMENT uiElement)
		: color(color), uiElement(uiElement)
	{}

	ubyte3 color;
	UI_ELEMENT uiElement;
};

struct ToolTile
{
	ToolTile(UI_ELEMENT uiElement, DRAW_TOOL tool, BmpImage image)
		: image(image), tool(tool), uiElement(uiElement)
	{}

	UI_ELEMENT uiElement;
	DRAW_TOOL tool;
	BmpImage image;
};

struct WindowData
{
	HDC windowDC;
	BITMAPINFO windowBitmapInfo;
	ubyte4* windowBitmap;

	HDC backgroundDC;
	HBITMAP backgroundBmp;

	BITMAPINFO drawingBitmapInfo;
	ubyte4* drawingBitmap;
	int2 drawingBitmapSize;
	int2 drawingZoneSize; // for client rect
	int2 drawingZonePosition; // relative to client rect
	int2 drawingZoneCornerPosition; // relative to client rect
	int2 drawingZoneCornerSize; // relative to client rect

	int drawingZoomLevel;
	int2 drawingOffset; // relative to bottom-left point

	// (first byte for z index, second byte for id) of a ui element
	//ubyte2* zAndIdBuffer;

	int2 windowClientSize;

	int2 prevMousePosition;
	int2 mousePosition;

	bool isRightButtonHold;

	bool wasRightButtonPressed;
	bool wasRightButtonReleased;

	//> ui
	UI_ELEMENT prevHotUi;
	UI_ELEMENT hotUi;
	int2 activeUiOffset; // delta coordinates from element position to mouse position, when element because active
	UI_ELEMENT activeUi;
	UI_ELEMENT sumbitedUi; // ui element that should be handled
	//<

	//bool isDrawing;

	DRAW_TOOL selectedTool;
	//SimpleDynamicArray<BmpImage> toolsImages;
	SimpleDynamicArray<ToolTile> toolTiles;

	ubyte3 selectedColor;
	SimpleDynamicArray<BrushColorTile> brushColorTiles;

	//SimpleDynamicArray<int2> pixelsToDraw;

	LARGE_INTEGER perfomanceCounterFreq;
};

double GetCurrentTimestamp(WindowData* windowData);

void InitWindowData(WindowData* windowData);

//LRESULT WINAPI WindowCallback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);