#pragma once

#include "dynamic_array.h"

template<typename T, typename H>
struct HashTableItem
{
	HashTableItem()
	{}

	HashTableItem(T key, H value) :
		key(key), value(value)
	{}

	T key;
	H value;
};


//template<typename T, typename H>
//struct HashTable
//{
//
//};

template<typename T>
struct HashTable
{
	/*int _keySize;
	int _valueSize;*/
	int _capacity;

	HashTableItem<int, T>* _elements;
	SimpleDynamicArray<HashTableItem<int, T>>* _buckets;
	bool* _occupiedIndexes;

	int _currentIterationIndex;
	int _currentIterationIndexInBucket;

	HashTable(int capacity)
	{
		/*	_keySize = sizeof(T);
			_valueSize = sizeof(H);*/
		_capacity = capacity;

		_elements = (HashTableItem<int, T>*)malloc(capacity * sizeof(HashTableItem<int, T>));
		_buckets = (SimpleDynamicArray<HashTableItem<int, T>>*)malloc(capacity * sizeof(SimpleDynamicArray<HashTableItem<int, T>>));
		ZeroMemory(_buckets, capacity * sizeof(SimpleDynamicArray<HashTableItem<int, T>>));
		_occupiedIndexes = (bool*)malloc(capacity);
		ZeroMemory(_occupiedIndexes, capacity);
	}

	void resetIteration()
	{
		_currentIterationIndex = 0;
		_currentIterationIndexInBucket = -1;
	}

	bool getNext(HashTableItem<int, T>* item)
	{
		bool foundAny = false;

		for (; _currentIterationIndex < _capacity; _currentIterationIndex++)
		{
			if (!_occupiedIndexes[_currentIterationIndex])
			{
				continue;
			}

			if (_currentIterationIndexInBucket == -1)
			{
				*item = _elements[_currentIterationIndex];
				
				if (_buckets[_currentIterationIndex]._elements != NULL)
				{
					_currentIterationIndexInBucket = 0;
				}
				else
				{
					_currentIterationIndex++;
				}
				foundAny = true;
				break;
			}

			if (_currentIterationIndexInBucket < _buckets[_currentIterationIndex].length)
			{
				*item = _buckets[_currentIterationIndex].get(_currentIterationIndexInBucket);

				foundAny = true;
				_currentIterationIndexInBucket++;
				break;
			}
			else
			{
				_currentIterationIndexInBucket = -1;
			}
		}

		return foundAny;
	}

	void set(int key, T value)
	{
		int index = _getIndex(key);

		if (!_occupiedIndexes[index])
		{
			_occupiedIndexes[index] = true;
			_elements[index] = HashTableItem<int, T>(key, value);
			return;
		}

		if (_elements[index].key == key)
		{
			_elements[index] = HashTableItem<int, T>(key, value);
			return;
		}

		if (_buckets[index]._elements == NULL)
		{
			_buckets[index] = SimpleDynamicArray<HashTableItem<int, T>>(2);
			_buckets[index].add(HashTableItem<int, T>(key, value));
			return;
		}

		bool keyAlreadyInBucket = false;

		for (int i = 0; i < _buckets[index].length; i++)
		{
			HashTableItem<int, T>* bucketItem = _buckets[index].getPointer(i);
			if (bucketItem->key == key)
			{
				bucketItem->value = value;
				keyAlreadyInBucket = true;
				break;
			}
		}

		if (!keyAlreadyInBucket)
		{
			_buckets[index].add(HashTableItem<int, T>(key, value));
		}
	}

	T get(int key)
	{
		int index = _getIndex(key);

		if (!_occupiedIndexes[index])
		{
			assert(false);
		}

		if (_elements[index].key == key)
		{
			return _elements[index].value;
		}

		if (_buckets[index]._elements == NULL)
		{
			assert(false);
			T empty;
			ZeroMemory(&empty, sizeof(empty));
			return empty;
		}

		for (int i = 0; i < _buckets[index].length; i++)
		{
			HashTableItem<int, T> bucketItem = _buckets[index].get(i);

			if (bucketItem.key == key)
			{
				return bucketItem.value;
			}
		}

		assert(false);

		T empty;
		ZeroMemory(&empty, sizeof(empty));
		return empty;
	}

	int _getIndex(int key)
	{
		return key % _capacity;
	}

	void freeMemory()
	{
		for (int i = 0; i < _capacity; i++)
		{
			if (_buckets[i]._elements != NULL)
			{
				_buckets[i].freeMemory();
			}
		}

		free(_elements);
		free(_buckets);
		free(_occupiedIndexes);
	}
};