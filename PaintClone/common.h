#pragma once

#include <vcruntime_string.h>
#include "dynamic_array.h"
#include "string.h"

bool CompareStrings(const char* first, int firstLength, const char* second, int secondLength);

void BubbleSort(SimpleDynamicArray<int>* arr);
void BubbleSort(SimpleDynamicArray<float>* arr);

void PutTextIntoClipboard(HWND hwnd, WideString text);
wchar_t* GetTextFromClipBoard(HWND hwnd);