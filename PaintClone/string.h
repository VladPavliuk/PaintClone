#pragma once

#include <corecrt_wstring.h>
#include <corecrt_malloc.h>
#include "math.h"

struct WideString
{
	wchar_t* chars;
	int length;

	WideString()
	{
		chars = NULL;
		length = -1;
	}

	WideString(const wchar_t* initString)
	{
		_initStringValue(initString);
	}

	void _initStringValue(const wchar_t* initString)
	{
		length = (int)wcslen(initString);
		//capacity = maxInt(length, 1); // we don't allow to create an array of chars that have 0 length

		chars = (wchar_t*)malloc(sizeof(wchar_t) * (length + 1));
		memcpy(chars, initString, sizeof(wchar_t) * length);
		chars[length] = L'\0';
	}

	void initEmptyString()
	{
		chars = (wchar_t*)malloc(sizeof(wchar_t));
		chars[0] = L'\0';
		length = 0;
	}

	void freeMemory() {
		if (chars == NULL)
		{
			return;
		}

		free(chars);

		chars = NULL;
		length = -1;
	}

	void clear()
	{
		if (chars != NULL)
		{
			free(chars);
		}

		initEmptyString();
	}

	void append(wchar_t charToAppend)
	{
		if (chars == NULL)
		{
			length = 1;
			chars = (wchar_t*)malloc(sizeof(wchar_t) * 2);
			memcpy(chars, &charToAppend, sizeof(wchar_t));
			chars[length] = L'\0';
			return;
		}

		int oldLength = length;
		length++;
		chars = (wchar_t*)realloc(chars, sizeof(wchar_t) * (length + 1));
		memcpy(chars + oldLength, &charToAppend, sizeof(wchar_t));
		chars[length] = L'\0';
	}

	void append(const wchar_t* stringToAppend)
	{
		int lengthToAppend = (int)wcslen(stringToAppend);

		assert(lengthToAppend > 0);

		if (chars == NULL)
		{
			_initStringValue(stringToAppend);
			return;
		}

		int oldLength = length;
		length += lengthToAppend;
		chars = (wchar_t*)realloc(chars, sizeof(wchar_t) * (length + 1));
		memcpy(chars + oldLength, stringToAppend, sizeof(wchar_t) * lengthToAppend);
		chars[length] = L'\0';
	}

	void insert(int index, wchar_t charToInsert)
	{
		assert(index >= 0);
		assert(index <= length);

		length++;
		chars = (wchar_t*)realloc(chars, sizeof(wchar_t) * (length + 1));

		memcpy(chars + index + 1, chars + index, sizeof(wchar_t) * (length - index));
		chars[index] = charToInsert;
	}

	void removeLast()
	{
		if (chars == NULL)
		{
			return;
		}

		if (length == 0)
		{
			return;
		}

		length--;

		chars = (wchar_t*)realloc(chars, sizeof(wchar_t) * (length + 1));
		chars[length] = L'\0';
	}

	void removeByIndex(int index)
	{
		assert(index >= 0);
		if (index >= length)
		{
			return;
		}

		memcpy(chars + index, chars + index + 1, sizeof(wchar_t) * (length - index));

		length--;
		chars[length] = L'\0';
	}
};