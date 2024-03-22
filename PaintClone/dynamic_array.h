#pragma once

#include <assert.h>
#include <windows.h>

template<typename T>
struct SimpleDynamicArray
{
	int length;

	int _capacity;
	int _singleElementSize;

	T* _elements;

	SimpleDynamicArray()
	{
		//_elements = NULL;
	}

	SimpleDynamicArray(int initCapacity)
	{
		assert(initCapacity > 0);

		length = 0;
		_capacity = initCapacity;
		_singleElementSize = sizeof(T);

		_elements = (T*)malloc(initCapacity * _singleElementSize);
		ZeroMemory(_elements, initCapacity * _singleElementSize);
	}

	static SimpleDynamicArray<T>* allocate(int initCapacity)
	{
		assert(initCapacity > 0);

		SimpleDynamicArray<T>* dynamicArray = (SimpleDynamicArray<T>*)malloc(sizeof(SimpleDynamicArray<T>));
		SimpleDynamicArray<T> localDynamicArray = SimpleDynamicArray<T>(initCapacity);

		memcpy(dynamicArray, &localDynamicArray, sizeof(SimpleDynamicArray<T>));

		return dynamicArray;
	}

	void add(T item)
	{
		if (length >= _capacity)
		{
			_capacity *= 2;

			_elements = (T*)realloc(_elements, _singleElementSize * _capacity);
			ZeroMemory(_elements + _capacity / 2, _singleElementSize * _capacity / 2);
		}

		_elements[length] = item;

		length++;
	}

	void insert(int index, T item)
	{
		assert(index < _capacity);
		assert(index >= 0);

		if (index >= length)
		{
			set(index, item);
			return;
		}

		length++;
		if (length >= _capacity)
		{
			_capacity *= 2;

			_elements = (T*)realloc(_elements, _singleElementSize * _capacity);
			ZeroMemory(_elements + _capacity / 2, _singleElementSize * _capacity / 2);
		}

		memcpy(_elements + index + 1, _elements + index, _singleElementSize * ((length - 1) - index));
		_elements[index] = item;
	}

	void set(int index, T item)
	{
		assert(index < _capacity);
		assert(index >= 0);

		if (index >= length)
		{
			length = index + 1;
		}
		_elements[index] = item;
	}

	T get(int index)
	{
		assert(index < length);
		assert(index >= 0);

		return _elements[index];
	}

	T* getPointer(int index)
	{
		assert(index < length);
		assert(index >= 0);

		return _elements + index;
	}

	void remove(int index)
	{
		assert(index < length);
		assert(index >= 0);

		memcpy(_elements + index, _elements + (index + 1), _singleElementSize * ((length - 1) - index));
		length--;
	}

	void clear()
	{
		if (length == 0)
		{
			return;
		}

		ZeroMemory(_elements, _singleElementSize * length);
		length = 0;
	}

	void freeMemory()
	{
		free(_elements);

		_elements = NULL;

		length = 0;
	}
};

template<typename T>
struct DynamicArray
{
	int length;

	int _capacity;
	int _singleElementSize;

	T* _elements;
	bool* _occupiedIndexes;

	DynamicArray()
	{}

	DynamicArray(int initCapacity)
	{
		assert(initCapacity > 0);

		length = 0;
		_capacity = initCapacity;
		_singleElementSize = sizeof(T);

		_elements = (T*)malloc(initCapacity * _singleElementSize);
		ZeroMemory(_elements, initCapacity * _singleElementSize);

		_occupiedIndexes = (bool*)malloc(initCapacity);
		ZeroMemory(_occupiedIndexes, initCapacity);
	}

	static DynamicArray<T>* allocate(int initCapacity)
	{
		assert(initCapacity > 0);

		DynamicArray<T>* dynamicArray = (DynamicArray<T>*)malloc(sizeof(DynamicArray<T>));
		DynamicArray<T> localDynamicArray = DynamicArray<T>(initCapacity);

		memcpy(dynamicArray, &localDynamicArray, sizeof(DynamicArray<T>));

		return dynamicArray;
	}

	void add(T item)
	{
		if (length >= _capacity)
		{
			_capacity *= 2;

			_elements = (T*)realloc(_elements, _singleElementSize * _capacity);
			ZeroMemory(_elements + _capacity / 2, _singleElementSize * _capacity / 2);

			_occupiedIndexes = (bool*)realloc(_occupiedIndexes, _capacity);
			ZeroMemory(_occupiedIndexes + _capacity / 2, _capacity / 2);
		}

		_elements[length] = item;
		_occupiedIndexes[length] = true;

		length++;
	}

	void set(int index, T item)
	{
		assert(index < _capacity);
		assert(index >= 0);

		if (index >= length)
		{
			length = index + 1;
		}
		_elements[index] = item;
		_occupiedIndexes[index] = true;
	}

	T get(int index)
	{
		assert(index < length);
		assert(index >= 0);
		assert(_occupiedIndexes[index]);

		return _elements[index];
	}

	T* getPointer(int index)
	{
		assert(index < length);
		assert(index >= 0);
		assert(_occupiedIndexes[index]);

		return _elements + index;
	}

	bool has(int index)
	{
		//assert(index < length);
		assert(index >= 0);

		return _occupiedIndexes[index];
	}

	void remove(int index)
	{
		assert(index < length);
		assert(index >= 0);

		ZeroMemory(_elements + index, _singleElementSize);
		_occupiedIndexes[index] = false;
	}

	DynamicArray<T> copy()
	{
		DynamicArray<T> tmp = DynamicArray<T>(_capacity);

		tmp.length = length;

		memcpy(tmp._elements, _elements, _capacity * _singleElementSize);
		memcpy(tmp._occupiedIndexes, _occupiedIndexes, _capacity);

		return tmp;
	}

	void freeMemory()
	{
		if (_elements == NULL && _occupiedIndexes == NULL)
		{
			return;
		}
		free(_elements);
		free(_occupiedIndexes);

		_elements = NULL;
		_occupiedIndexes = NULL;

		length = 0;
	}
};