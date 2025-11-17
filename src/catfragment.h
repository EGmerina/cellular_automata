#pragma once

// Handles fragmenting the cell data into fragments, each up to a certain size
// This allows ...stuff (TODO: what exactly)

#include <stddef.h>
#include <stdio.h>

#include "catlib.h"
#include "catglobals.h"
#include "catlzma.h"
#include "deps/md5.h"

typedef struct frag_t {
    size_t cellSize; // Size of each cell in bytes
    size_t cellAmt;
    char* data;
} CAT_Fragment;

typedef struct frags_t {
    size_t len;
    CAT_Fragment** frags;
} CAT_Fragments;

typedef struct comp_frag_t {
    CAT_CompData comp;
    size_t decompressedSize;
} CAT_CompressedFragment;

// NOTE: Changing fragment size will result in incorrect behavior with files
//       which were generated using catlib with a different fragment size
// TODO: how fix /?//???

extern const size_t CAT_FragmentSize; // (in cells...?)

int CAT_AllocFragment(caFileHeader* fileHeader, CAT_Fragment** into);

void CAT_FreeFragment(CAT_Fragment* frag);

size_t CAT_FileReadCompressedFragment(FILE* fp, caFileHeader* fileHeader, CAT_CompressedFragment* frag);
int CAT_FileWriteCompressedFragment(FILE* fp, caFileHeader* fileHeader, CAT_CompressedFragment* frag);

void CAT_HashCompressedFragment(MD5Context* ctx, CAT_CompressedFragment* compFrag);

int CAT_DecompressFragment(caFileHeader* fileHeader, CAT_CompressedFragment* frag, CAT_Fragment* fragOut);
int CAT_CompressFragment(caFileHeader* fileHeader, CAT_Fragment* frag, CAT_CompressedFragment* fragOut);

size_t CAT_CalcFragsRequired(caFileHeader* fileHeader);
