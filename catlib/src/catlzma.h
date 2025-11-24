#pragma once

#include "deps/lzma/LzmaEnc.h"
#include "deps/lzma/LzmaDec.h"

typedef struct {
	Byte* buf;
	SizeT bufSz;
} CAT_CompData;

SRes CAT_Compress(CAT_CompData* in, CAT_CompData* out);
SRes CAT_Decompress(CAT_CompData* in, CAT_CompData* out);