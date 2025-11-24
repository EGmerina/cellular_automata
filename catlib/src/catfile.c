#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "catglobals.h"
#include "catfile.h"
#include "catlzma.h"
#include "catlib.h"
#include "catfragment.h"
#include "deps/md5.h"

void CAT_MD5(unsigned int arraySize, caFileHeader* header, char *ca, MD5Hash* out) {
	MD5Context ctx;
	md5Init(&ctx);

	md5Update(&ctx, (uint8_t*)header, sizeof(caFileHeader));
	md5Update(&ctx, (uint8_t*)ca, arraySize);

	md5Finalize(&ctx);

	memcpy(out->hash, ctx.digest, sizeof(out->hash));
}

void CAT_ToMD5Hash(MD5Context* ctx, MD5Hash* out) {
	memcpy(out->hash, ctx->digest, sizeof(ctx->digest));
}

void printMD5(MD5Hash* h) {
	printf("md5: ");
	for (int i = 0; i < sizeof(h->hash); i++) {
		printf("%02X", h->hash[i]);
	}
	printf("\n");
}

/*
void printMD5(MD5Context* h) {
	printf("md5: ");
	for (int i = 0; i < sizeof(h->digest); i++) {
		printf("%02X", h->digest[i]);
	}
	printf("\n");
}
*/

static void printBytes(Byte* arr, int n) {
	for (int i = 0; i < n; i++) {
		printf("%02X", arr[i]);
	}
	printf("\n");
}

static size_t getArraySize(caFileHeader* header) {
	return header->cellSize * header->arraySizeI *
							  header->arraySizeJ *
							  header->arraySizeK *
							  header->arraySizeL;
}

static int CAT_ReadData(FILE* fp, caFileHeader* header, char** into) {
	int flag = 0;
	MD5Context ctx;
	MD5Hash fileHash;
	MD5Hash calcHash;

	if (header->compressionType == LZMA_FRAGMENTS) {
		size_t totalSize = getArraySize(header);
		char* arr = malloc(totalSize);
		*into = arr;

		if (arr == NULL) {
			return -3;
		}

		CAT_CompressedFragment frag;
		CAT_Fragment uncompFrag;

		size_t read = 0;

		md5Init(&ctx);
		md5Update(&ctx, (uint8_t*)header, sizeof(caFileHeader));

		while (read < totalSize) {
			flag = CAT_FileReadCompressedFragment(fp, header, &frag);
			if (flag) {
				return -3;
			}

			CAT_HashCompressedFragment(&ctx, &frag);

			flag = CAT_DecompressFragment(header, &frag, &uncompFrag);
			if (flag) {
				return -3;
			}

			memcpy(&arr[read], uncompFrag.data, frag.decompressedSize);
			read += frag.decompressedSize;

			// TODO: this can be improved
			free(uncompFrag.data);
			free(frag.comp.buf);
		}
		
		md5Finalize(&ctx);
		CAT_ToMD5Hash(&ctx, &calcHash);
	} else if (header->compressionType == LZMA) {
		CAT_CompData comp;
		CAT_CompData uncomp = { NULL, 0 };

		flag = fread(&uncomp.bufSz, sizeof(uncomp.bufSz), 1, fp); // TODO: can't we deduce this via getArraySize?
		if (flag != 1) {
			return -3;
		}

		flag = fread(&comp.bufSz, sizeof(comp.bufSz), 1, fp);
		if (flag != 1) {
			return -3;
		}

		comp.buf = malloc(comp.bufSz);
		if (comp.buf == NULL) {
			return -1; // failure to allocate
		}

		flag = fread(comp.buf, 1, comp.bufSz, fp);
		if (flag != comp.bufSz) {
			free(comp.buf);
			return -3;
		}

		flag = CAT_Decompress(&comp, &uncomp);
		if (flag != SZ_OK) {
			free(comp.buf);
			return -6; // decompression failure
		}

		*into = (char*)uncomp.buf;
		free(comp.buf);

		CAT_MD5(uncomp.bufSz, header, uncomp.buf, &calcHash);
	} else if (header->compressionType == UNCOMPRESSED) {
		size_t sz = getArraySize(header);
		char* caArray = malloc(sz);
		*into = caArray;

		if (caArray == NULL) {
			return -1;
		}
		flag = fread(caArray, sz, 1, fp);
		if (flag != 1) {
			free(caArray);
			return -3;
		}

		CAT_MD5(sz, header, caArray, &calcHash);
	} else {
		printf("unsupported compression type");
		return -7;
	}
	
	flag = fread(fileHash.hash, 1, sizeof(fileHash.hash), fp);

	if (flag != sizeof(fileHash.hash)) {
		fclose(fp);
		return -3;
	}

	if (memcmp(fileHash.hash, calcHash.hash, sizeof(fileHash.hash)) != 0) {
		// TODO: stderr
		printf("hash mismatch:\n\tfile: ");
		printMD5(&fileHash);
		printf("\tcalculated: ");
		printMD5(&calcHash);

		fclose(fp);
		return -3;
	}

	return 0;
}

// For future reference: *caArrayPtr doesn't point to an allocated region;
// it's presumed it'll be allocated by CAT_FileRead
int CAT_FileRead(char* filename, caFileHeader** fileHeaderPtr, char** caArrayPtr) {
	FILE *fp;
	*fileHeaderPtr = malloc(sizeof(caFileHeader));
	caFileHeader* fileHeader = *fileHeaderPtr;

	if (fileHeader == NULL){
		return -1;
	}

	if ((fp = fopen(filename, "rb")) == NULL){
		return -2;
	}

	int flag = fread(fileHeader, sizeof(caFileHeader), 1, fp);
	if (flag != 1) {
		fclose(fp);
		return -3;
	}

	flag = CAT_ReadData(fp, fileHeader, caArrayPtr);

	if (flag != 0) {
		free(fileHeader);
		return flag;
	}

	size_t totalSize = getArraySize(fileHeader);

	fclose(fp);
	return 0;
}


static int CAT_WriteData(FILE* fp, caFileHeader* header, char* caArray) {
	size_t totalSize = getArraySize(header);
	int flag = 0;

	if (header->compressionType == LZMA_FRAGMENTS) {
		size_t curWritten = 0;

		MD5Context ctx;
		md5Init(&ctx);
		md5Update(&ctx, (uint8_t*)header, sizeof(caFileHeader));
	
		CAT_CompressedFragment compFrag;

		CAT_Fragment uncompFrag;
		uncompFrag.cellSize = header->cellSize;

		while (totalSize > curWritten) {
			size_t consume = CAT_FragmentSize * header->cellSize;
			size_t left = totalSize - curWritten;

			if (left < consume) {
				consume = left;
			}

			assert( (consume % header->cellSize) == 0 );

			uncompFrag.cellAmt = consume / header->cellSize;
			uncompFrag.data = &caArray[curWritten];

			flag = CAT_CompressFragment(header, &uncompFrag, &compFrag);
			if (flag) {
				return 2;
			}

			CAT_FileWriteCompressedFragment(fp, header, &compFrag);
			CAT_HashCompressedFragment(&ctx, &compFrag);

			free(compFrag.comp.buf);
			curWritten += consume;
		}
		
		md5Finalize(&ctx);

		flag = fwrite(&ctx.digest, sizeof(ctx.digest), 1, fp);
		if (flag != 1) {
			return 2;
		}
		
	} else if (header->compressionType == LZMA) {
		CAT_CompData uncomp = { (unsigned char*)caArray, totalSize };
		CAT_CompData comp;

		SRes status = CAT_Compress(&uncomp, &comp);
		if (status != SZ_OK) {
			return 3;
		}

		// [uncomp_length][comp_length][  compressed cell data  ]
		flag = fwrite(&uncomp.bufSz, sizeof(uncomp.bufSz), 1, fp);
		if (flag != 1)  {
			return 2;
		}

		flag = fwrite(&comp.bufSz, sizeof(comp.bufSz), 1, fp);
		if (flag != 1)  {
			return 2;
		}

		flag = fwrite(comp.buf, comp.bufSz, 1, fp);
		if (flag != 1)  {
			return 2;
		}
	} else {
		flag = fwrite(caArray, totalSize, 1, fp);
		if (flag != 1)  {
			return 2;
		}
	}

	return 0;
}

int CAT_FileSave(char *filename, caFileHeader** fileHeaderPtr, char** caArrayPtr) {
	caFileHeader* fileHeader = *fileHeaderPtr;
	assert(fileHeader->compressionType == LZMA_FRAGMENTS);

	char* caArray = *caArrayPtr;
	FILE *fp;
	unsigned int headerSize = sizeof(caFileHeader);

	if ((fp = fopen(filename, "wb")) == NULL) {
		// printf("Error: can't open file: %s\n", filename);
		// perror("Error while opening file");
		return 1;
	}

	int flag = fwrite((void *)fileHeader, headerSize, 1, fp);
	if (flag != 1) {
		fclose(fp);
		return 2;
	}

	flag = CAT_WriteData(fp, fileHeader, caArray);
	if (flag != 0) {
		fclose(fp);
		return flag;
	}

	fclose(fp);

	return 0;
}
