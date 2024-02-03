#pragma once

template<typename T>
struct Queue
{
	int length;

	int _capacity;
	int _singleElementSize;

	T* _elements;

	Queue(int initCapacity)
	{
		assert(initCapacity > 0);

		length = 0;
		_capacity = initCapacity;
		_singleElementSize = sizeof(T);

		_elements = (T*)malloc(initCapacity * _singleElementSize);
		ZeroMemory(_elements, initCapacity * _singleElementSize);
	}

	void enqueue(T item)
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

	T pop()
	{
		assert(length > 0);
		length--;
		T item = _elements[0];

		memcpy(_elements, _elements + 1, length * _singleElementSize);

		return item;
	}

	bool empty()
	{
		return length == 0;
	}

	void freeMemory()
	{
		if (_elements != NULL)
		{
			free(_elements);
		}
	}
};