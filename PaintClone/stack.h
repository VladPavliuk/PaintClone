#pragma once

template<typename T>
struct Stack
{
	int length;

	int _capacity;
	int _singleElementSize;

	T* _elements;

	Stack()
	{}

	Stack(int initCapacity)
	{
		assert(initCapacity > 0);

		length = 0;
		_capacity = initCapacity;
		_singleElementSize = sizeof(T);

		_elements = (T*)malloc(initCapacity * _singleElementSize);
	}

	static Stack<T>* allocate(int initCapacity)
	{
		Stack<T>* stack = (Stack<T>*)malloc(sizeof(Stack<T>));
		*stack = Stack(initCapacity);

		return stack;
	}

	void enqueue(T item)
	{
		if (length >= _capacity)
		{
			_capacity *= 2;

			_elements = (T*)realloc(_elements, _singleElementSize * _capacity);
		}

		_elements[length] = item;
		length++;
	}

	T pop()
	{
		assert(length > 0);
		length--;
		return _elements[length];
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