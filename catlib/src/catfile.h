#pragma once
#include "catlib.h"
#include "deps/md5.h"

typedef struct caFileHeader
{
	uint8_t caSignature[2];
	uint8_t dimension;
	uint8_t dSignature;
	uint8_t vSignature[4];
	uint8_t version[8];
	uint32_t arrayTopology;
	uint32_t modelType;
	uint32_t operationMode;
	uint32_t compressionType;
	uint64_t cellSize;
	uint64_t globalSize;
	uint64_t arraySizeI;
	uint64_t arraySizeJ;
	uint64_t arraySizeK;
	uint64_t arraySizeL;
	uint64_t iterationsDone;
	double  cellsPerMeter;

	// Fragment data
	// uint64_t fragSize; 	// How many cells is a single fragment
	// uint64_t fragCount;	// How many fragments are there in this file

	uint8_t reserved96[32];
	uint8_t reserved128[128];
	uint8_t reserved256[256];
	uint8_t reserved512[512];
} caFileHeader;

int CAT_FileSave(char *filename, caFileHeader** fileHeaderPtr, char** caArrayPtr);
int CAT_FileRead(char *filename, caFileHeader** fileHeaderPtr, char** caArrayPtr);

typedef struct CAT_md5_t { uint8_t hash[16]; } MD5Hash;

void CAT_MD5(unsigned int arraySize, caFileHeader* header, char *ca, MD5Hash* out);
void CAT_ToMD5Hash(MD5Context* ctx, MD5Hash* out);
void printMD5(MD5Hash* h);

