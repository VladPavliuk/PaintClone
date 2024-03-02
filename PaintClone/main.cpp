#include "ui.h"

#include <stdio.h>
#include <dwmapi.h>
#include "fonts.h"
#include "hash_table.h"
#include "string.h"

void RasterizeTestingFontAndPutOnCanvas(WindowData* windowData)
{
	const wchar_t* fontFilePath = L"C:\\Windows\\Fonts\\arial.ttf";
	//const wchar_t* fontFilePath = L"C:\\Windows\\Fonts\\timesi.ttf";
	//const wchar_t* fontFilePath = L"C:\\Windows\\Fonts\\times.ttf";
	//const wchar_t* fontFilePath = L"C:\\Windows\\Fonts\\timesbi.ttf";
	//const wchar_t* fontFilePath = L"C:\\Windows\\Fonts\\calibri.ttf"; // THIS ONE HAS COMPLEX CONTOURS
	//const wchar_t* fontFilePath = L"C:\\Windows\\Fonts\\Candarai.ttf"; // THIS ONE HAS COMPLEX CONTOURS
	//const wchar_t* fontFilePath = L"C:\\Windows\\Fonts\\corbel.ttf"; // THIS ONE HAS COMPLEX CONTOURS
	//const wchar_t* fontFilePath = L"C:\\Windows\\Fonts\\comicbd.ttf";
	//const wchar_t* fontFilePath = L"C:\\Windows\\Fonts\\segoeuiz.ttf";
	//const wchar_t* fontFilePath = L"Envy Code R.ttf";
	//const wchar_t* fontFilePath = L"ShadeBlue-2OozX.ttf";

	//> testing fonts
	//wchar_t alphabetStr[] = L"АБВГҐДЕЄЖЗИІЇЙКЛМНОПРСТУФХЦЧШЩЬЮЯабвгґдеєжзиіїйклмнопрстуфхцчшщьюя !\"#$%&'()*+,-./0123456789:;<=>?@[\\]^_`{|}~ABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnoprstuvwxyz";
	wchar_t alphabetStr[] = L" !\"#$%&'()*+,-./0123456789:;<=>?@[\\]^_`{|}~ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	//wchar_t alphabetStr[] = L"ABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnoprstuvwxyz";
	//wchar_t alphabetStr[] = L"~";

	SimpleDynamicArray<wchar_t> alphabet = SimpleDynamicArray<wchar_t>(10);
	for (int i = 0; i < wcslen(alphabetStr); i++)
	{
		alphabet.add(alphabetStr[i]);
	}

	FontData font = ReadFontFromTTF(fontFilePath, &alphabet);

	/*glyphs.resetIteration();
	HashTableItem<int, FontGlyph> fontGlyph;
	while (glyphs.getNext(&fontGlyph))
	{
		for (int i = 0; i < fontGlyph.value.contours.length; i++)
		{
			fontGlyph.value.contours.get(i).freeMemory();
		}
		fontGlyph.value.contours.freeMemory();
	}
	glyphs.freeMemory();*/

	int verticalSize = 50;
	double timeDelta2 = GetCurrentTimestamp(windowData);
	//windowData->fontData.maxBoundaries = verticalSize;

	windowData->fontData = RasterizeFontGlyphs(&font, verticalSize);

	timeDelta2 = GetCurrentTimestamp(windowData) - timeDelta2;
	char buff[100];
	sprintf_s(buff, "frame time: %f ml sec.\n", timeDelta2 * 1000.0f);
	OutputDebugStringA(buff);

	//RasterizedGlyph glyph = rasterizedGlyphs.get(L'a');

	//int2 bottomLeft = { 0,font.lineHeight };
	//for (int i = 0; i < alphabet.length; i++)
	//{
	//	wchar_t code = alphabet.get(i);

	//	RasterizedGlyph rasterizedGlyph = rasterizedGlyphs.get(code);

	//	int2 position = { bottomLeft.x + rasterizedGlyph.leftSideBearings, bottomLeft.y + rasterizedGlyph.boundaries.y };

	//	CopyMonochromicBitmapToBitmap(rasterizedGlyph.bitmap, rasterizedGlyph.bitmapSize, windowData->drawingBitmap, position, windowData->drawingBitmapSize);

	//	if (rasterizedGlyph.bitmapSize.x > windowData->drawingBitmapSize.x) continue;

	//	bottomLeft.x += rasterizedGlyph.advanceWidth;

	//	if (bottomLeft.x + rasterizedGlyph.bitmapSize.x > windowData->drawingBitmapSize.x)
	//	{
	//		bottomLeft.y += font.lineHeight;
	//		//bottomLeft.y += 10;
	//		bottomLeft.x = 0;
	//	}

	//	if (bottomLeft.y + verticalSize > windowData->drawingBitmapSize.y) break;
	//}
}

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
	case WM_LBUTTONUP:
	{
		WindowData* windowData = (WindowData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

		windowData->isRightButtonHold = false;
		windowData->wasRightButtonReleased = true;

		// NOTE: We have to release previously capture, because we won't be able to use windws default buttons on the window
		ReleaseCapture();
		break;
	}
	case WM_LBUTTONDOWN:
	{
		WindowData* windowData = (WindowData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

		windowData->isRightButtonHold = true;
		windowData->wasRightButtonPressed = true;

		if (windowData->isTextEnteringMode)
		{
			if (IsInRect(windowData->textBlockOnClient, windowData->mousePosition))
			{
				if (GetKeyState(VK_SHIFT) & 0x8000)
				{
					if (windowData->selectedTextStartIndex == -1)
						windowData->selectedTextStartIndex = windowData->cursorPosition;
				}
				else
				{
					windowData->selectedTextStartIndex = -1;
				}

				int textBlockHeight = windowData->textBlockOnClient.size().y;
				int visibleLinesCount = textBlockHeight / windowData->fontData.lineHeight;
				int lineIndex = (windowData->textBlockOnClient.w - windowData->mousePosition.y) / windowData->fontData.lineHeight;

				lineIndex += windowData->topLineIndexToShow;
				int mouseLeftOffset = windowData->mousePosition.x - windowData->textBlockOnClient.x;

				if (lineIndex >= windowData->glyphsLayout->length)
				{
					int lastLindeIndex = windowData->glyphsLayout->length - 1;
					auto lastLine = windowData->glyphsLayout->get(lastLindeIndex);
					int2 symbol = lastLine.get(lastLine.length - 1);
					windowData->cursorPosition = symbol.y;
				}
				else
				{
					auto line = windowData->glyphsLayout->get(lineIndex);

					for (int i = 0; i < line.length; i++)
					{
						int2 symbol = line.get(i);

						if (mouseLeftOffset < symbol.x)
						{
							bool hasPrevSymbol = i > 0;
							if (hasPrevSymbol)
							{
								int2 prevSymbol = line.get(i - 1);

								if ((symbol.x - mouseLeftOffset) < (mouseLeftOffset - prevSymbol.x))
								{
									windowData->cursorPosition = symbol.y;
								}
							}
							break;
						}

						windowData->cursorPosition = symbol.y;
					}
				}
			}
			else
			{
				// exit text entering mode
				if (windowData->textBuffer.length > 0)
				{
					//CopyTextBufferToCanvas(windowData);
				}

				windowData->isTextEnteringMode = false;
				windowData->textBlockOnClient = { -1,-1,-1,-1 };
				windowData->cursorPosition = -1;
				windowData->topLineIndexToShow = 0;
				windowData->selectedTextStartIndex = -1;
				//windowData->cursorLayoutPosition = { -1, -1 };
				windowData->textBuffer.clear();
			}
		}
		SetCapture(hwnd);
		break;
	}
	case WM_KEYUP:
	{
		WindowData* windowData = (WindowData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

		/*if (0x51 == wParam)
		{
			FillBitmapWithWhite(windowData->drawingBitmap, windowData->drawingBitmapSize);
			RasterizeTestingFontAndPutOnCanvas(windowData);
		}*/
		break;
	}
	case WM_KEYDOWN:
	{
		WindowData* windowData = (WindowData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

		if (windowData->isTextEnteringMode)
		{
			if (wParam == 0x56 && (GetKeyState(VK_CONTROL) & 0x8000)) // v + ctrl
			{
				wchar_t* textFromClipboardData = (wchar_t*)GetTextFromClipBoard(hwnd);

				if (textFromClipboardData != 0)
				{
					WideString textFromClipboard = WideString(textFromClipboardData);

					textFromClipboard.removeChar(L'\r');

					if (windowData->selectedTextStartIndex != -1)
					{
						int2 selectionRange = GetSelectedTextRange(windowData);

						windowData->textBuffer.removeRange(selectionRange.x, selectionRange.y - selectionRange.x);

						if (windowData->cursorPosition > windowData->selectedTextStartIndex)
						{
							windowData->cursorPosition -= selectionRange.y - selectionRange.x;
						}
					}

					windowData->textBuffer.insert(windowData->cursorPosition, textFromClipboard.chars);
					windowData->cursorPosition += (int)wcslen(textFromClipboard.chars);

					RecreateGlyphsLayout(windowData, windowData->textBuffer, windowData->textBlockOnClient.size().x);
					UpdateTextBlockTopLine(windowData);

					textFromClipboard.freeMemory();
					windowData->selectedTextStartIndex = -1;
				}
			}
			else if (wParam == 0x43 && (GetKeyState(VK_CONTROL) & 0x8000)) // c
			{
				if (windowData->selectedTextStartIndex != -1)
				{
					int2 selectionRange = GetSelectedTextRange(windowData);

					WideString selectedText = windowData->textBuffer.substring(selectionRange.x, selectionRange.y - selectionRange.x);

					PutTextIntoClipboard(hwnd, selectedText);
					selectedText.freeMemory();
				}
			}
			else if (wParam == 0x5A && (GetKeyState(VK_CONTROL) & 0x8000)) // z
			{
				//TODO: implement
			}
			else if (wParam == 0x41 && (GetKeyState(VK_CONTROL) & 0x8000)) // ctrl + a
			{
				windowData->cursorPosition = 0;
				windowData->selectedTextStartIndex = windowData->textBuffer.length;
			}
			else if (wParam == VK_RETURN)
			{
				if (windowData->selectedTextStartIndex != -1)
				{
					int2 selectionRange = GetSelectedTextRange(windowData);

					windowData->textBuffer.removeRange(selectionRange.x, selectionRange.y - selectionRange.x);
					if (windowData->cursorPosition > windowData->selectedTextStartIndex)
					{
						windowData->cursorPosition -= selectionRange.y - selectionRange.x;
					}
					windowData->selectedTextStartIndex = -1;
				}

				windowData->textBuffer.insert(windowData->cursorPosition, L'\n');
				windowData->cursorPosition++;

				RecreateGlyphsLayout(windowData, windowData->textBuffer, windowData->textBlockOnClient.size().x);
				UpdateTextBlockTopLine(windowData);

				windowData->selectedTextStartIndex = -1;
			}
			else if (wParam == VK_BACK)
			{
				if (windowData->selectedTextStartIndex != -1)
				{
					int2 selectionRange = GetSelectedTextRange(windowData);

					windowData->textBuffer.removeRange(selectionRange.x, selectionRange.y - selectionRange.x);
					windowData->selectedTextStartIndex = -1;
					windowData->cursorPosition = selectionRange.x;
				}
				else if (windowData->cursorPosition > 0)
				{
					windowData->textBuffer.removeByIndex(windowData->cursorPosition - 1);
					windowData->cursorPosition--;
				}

				RecreateGlyphsLayout(windowData, windowData->textBuffer, windowData->textBlockOnClient.size().x);
				UpdateTextBlockTopLine(windowData);
			}
			else if (wParam == VK_HOME)
			{
				int2 cursorLayoutPosition = GetCursorLayoutPotision(windowData);

				auto cursorLine = windowData->glyphsLayout->get(cursorLayoutPosition.x);

				int2 firstSymbolOnLine = cursorLine.get(0);

				if (GetKeyState(VK_SHIFT) & 0x8000)
				{
					if (windowData->selectedTextStartIndex == -1)
						windowData->selectedTextStartIndex = windowData->cursorPosition;
				}
				else
				{
					windowData->selectedTextStartIndex = -1;
				}

				windowData->cursorPosition = firstSymbolOnLine.y;
			}
			else if (wParam == VK_END)
			{
				int2 cursorLayoutPosition = GetCursorLayoutPotision(windowData);

				auto cursorLine = windowData->glyphsLayout->get(cursorLayoutPosition.x);

				int2 firstSymbolOnLine = cursorLine.get(cursorLine.length - 1);

				if (GetKeyState(VK_SHIFT) & 0x8000)
				{
					if (windowData->selectedTextStartIndex == -1)
						windowData->selectedTextStartIndex = windowData->cursorPosition;
				}
				else
				{
					windowData->selectedTextStartIndex = -1;
				}

				windowData->cursorPosition = firstSymbolOnLine.y;
			}
			else if (wParam == VK_DELETE)
			{
				if (windowData->selectedTextStartIndex != -1)
				{
					int2 selectionRange = GetSelectedTextRange(windowData);

					windowData->textBuffer.removeRange(selectionRange.x, selectionRange.y - selectionRange.x);
					windowData->selectedTextStartIndex = -1;
					windowData->cursorPosition = selectionRange.x;
				}
				else if (windowData->cursorPosition >= 0)
				{
					windowData->textBuffer.removeByIndex(windowData->cursorPosition);
				}
				RecreateGlyphsLayout(windowData, windowData->textBuffer, windowData->textBlockOnClient.size().x);
				UpdateTextBlockTopLine(windowData);
			}
			else if (wParam == VK_SPACE
				//|| wParam == VK_RETURN
				|| wParam == VK_OEM_PLUS
				|| wParam == VK_OEM_COMMA
				|| wParam == VK_OEM_MINUS
				|| wParam == VK_OEM_PERIOD
				|| wParam == VK_OEM_1 // ;:
				|| wParam == VK_OEM_2 // /?
				|| wParam == VK_OEM_3 // `~
				|| wParam == VK_OEM_4 // [{
				|| wParam == VK_OEM_5 // \|
				|| wParam == VK_OEM_6 // ]}
				|| wParam == VK_OEM_7 // '"
				|| wParam == VK_ADD // +
				|| wParam == VK_SUBTRACT // -
				|| wParam == VK_MULTIPLY // *
				|| wParam == VK_DIVIDE // /
				|| wParam == VK_DECIMAL // .
				|| wParam >= 0x60 && wParam <= 0x69 // numpad
				|| wParam >= 0x41 && wParam <= 0x5A
				|| wParam >= 0x30 && wParam <= 0x39)
			{
				windowData->isValidVirtualKeycodeForText = true;
			}
			else if (VK_UP == wParam)
			{
				int2 cursorLayoutPosition = GetCursorLayoutPotision(windowData);

				UpdateTextSelectionifShiftPressed(windowData);

				MoveCursorToNewLine(windowData, cursorLayoutPosition.x - 1, cursorLayoutPosition.y);
				UpdateTextBlockTopLine(windowData);
			}
			else if (VK_DOWN == wParam)
			{
				int2 cursorLayoutPosition = GetCursorLayoutPotision(windowData);

				UpdateTextSelectionifShiftPressed(windowData);

				MoveCursorToNewLine(windowData, cursorLayoutPosition.x + 1, cursorLayoutPosition.y);
				UpdateTextBlockTopLine(windowData);
			}
			else if (VK_RIGHT == wParam)
			{
				bool wasAnyTextSelected = windowData->selectedTextStartIndex != -1;
				int2 selectionRange = {-1,-1};
				if (wasAnyTextSelected)
				{
					selectionRange = GetSelectedTextRange(windowData);
				}
				UpdateTextSelectionifShiftPressed(windowData);

				bool isTextSelectionGone = wasAnyTextSelected && windowData->selectedTextStartIndex == -1;

				if (isTextSelectionGone)
				{
					windowData->cursorPosition = selectionRange.y;
				}
				else
				{
					windowData->cursorPosition++;
				}

				if (windowData->cursorPosition > windowData->textBuffer.length)
					windowData->cursorPosition = windowData->textBuffer.length;
				UpdateTextBlockTopLine(windowData);
			}
			else if (VK_LEFT == wParam)
			{
				bool wasAnyTextSelected = windowData->selectedTextStartIndex != -1;
				int2 selectionRange = { -1,-1 };
				if (wasAnyTextSelected)
				{
					selectionRange = GetSelectedTextRange(windowData);
				}
				UpdateTextSelectionifShiftPressed(windowData);

				bool isTextSelectionGone = wasAnyTextSelected && windowData->selectedTextStartIndex == -1;

				if (isTextSelectionGone)
				{
					windowData->cursorPosition = selectionRange.x;
				}
				else
				{
					windowData->cursorPosition--;
				}

				if (windowData->cursorPosition < 0)
					windowData->cursorPosition = 0;
				UpdateTextBlockTopLine(windowData);
			}
		}
		else
		{
			int offsetPerMove = (int)(100.0f / (float)windowData->drawingZoomLevel);

			if (VK_UP == wParam)
			{
				windowData->drawingOffset.y += offsetPerMove;
			}

			if (VK_DOWN == wParam)
			{
				windowData->drawingOffset.y -= offsetPerMove;
			}

			if (VK_RIGHT == wParam)
			{
				windowData->drawingOffset.x += offsetPerMove;
			}

			if (VK_LEFT == wParam)
			{
				windowData->drawingOffset.x -= offsetPerMove;
			}

			ValidateDrawingOffset(windowData);
		}
		break;
	}
	case WM_CHAR:
	{
		WindowData* windowData = (WindowData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

		if (windowData->isTextEnteringMode)
		{
			if (windowData->isValidVirtualKeycodeForText)
			{
				if (windowData->selectedTextStartIndex != -1)
				{
					int2 selectionRange = GetSelectedTextRange(windowData);

					windowData->textBuffer.removeRange(selectionRange.x, selectionRange.y - selectionRange.x);
					if (windowData->cursorPosition > windowData->selectedTextStartIndex)
					{
						windowData->cursorPosition -= selectionRange.y - selectionRange.x;
					}
					windowData->selectedTextStartIndex = -1;
				}

				windowData->textBuffer.insert(windowData->cursorPosition, wchar_t(wParam));
				windowData->cursorPosition++;
				windowData->isValidVirtualKeycodeForText = false;

				RecreateGlyphsLayout(windowData, windowData->textBuffer, windowData->textBlockOnClient.size().x);
				UpdateTextBlockTopLine(windowData);
			}

			if (windowData->cursorPosition > windowData->textBuffer.length)
				windowData->cursorPosition = windowData->textBuffer.length;
			else if (windowData->cursorPosition < 0)
				windowData->cursorPosition = 0;
		}
		break;
	}
	case WM_MOUSELEAVE:
	{
		WindowData* windowData = (WindowData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

		windowData->prevHotUi = UI_ELEMENT::NONE;
		windowData->hotUi = UI_ELEMENT::NONE;
		windowData->activeUiOffset = { -1,-1 };
		// NOTE: we don't reset active ui after the mouse left the window
		// because it would prevent us from having some behaviour, like holding a slider and move it even if the mouse is outside of the window
		//windowData->activeUi = UI_ELEMENT::NONE;
		windowData->sumbitedUi = UI_ELEMENT::NONE;
		windowData->sumbitedOnAnyHotUi = UI_ELEMENT::NONE;
		break;
	}
	case WM_MOUSEMOVE:
	{
		//> It's required for WM_MOUSELEAVE
		TRACKMOUSEEVENT trackMouseEvent;

		trackMouseEvent.cbSize = sizeof(TRACKMOUSEEVENT);
		trackMouseEvent.hwndTrack = hwnd;
		trackMouseEvent.dwHoverTime = 0;
		trackMouseEvent.dwFlags = TME_LEAVE;

		TrackMouseEvent(&trackMouseEvent);
		//<

		WindowData* windowData = (WindowData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

		int xMouse = GET_X_LPARAM(lParam);
		int yMouse = GET_Y_LPARAM(lParam);

		yMouse = windowData->windowClientSize.y - yMouse;

		windowData->mousePosition = { xMouse, yMouse };
		windowData->mousePositionChanged = true;

		/*if (windowData->isRightButtonHold
			&& windowData->isTextEnteringMode
			&& IsInRect(windowData->textBlockOnClient, windowData->mousePosition))
		{
			OutputDebugString(L"ASD\n");
		}*/

		break;
	}
	case WM_SIZE:
	{
		WindowData* windowData = (WindowData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

		// NOTE: after window is create WM_SIZE message is recevied, since we init rendering after, we should ignore the first call
		if (windowData->windowDC == 0)
		{
			break;
		}
		RECT clientRect;
		GetClientRect(hwnd, &clientRect);
		windowData->windowClientSize.x = clientRect.right - clientRect.left;
		windowData->windowClientSize.y = clientRect.bottom - clientRect.top;

		windowData->windowBitmapInfo.bmiHeader.biWidth = windowData->windowClientSize.x;
		windowData->windowBitmapInfo.bmiHeader.biHeight = windowData->windowClientSize.y;

		RecreateBackgroundBmp(windowData);
		CalculateDrawingZoneSize(windowData);
		ValidateDrawingOffset(windowData);

		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

// TODO:
// 
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPWSTR pCmd, int windowMode)
{
	/*WideString test = WideString(L"YEAH");

	test.insert(4, L"_1");*/

	WNDCLASS windowClass = {};
	windowClass.hInstance = hInstance;
	windowClass.lpszClassName = L"window class";
	windowClass.lpfnWndProc = WindowCallback;
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);

	RegisterClass(&windowClass);

	WindowData windowData;
	InitWindowData(&windowData);

	QueryPerformanceFrequency(&windowData.perfomanceCounterFreq);

	HWND hwnd = CreateWindowExW(
		0,
		windowClass.lpszClassName,
		L"Paint Clone",
		WS_OVERLAPPEDWINDOW /* | CS_VREDRAW | CS_HREDRAW*/,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		0, 0,
		hInstance,
		&windowData
	);

	if (hwnd == 0)
	{
		return -1;
	}

	//> Remove fade in animation when window is opened up
	BOOL attrib = TRUE;
	DwmSetWindowAttribute(hwnd, DWMWA_TRANSITIONS_FORCEDISABLED, &attrib, sizeof(attrib));
	//<

	ShowWindow(hwnd, windowMode);

	InitRenderer(&windowData, hwnd);

	RasterizeTestingFontAndPutOnCanvas(&windowData);
	//CreateGlyphsLayout(&windowData, WideString(L"Y\r\na"));

	windowData.toolTiles = SimpleDynamicArray<ToolTile>(10);
	windowData.toolTiles.add(ToolTile(UI_ELEMENT::PENCIL_TOOL, DRAW_TOOL::PENCIL, LoadBmpFile(L"./pencil.bmp")));
	windowData.toolTiles.add(ToolTile(UI_ELEMENT::FILL_TOOL, DRAW_TOOL::FILL, LoadBmpFile(L"./fill.bmp")));
	windowData.toolTiles.add(ToolTile(UI_ELEMENT::ZOOM_TOOL, DRAW_TOOL::ZOOM, LoadBmpFile(L"./zoom.bmp")));
	windowData.toolTiles.add(ToolTile(UI_ELEMENT::TEXT_TOOL, DRAW_TOOL::TEXT, LoadBmpFile(L"./text.bmp")));

	windowData.brushColorTiles = SimpleDynamicArray<BrushColorTile>(10);
	windowData.brushColorTiles.add(BrushColorTile({ 0, 0, 0 }, UI_ELEMENT::COLOR_BRUCH_1));
	windowData.brushColorTiles.add(BrushColorTile({ 255, 255, 255 }, UI_ELEMENT::COLOR_BRUCH_2));
	windowData.brushColorTiles.add(BrushColorTile({ 255, 0, 0 }, UI_ELEMENT::COLOR_BRUCH_3));
	windowData.brushColorTiles.add(BrushColorTile({ 0, 255, 0 }, UI_ELEMENT::COLOR_BRUCH_4));
	windowData.brushColorTiles.add(BrushColorTile({ 0, 0, 255 }, UI_ELEMENT::COLOR_BRUCH_5));
	windowData.brushColorTiles.add(BrushColorTile({ 255, 255, 0 }, UI_ELEMENT::COLOR_BRUCH_6));
	windowData.brushColorTiles.add(BrushColorTile({ 0, 255, 255 }, UI_ELEMENT::COLOR_BRUCH_7));
	windowData.brushColorTiles.add(BrushColorTile({ 255, 0, 255 }, UI_ELEMENT::COLOR_BRUCH_8));
	//DrawLine(&windowData, { 500,500 }, { 100,100 });
	//DrawRect(&windowData, 0, 0, 50, 30, { (ubyte)12,(ubyte)12,(ubyte)12 });

	//DrawBitmap(&windowData, bmpTestImage.rgbaBitmap, { 300,300 }, bmpTestImage.size);
	//DrawBorderRect(&windowData, {0,0}, { 100,100 }, 2, { 120,120,49 });
	//FillFromPixel(&windowData, { 10,10 }, { 255, 0, 255 });
	//DrawBorderRect(&windowData, { 500, 10 }, { 100, 10 }, 4, { 0,0,0 });
	double deltasSum = 0.0f;
	int framesCount = 1;
	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}

		/*POINT mousePosition;
		GetCursorPos(&mousePosition);
		ScreenToClient(hwnd, &mousePosition);
		mousePosition.y = windowData.windowClientSize.y - mousePosition.y;
		windowData.mousePosition = { mousePosition.x, mousePosition.y };*/

		double timeDelta = GetCurrentTimestamp(&windowData);

		FillBitmapWithWhite(windowData.windowBitmap, windowData.windowClientSize);
		// ui
		DrawColorsBrush(&windowData, &windowData.brushColorTiles, { 5, 5 }, { 15, 15 }, 5);
		DrawToolsPanel(&windowData, { 5, 30 }, { 15, 15 }, 5);

		int2 drawingZoneSize = windowData.drawingZone.size();
		StretchDIBits(
			windowData.backgroundDC,
			windowData.drawingZone.x, windowData.windowClientSize.y - windowData.drawingZone.y - drawingZoneSize.y,
			drawingZoneSize.x, drawingZoneSize.y,

			(int)((float)windowData.drawingOffset.x / (float)windowData.drawingZoomLevel), (int)((float)windowData.drawingOffset.y / (float)windowData.drawingZoomLevel),
			(int)((float)drawingZoneSize.x / (float)windowData.drawingZoomLevel), (int)((float)drawingZoneSize.y / (float)windowData.drawingZoomLevel),

			windowData.drawingBitmap,
			&windowData.drawingBitmapInfo,
			DIB_RGB_COLORS, SRCCOPY
		);

		DrawDrawingCanvas(&windowData);
		DrawDraggableCornerOfDrawingZone(&windowData);

		DrawTextBufferToClient(&windowData);

		// NOTE: when charger is not connected to the laptop, it has around 10x slower performance!
		BitBlt(windowData.windowDC,
			0, 0, windowData.windowClientSize.x, windowData.windowClientSize.y,
			windowData.backgroundDC, 0, 0, SRCCOPY);

		HandleUiElements(&windowData);
		windowData.prevMousePosition = windowData.mousePosition;
		windowData.prevHotUi = windowData.hotUi;
		windowData.hotUi = UI_ELEMENT::NONE;

		// NOTE: at the end of frame we should clean mouse buttons state
		// otherwise we might get overspaming behaviour
		windowData.wasRightButtonPressed = false;
		windowData.wasRightButtonReleased = false;
		windowData.mousePositionChanged = false;

		timeDelta = GetCurrentTimestamp(&windowData) - timeDelta;
		char buff[100];
		sprintf_s(buff, "frame time: %f ml sec.\n", timeDelta * 1000.0f);
		//OutputDebugStringA(buff);
		deltasSum += timeDelta;
		framesCount++;
	}

	// 08.02.2024 average frame time - 2.21 ml sec. without charger connected to the laptop, 0.34 ml sec. with charger connected to the laptop
	double averageDelta = deltasSum / framesCount;
	char buff2[100];
	sprintf_s(buff2, "AVERAGE FRAME TIME: %f ml sec.\n", averageDelta * 1000.0f);
	OutputDebugStringA(buff2);

	DeleteDC(windowData.backgroundDC);
	ReleaseDC(hwnd, windowData.windowDC);
	//SelectObject(windowData.backgroundDC, objectHandler);
	windowData.brushColorTiles.freeMemory();
	windowData.toolTiles.freeMemory();

	return 0;
}