#include "common.h"

bool CompareStrings(const char* first, int firstLength, const char* second, int secondLength)
{
	if (firstLength != secondLength) return false;

	return memcmp(first, second, secondLength) == 0;
}

void BubbleSort(SimpleDynamicArray<int>* arr)
{
	for (int i = 0; i < arr->length - 1; i++)
	{
		for (int j = i + 1; j < arr->length; j++)
		{
			int* a = arr->getPointer(i);
			int* b = arr->getPointer(j);

			if (*a > *b) {
				*a += *b;
				*b = *a - *b;
				*a -= *b;
			}
		}
	}
}

void BubbleSort(SimpleDynamicArray<float>* arr)
{
	for (int i = 0; i < arr->length - 1; i++)
	{
		for (int j = i + 1; j < arr->length; j++)
		{
			float* a = arr->getPointer(i);
			float* b = arr->getPointer(j);

			if (*a > *b) {
				*a += *b;
				*b = *a - *b;
				*a -= *b;
			}
		}
	}
}

void PutTextIntoClipboard(HWND hwnd, WideString text)
{
	if (!IsClipboardFormatAvailable(CF_UNICODETEXT))
	{
		DWORD test = GetLastError();
		return;
	}

	if (!OpenClipboard(hwnd))
	{
		DWORD test = GetLastError();
		return;
	}
	EmptyClipboard();

	int textLength = (text.length + 1) * sizeof(wchar_t);
	HGLOBAL globalMemoryHandler = GlobalAlloc(GMEM_MOVEABLE, textLength);

	wchar_t* globalMemory = (wchar_t*)GlobalLock(globalMemoryHandler);

	memcpy(globalMemory, text.chars, textLength);
	GlobalUnlock(globalMemoryHandler);

	SetClipboardData(CF_UNICODETEXT, globalMemoryHandler);
	CloseClipboard();
	//GlobalFree(globalMemoryHandler);
}

wchar_t* GetTextFromClipBoard(HWND hwnd)
{
	if (!IsClipboardFormatAvailable(CF_UNICODETEXT) || !OpenClipboard(hwnd))
	{
		return 0;
	}

	HANDLE clipboardHandle = GetClipboardData(CF_UNICODETEXT);

	if (clipboardHandle == 0)
	{
		return 0;
	}

	wchar_t* data = (wchar_t*)GlobalLock(clipboardHandle);

	GlobalUnlock(clipboardHandle);
	CloseClipboard();

	return data;
}
