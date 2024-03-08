#pragma once

#include <Windows.h>
#include <windowsx.h>

#include "custom_types.h"
#include "dynamic_array.h"
#include "queue.h"
#include "bmp_images.h"
#include "fonts.h"
#include "string.h"

enum class DialogWindowType
{
	NONE,
	COLOR_PICKER
};

enum class DRAW_TOOL
{
	UNDEFINED,
	PENCIL,
	FILL,
	ZOOM,
	TEXT,
	ERASER,
	LINE
};

enum class UI_ELEMENT
{
	NONE,

	DRAWING_CANVAS,

	CANVAS_VERTICAL_SCROLL,
	CANVAS_HORIZONTAL_SCROLL,
	CANVAS_CORNER_RESIZE,

	TEXT_BLOCK,
	TEXT_BLOCK_TOP_LEFT_CORNER_RESIZE,
	TEXT_BLOCK_TOP_RIGHT_CORNER_RESIZE,
	TEXT_BLOCK_BOTTOM_RIGHT_CORNER_RESIZE,
	TEXT_BLOCK_BOTTOM_LEFT_CORNER_RESIZE,
	TEXT_BLOCK_TOP_RESIZE,
	TEXT_BLOCK_RIGHT_RESIZE,
	TEXT_BLOCK_BOTTOM_RESIZE,
	TEXT_BLOCK_LEFT_RESIZE,

	COLOR_PICKER_MODAL_WINDOW,

	PENCIL_TOOL,
	FILL_TOOL,
	ZOOM_TOOL,
	TEXT_TOOL,
	ERASER_TOOL,
	LINE_TOOL,

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

struct Bitmap
{
	Bitmap()
	{}

	Bitmap(ubyte4* pixels, int2 size):
		pixels(pixels), size(size)
	{}

	ubyte4* pixels;
	int2 size;
};

struct WindowData
{
	int test;
	HWND parentHwnd;
	HINSTANCE hInstance;
	int4 windowRect;

	HDC windowDC;
	BITMAPINFO windowBitmapInfo;
	
	Bitmap windowBitmap;
	Bitmap canvasBitmap;

	HDC backgroundDC;
	HBITMAP backgroundBmp;

	BITMAPINFO drawingBitmapInfo;

	int4 drawingZone; // relative to client rect

	//TODO: probably there's no need to store dimensions of this button, sine we can get it from canavs dimensions
	int4 drawingZoneCornerResize; // relative to client rect

	int drawingZoomLevel;
	int2 drawingOffset; // relative to bottom-left point

	int2 prevMousePosition;
	int2 mousePosition;
	int2 lastMouseCanvasPosition;
	bool mousePositionChanged;

	bool isRightButtonHold;

	bool wasRightButtonPressed;
	bool wasRightButtonReleased;
	bool wasMouseDoubleClick;

	//> ui
	UI_ELEMENT prevHotUi;
	UI_ELEMENT hotUi;
	int2 activeUiOffset; // delta coordinates from element position to mouse position, when element because active
	UI_ELEMENT activeUi;
	UI_ELEMENT sumbitedUi; // ui element that is set when press and release happens on the same ui element
	UI_ELEMENT sumbitedOnAnyHotUi; // ui element that is set when press and release happens on any ui element
	//<

	//> text
	FontDataRasterized fontData;
	bool isTextEnteringMode;
	bool isValidVirtualKeycodeForText;
	int4 textBlockOnClient;
	int2 textBlockButtonsSize;
	WideString textBuffer;
	SimpleDynamicArray<SimpleDynamicArray<int2>>* glyphsLayout;
	int cursorPosition;
	int topLineIndexToShow;
	int selectedTextStartIndex; // from index in text buffer, to index in text buffer
	//<

	DRAW_TOOL selectedTool;
	SimpleDynamicArray<ToolTile> toolTiles;

	//ubyte3 selectedColor;
	UI_ELEMENT selectedColorBruchTile;
	SimpleDynamicArray<BrushColorTile> brushColorTiles;

	int2 initClickOnCanvasPosition;
	int eraserBoxSize;

	int4 canvasSizeLabelBox;
	int4 mouseCanvasPositionLabelBox;

	LARGE_INTEGER perfomanceCounterFreq;

	//> dialogs
	DialogWindowType dialogType;
	HWND dialogHwnd;
	HDC dialogDC;
	BITMAPINFO dialogBitmapInfo;
	Bitmap dialogBitmap;

	// color picker
	bool wasColorPickerPreRendered;
	int4 colorPickerRect;
	Bitmap colorPickerBitmap;
	UI_ELEMENT selectedColorBrushForColorPicker;
	ubyte3 selectedColorInColorPicker;
	//<
};

double GetCurrentTimestamp(WindowData* windowData);

void InitWindowData(WindowData* windowData);

ubyte3 GetSelectedColor(WindowData* windowData);
//LRESULT WINAPI WindowCallback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);