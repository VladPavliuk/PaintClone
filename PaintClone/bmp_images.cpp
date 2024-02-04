#include "bmp_images.h"

BmpImage LoadBmpFile(const wchar_t* bmpFilePath)
{
	ubyte* bmpFileBuffer;
	int bmpFileSize;
	ReadFileIntoBuffer(bmpFilePath, &bmpFileBuffer, &bmpFileSize);

	ubyte* bmpFileBufferStart = bmpFileBuffer;

	BmpFileHeader bmpFileHeader;
	memcpy_s(&bmpFileHeader, sizeof(BmpFileHeader), bmpFileBuffer, sizeof(BmpFileHeader));
	assert(bmpFileSize == bmpFileHeader.fileSize);

	bmpFileBuffer += sizeof(BmpFileHeader);

	BmpDIBHeader bmpDIBHeader;
	memcpy_s(&bmpDIBHeader, sizeof(BmpDIBHeader), bmpFileBuffer, sizeof(BmpDIBHeader));
	assert(bmpDIBHeader.dibHeaderSize == 40);

	assert(sizeof(BmpFileHeader) + sizeof(BmpDIBHeader) == bmpFileHeader.offsetToBitmap);

	bmpFileBuffer += sizeof(BmpDIBHeader);
	BmpImage bmpImage;

	// if no compression just return bitmap
	if (bmpDIBHeader.compression == 0)
	{
		bmpImage.size = { bmpDIBHeader.imageWidth, bmpDIBHeader.imageHeight };
		//long pixelsCount = bmpImage.size.x * bmpImage.size.y;

		bmpImage.rgbaBitmap = (ubyte4*)malloc(sizeof(ubyte4) * bmpImage.size.x * bmpImage.size.y);
		if (bmpDIBHeader.bitsPerPixel == 24) // bgr
		{
			// NOTE: in bmp format all bitmap rows are in 4 byts lines
			// so if we have 3 bytes (24-bits) image, we might have a situation
			// when some empty bytes will be added to each row and we just have to skip them
			int rowRightPadding = (4 - ((3 * bmpImage.size.x) % 4)) % 4;

			for (int y = 0; y < bmpImage.size.y; y++)
			{
				int pitch = y * bmpImage.size.x;
				for (int x = 0; x < bmpImage.size.x; x++)
				{
					bmpImage.rgbaBitmap[pitch + x] = ubyte4(
						*(bmpFileBuffer),
						*(bmpFileBuffer + 1),
						*(bmpFileBuffer + 2),
						0
					);
					bmpFileBuffer += 3;
				}

				bmpFileBuffer += rowRightPadding;
			}
		}
		else if (bmpDIBHeader.bitsPerPixel == 32) // bgra
		{

		}
	}

	free(bmpFileBufferStart);
	return bmpImage;
}
