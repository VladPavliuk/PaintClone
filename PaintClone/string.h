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

	WideString(int _length)
	{
		length = _length;

		chars = (wchar_t*)malloc(sizeof(wchar_t) * (length + 1));
		ZeroMemory(chars, sizeof(wchar_t) * (length + 1));
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

	void append(int number)
	{
		bool isNegative = number < 0;

		if (isNegative) number = absInt(number);

		WideString numberString = WideString(L"");
		do
		{
			int digit = number % 10;
			wchar_t digitString = wchar_t(48 + digit); // 48 is 0 char in ascii table

			numberString.append(digitString);
			number /= 10;
		} while (number != 0);

		if (isNegative) append(L'-');

		for (int i = numberString.length - 1; i >= 0; i--)
		{
			append(numberString.chars[i]);
		}

		numberString.freeMemory();
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

	void removeChar(wchar_t charToRemove)
	{
		for (int i = length - 1; i >= 0; i--)
		{
			if (chars[i] == charToRemove)
			{
				removeByIndex(i);
			}
		}
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

	void insert(int index, const wchar_t* stringToInsert)
	{
		assert(length >= index);
		assert(stringToInsert != 0);

		int stringToInsertLength = (int)wcslen(stringToInsert);
		assert(stringToInsertLength > 0);

		int newLength = length + stringToInsertLength;

		wchar_t* newChars = (wchar_t*)malloc((newLength + 1) * sizeof(wchar_t));

		if (index > 0)
		{
			memcpy(newChars, chars, index * sizeof(wchar_t));
		}

		memcpy(newChars + index, stringToInsert, stringToInsertLength * sizeof(wchar_t));

		memcpy(newChars + index + stringToInsertLength, chars + index, (length - index) * sizeof(wchar_t));

		length = newLength;
		newChars[length] = L'\0';

		free(chars);
		chars = newChars;
	}

	void removeRange(int fromIndex, int count)
	{
		assert(fromIndex >= 0);
		assert(length >= fromIndex + count);
		memcpy(chars + fromIndex, chars + fromIndex + count, sizeof(wchar_t) * (length - (fromIndex + count)));
		length -= count;
		chars[length] = L'\0';
	}

	WideString substring(int fromIndex, int count)
	{
		assert(length >= fromIndex + count);
		WideString substring = WideString(count);

		for (int i = 0; i < count; i++)
		{
			substring.chars[i] = chars[fromIndex + i];
		}

		return substring;
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