#pragma once

template<typename T>
struct BinaryNode
{
	BinaryNode* root;
	BinaryNode* left;
	BinaryNode* right;
	T value;

	BinaryNode(T _value)
	{
		value = _value;
		root = nullptr;
		left = nullptr;
		right = nullptr;
	}

	static BinaryNode* allocate(T value)
	{
		BinaryNode* node = (BinaryNode*)malloc(sizeof(BinaryNode<T>));
		*node = BinaryNode(value);

		return node;
	}
};

template<typename T>
struct BinaryTree
{

};