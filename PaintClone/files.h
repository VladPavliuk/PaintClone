#pragma once

#include <cstdio>
#include <cassert>
#include <corecrt_malloc.h>

#include "custom_types.h"

void ReadFileIntoBuffer(const wchar_t* fileName, ubyte** buffer, int* bufferSize);