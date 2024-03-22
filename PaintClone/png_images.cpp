#include "png_images.h"
#include "binary_tree.h"

void HuffmanCodingEncode(ubyte* input, int inputLength)
{
	ubyte bytesFrequency[255];
	ZeroMemory(bytesFrequency, 255);

	// find symbols frequency
	for (int i = 0; i < inputLength; i++)
	{
		//bytesFrequency[input[i]]++;
	}

	bytesFrequency['a'] = 5;
	bytesFrequency['b'] = 9;
	bytesFrequency['c'] = 12;
	bytesFrequency['d'] = 13;
	bytesFrequency['e'] = 16;
	bytesFrequency['f'] = 45;

	// sort symbols frequency
	SimpleDynamicArray<int2> frequency = SimpleDynamicArray<int2>(255);
	for (int i = 0; i < 255; i++)
	{
		if (bytesFrequency[i] > 0)
		{
			frequency.add({ i, bytesFrequency[i] });
		}
	}

	for (int i = 0; i < frequency.length - 1; i++)
	{
		for (int j = i + 1; j < frequency.length; j++)
		{
			if (frequency.get(i).y >= frequency.get(j).y)
			{
				int2 tmp = frequency.get(i);
				frequency.set(i, frequency.get(j));
				frequency.set(j, tmp);
			}
		}
	}

	// create initial nodes for tree from initial frequency values
	auto frequencyNodes = SimpleDynamicArray<BinaryNode<int2>*>(frequency.length);
	for (int i = 0; i < frequency.length; i++)
	{
		frequencyNodes.add(BinaryNode<int2>::allocate(frequency.get(i)));
	}
	frequency.freeMemory();

	//for (int j = 0; j < frequencyNodes.length; j++)
	//{
	//	char buff[100];
	//	sprintf_s(buff, "%c: %i\n", frequencyNodes.get(j)->value.x, frequencyNodes.get(j)->value.y);
	//	OutputDebugStringA(buff);
	//}

	// generate binary tree from bottom-up based on symbol's frequency
	while (frequencyNodes.length > 1)
	{
		// get 2 bottom symbols (with smallest frequency)
		auto leftNode = frequencyNodes.get(0);
		auto rightNode = frequencyNodes.get(1);

		int2 rootNodeValue = { -1, leftNode->value.y + rightNode->value.y };

		auto rootNode = BinaryNode<int2>::allocate(rootNodeValue);

		leftNode->root = rootNode;
		rightNode->root = rootNode;

		rootNode->left = leftNode;
		rootNode->right = rightNode;

		// remove these 2 smallest frequency symbols
		//TODO: create remove method that accepnts range to remove
		frequencyNodes.remove(1);
		frequencyNodes.remove(0);

		// insert internal node into frequency list
		bool foundLess = false;
		for (int i = 0; i < frequencyNodes.length; i++)
		{
			if (rootNodeValue.y < frequencyNodes.get(i)->value.y)
			{
				frequencyNodes.insert(i, rootNode);
				foundLess = true;
				break;
			}
		}

		if (!foundLess)
		{
			frequencyNodes.add(rootNode);
		}

		for (int j = 0; j < frequencyNodes.length; j++)
		{
			char buff[100];
			sprintf_s(buff, "%c: %i\n", frequencyNodes.get(j)->value.x, frequencyNodes.get(j)->value.y);
			OutputDebugStringA(buff);
		}

		OutputDebugStringA("\n");
	}

	BinaryNode<int2>* test = frequencyNodes.get(0);

	// print tree to console
	/*if (test->left != nullptr)
	{
		OutputDebugStringA("|");
	}*/

	// free frequencyNodes
	/*char buff[100];
	sprintf_s(buff, "%c: %i\n", input[i], tree[input[i]]);
	OutputDebugStringA(buff);*/
}

/*
	Each block consists of two parts: a pair of Huffman code trees that
	describe the representation of the compressed data part, and a
	compressed data part.

	The Huffman trees themselves are compressed using Huffman encoding.
*/
void DeflateDecode()
{

}

PngImage LoadPngFile(const wchar_t* fileName)
{
	//char test[] = "peter piper picked a picked pepper";
	//char test[] = "peter";
	//HuffmanCodingEncode((ubyte*)test, (int)strlen(test));
	//
	ubyte* pngFileBuffer;
	int pngFileSize;
	ReadFileIntoBuffer(fileName, &pngFileBuffer, &pngFileSize);
	ubyte* pngFileBufferStart = pngFileBuffer;

	//uint64 pngSignature = _byteswap_ulong(*(uint64*)pngFileBuffer);
	uint64 pngSignature = *(uint64*)pngFileBuffer;

	if (pngSignature != 0x0a1a0a0d474e5089)
	{
		assert(false);
	}

	pngFileBuffer += sizeof(uint64);

	// skip length of ihdr chunk
	//pngFileBuffer += sizeof(uint);

	PngIHDRChunk ihdrChunk = *(PngIHDRChunk*)pngFileBuffer;

	ihdrChunk.signature = _byteswap_ulong(ihdrChunk.signature);
	ihdrChunk.width = _byteswap_ulong(ihdrChunk.width);
	ihdrChunk.height = _byteswap_ulong(ihdrChunk.height);

	if (ihdrChunk.signature != 0x49484452)
	{
		assert(false);
	}
	pngFileBuffer += sizeof(PngIHDRChunk);

	PngIDATChunk idatChunk = *(PngIDATChunk*)pngFileBuffer;

	idatChunk.length = _byteswap_ulong(idatChunk.length);
	idatChunk.signature = _byteswap_ulong(idatChunk.signature);

	if (idatChunk.signature != 0x49444154)
	{
		assert(false);
	}

	pngFileBuffer += sizeof(idatChunk.length) + sizeof(idatChunk.signature);

	idatChunk.content = (ubyte*)malloc(idatChunk.length);
	memcpy(idatChunk.content, pngFileBuffer, idatChunk.length);
	pngFileBuffer += idatChunk.length;
	pngFileBuffer += sizeof(idatChunk.crc);

	PngIENDChunk iendChunk = *(PngIENDChunk*)pngFileBuffer;

	iendChunk.length = _byteswap_ulong(iendChunk.length);
	iendChunk.signature = _byteswap_ulong(iendChunk.signature);

	if (iendChunk.signature != 0x49454e44)
	{
		assert(false);
	}

	//> compression
	// 78 01 - No Compression/low
	// 78 5E - Fast Compression
	// 78 9C - Default Compression
	// 78 DA - Best Compression



	//<

	free(idatChunk.content);
	free(pngFileBufferStart);
	return PngImage();
}
