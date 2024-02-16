#pragma once

#include <vcruntime_string.h>
#include "dynamic_array.h"

bool CompareStrings(const char* first, int firstLength, const char* second, int secondLength);

void BubbleSort(SimpleDynamicArray<int>* arr);