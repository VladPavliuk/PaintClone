#include "files.h"

void ReadFileIntoBuffer(const wchar_t* fileName, ubyte** buffer, int* bufferSize)
{
	FILE* file;
	int err = _wfopen_s(&file, fileName, L"rb");

	assert(err == 0);
	assert(file != 0);

	fseek(file, 0, SEEK_END);
	*bufferSize = ftell(file);
	fseek(file, 0, SEEK_SET);

	*buffer = (ubyte*)malloc(*bufferSize);

	assert(*buffer != 0);

	int bytesRead = fread_s(*buffer, *bufferSize, 1, *bufferSize, file);

	assert(bytesRead == *bufferSize);

	fclose(file);
}
