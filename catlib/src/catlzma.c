#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

// clang catlzma.c deps/lzma/*.c -march=armv8-a+crc -lpthread

#include "catlzma.h"

#define DICT_MAX_SIZE 1 << 24
#define DECOMP_BUFSIZE 65536

#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

static void * AllocForLzma(ISzAllocPtr p, size_t size) { return malloc(size); }
static void FreeForLzma(ISzAllocPtr p, void* address) { free(address); }
static ISzAlloc SzAllocForLzma = { &AllocForLzma, &FreeForLzma };

typedef struct {
  ISeqInStream stream;
  Byte* buf;
  unsigned bufPos;
  SizeT bufLen;
} CAT_CompressionIn;

SRes CAT_CompIn_Read(void *p, void *buf, size_t* size) {
	CAT_CompressionIn* ctx = (CAT_CompressionIn*)p;
	*size = min(*size, ctx->bufLen - ctx->bufPos);

	if (*size > 0)
		memcpy(buf, &(ctx->buf[ctx->bufPos]), *size);
	ctx->bufPos += *size;
	return SZ_OK;
}

typedef struct {
	ISeqOutStream stream;
	Byte* buf;
	unsigned bufPos;
	SizeT curSz;
	int errCode; // more elaborate error code than the generic SZ_ERROR_WRITE that lzmasdk returns
} CAT_CompressionOut;

typedef size_t (*streamWriteFn)(const ISeqOutStream *p, const void *buf, size_t size);
typedef SRes (*streamReadFn)(const ISeqInStream *p, void *buf, size_t *size);

static void printBytes(Byte* arr, int n) {
	for (int i = 0; i < n; i++) {
		printf("%02X", arr[i]);
	}
	printf("\n");
}

size_t CAT_CompOut_Write(void *p, const void *buf, size_t size) {
	CAT_CompressionOut *ctx = (CAT_CompressionOut*)p;
	if (!size) return size;

	unsigned oldSize = ctx->curSz;

	void* newMem = realloc(ctx->buf, oldSize + size);

	if (newMem == NULL) { // Failed to reallocate; set errCode and free the current buf
		ctx->errCode = SZ_ERROR_MEM;
		free(ctx->buf);
		return 0;
	}

	ctx->buf = newMem;
	ctx->curSz += size;
	memcpy(&(ctx->buf[ctx->bufPos]), buf, size);
	ctx->bufPos += size;

	return size;
}

SRes CAT_Compress(CAT_CompData* in, CAT_CompData* out) {
	CLzmaEncHandle enc = LzmaEnc_Create(&SzAllocForLzma);
	if (enc == NULL || !enc) {
		return SZ_ERROR_MEM;
	}

	CLzmaEncProps props;
	LzmaEncProps_Init(&props);
	props.writeEndMark = 1;

	SRes res = LzmaEnc_SetProps(enc, &props);
	if (res != SZ_OK) {
		LzmaEnc_Destroy(enc, &SzAllocForLzma, &SzAllocForLzma);
		return res;
	}

	SizeT propsSize = LZMA_PROPS_SIZE;
	Byte* outBuf = (Byte*)malloc(propsSize);

	res = LzmaEnc_WriteProperties(enc, &outBuf[0], &propsSize);
	assert(propsSize == LZMA_PROPS_SIZE);
	if (res != SZ_OK) {
		LzmaEnc_Destroy(enc, &SzAllocForLzma, &SzAllocForLzma);
		return res;
	}

	CAT_CompressionIn inStream = {
		.stream 	= (streamReadFn)&CAT_CompIn_Read,
		.buf 		= in->buf,
		.bufPos 	= 0,
		.bufLen 	= in->bufSz
	};

	CAT_CompressionOut outStream = {
		.stream 	= (streamWriteFn)&CAT_CompOut_Write,
		.buf 		= outBuf,
		.bufPos 	= propsSize,
		.curSz 		= propsSize,
		.errCode 	= SZ_OK
	};

	res = LzmaEnc_Encode(enc,
		(ISeqOutStream*)&outStream, (ISeqInStream*)&inStream,
		0, &SzAllocForLzma, &SzAllocForLzma);

	LzmaEnc_Destroy(enc, &SzAllocForLzma, &SzAllocForLzma);

	// error came during CAT_CompOut_Write; return our custom errCode
	if (res == SZ_ERROR_WRITE && outStream.errCode != SZ_OK) {
		return outStream.errCode;
	}

	// error came from elsewhere (LZMA SDK?); just return that
	if (res != SZ_OK) {
		return res;
	}

	out->buf = outStream.buf;
	out->bufSz = outStream.curSz;

	return res;
}

// out->bufSz must contain the uncompressed size
SRes CAT_Decompress(CAT_CompData* in, CAT_CompData* out) {
	assert(out->bufSz > 0);

	CLzmaDec dec;
	LzmaDec_Construct(&dec);

	SRes res = LzmaDec_Allocate(&dec, in->buf, LZMA_PROPS_SIZE, &SzAllocForLzma);
	if (res != SZ_OK) {
		return res;
	}

	LzmaDec_Init(&dec);

	Byte* outBuf = malloc(out->bufSz);

	unsigned outPos = 0, inPos = LZMA_PROPS_SIZE;
	ELzmaStatus status;
	const unsigned BUF_SIZE = DECOMP_BUFSIZE;

	int finished = 0;

	SizeT destLeft = out->bufSz;

	while (inPos < in->bufSz) {
		SizeT destDec = destLeft - outPos;
		SizeT srcLen  = min(BUF_SIZE, in->bufSz - inPos);

		res = LzmaDec_DecodeToBuf(&dec,
			&outBuf[outPos], &destDec,
			&(in->buf[inPos]), &srcLen,
			LZMA_FINISH_END, &status);

		if (res != SZ_OK) {
			free(outBuf);
			LzmaDec_Free(&dec, &SzAllocForLzma);
			return res;
		}

		inPos += srcLen;
		outPos += destDec;
		if (status == LZMA_STATUS_FINISHED_WITH_MARK) {
			break;
		}
	}

	LzmaDec_Free(&dec, &SzAllocForLzma);

	if (outPos < out->bufSz) {
		Byte* newOut = realloc(outBuf, outPos);
		if (newOut == NULL) { // uhhhhhh????? ok lmao not like i wanted to give you memory or anything
			out->bufSz = outPos;
			out->buf = outBuf;
			return SZ_OK;
		}
		outBuf = newOut;
		out->bufSz = outPos;
	}

	out->buf = outBuf;
	return SZ_OK;
}

const char* CAT_CompressErrorCode(int code) {
	const char* ret = "Unrecognized error!?";

	switch (code) {
		case (SZ_ERROR_MEM):
			ret = "Can't allocate required memory.";
			break;

		case (SZ_ERROR_WRITE):
			ret = "Internal error while writing to compression stream.";
			break;

		case (SZ_ERROR_PARAM):
			ret = "Incorrect compression parameters.";
			break;
	}

	return ret;
}

/*
int main() {
	char str[] = "hello hi Hello World :):)\0OhNo A Null ! ! !";
	size_t sz = sizeof(str);

	printf("Compressing %p\n", str);

	CAT_CompData in = { str, sz };
	CAT_CompData out;

	SRes status = CAT_Compress(&in, &out);
	printf("compression status: %d, len: %d\n", status, out.bufSz);

	if (status != SZ_OK) {
		printf("non-ok compression status, error: %s; exiting.", CAT_CompressErrorCode(status));
		return -1;
	}

	CAT_CompData outDecomp = { NULL, sz };

	status = CAT_Decompress(&out, &outDecomp);
	if (status != SZ_OK) {
		printf("non-ok decompression status, error: %s; exiting.", CAT_CompressErrorCode(status));
		return -1;
	}

	printf("decompression status: %d (%d -> ", status, outDecomp.bufSz);
	for (int i = 0; i < sz; i++) {
		if (outDecomp.buf[i] == 0)
			printf("\\0");
		else
			printf("%c", outDecomp.buf[i]);
	}
	printf(")\n");

	// CAT_FreeCompress(in);
	free(out.buf);
	free(outDecomp.buf);
}
*/