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
