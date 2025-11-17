#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "catfragment.h"
#include "catlib.h"
#include "catparallel.h"
#include "catlzma.h"
#include "deps/md5.h"

const size_t CAT_FragmentSize = 64 * (1 << 10);

int CAT_FileWriteCompressedFragment(FILE* fp, caFileHeader* fileHeader, CAT_CompressedFragment* frag) {
    uint64_t sizes[2] = { frag->comp.bufSz, frag->decompressedSize };
    int wrote = fwrite(&sizes, sizeof(sizes[0]), 2, fp);
    if (wrote != 2) {
        return -1;
    }

    wrote = fwrite(frag->comp.buf, frag->comp.bufSz, 1, fp);
    if (wrote != 1) {
        return -2;
    }

    return 0;
}

size_t CAT_FileReadCompressedFragment(FILE* fp, caFileHeader* fileHeader, CAT_CompressedFragment* frag) {
    int flag = 0;

    CAT_CompData uncomp = { NULL, 0 };

    uint64_t sizes[2]; // { compSz, uncompSz }
    flag = fread(&sizes, sizeof(sizes[0]), 2, fp);
    if (flag != 2) {
        return -1;
    }

    frag->comp.bufSz = sizes[0];
    frag->decompressedSize = sizes[1];
    frag->comp.buf = malloc(frag->comp.bufSz);

    if (frag->comp.buf == NULL) {
        return -2;
    }

    flag = fread(frag->comp.buf, 1, frag->comp.bufSz, fp);
    if (flag != frag->comp.bufSz) {
        free(frag->comp.buf);
        return -1;
    }

    return 0;
}

void CAT_HashCompressedFragment(MD5Context* ctx, CAT_CompressedFragment* compFrag) {
    /*
    printf("hashing buf[%d] + %d -> ", compFrag->comp.bufSz, compFrag->decompressedSize);
    for (int i = 0; i < compFrag->comp.bufSz; i++) {
        printf("%02X", compFrag->comp.buf[i]);    
    }
    printf("\n");
    */

    md5Update(ctx, compFrag->comp.buf, compFrag->comp.bufSz);
    md5Update(ctx, (uint8_t*) &compFrag->decompressedSize, sizeof(size_t));
}

int CAT_DecompressFragment(caFileHeader* fileHeader, CAT_CompressedFragment* frag, CAT_Fragment* fragOut) {
    CAT_CompData uncomp = { NULL, frag->decompressedSize };

    int flag = CAT_Decompress(&frag->comp, &uncomp);
    if (flag != SZ_OK) {
        return -3; // decompression failure
    }
    
    fragOut->data = (char*)uncomp.buf;
    fragOut->cellSize = fileHeader->cellSize;
    fragOut->cellAmt = uncomp.bufSz / fragOut->cellSize;

    assert( (uncomp.bufSz % fragOut->cellSize) == 0 );

    return 0;
}

int CAT_CompressFragment(caFileHeader* fileHeader, CAT_Fragment* frag, CAT_CompressedFragment* fragOut) {
    CAT_CompData uncomp = {
        .buf = (unsigned char*)frag->data,
        .bufSz = frag->cellSize * frag->cellAmt
    };

	CAT_CompData comp;

	SRes status = CAT_Compress(&uncomp, &comp);
	if (status != SZ_OK) {
		return -1;
	}

    fragOut->decompressedSize = uncomp.bufSz;
    fragOut->comp.buf = comp.buf;
    fragOut->comp.bufSz = comp.bufSz;

    return 0;
};

size_t CAT_FileReadIntoFragment(FILE* fp, caFileHeader* fileHeader, CAT_Fragment* frag) {
    assert(frag->cellSize == fileHeader->cellSize);

    unsigned long readAmt = fread(frag->data, fileHeader->cellSize, CAT_FragmentSize, fp);
    frag->cellAmt = readAmt;

    return readAmt;
}

int CAT_AllocFragment(caFileHeader* fileHeader, CAT_Fragment** into) {
    CAT_Fragment* frag = malloc(sizeof(CAT_Fragment));
    if (frag == NULL) {
        return ALLOC_ERROR;
    }

    frag->data = malloc(CAT_FragmentSize * fileHeader->cellSize);
    frag->cellSize = fileHeader->cellSize;
    frag->cellAmt = 0;

    if (frag->data == NULL) {
        free(frag);
        return ALLOC_ERROR;
    }

    *into = frag;
    return 0;
}

void CAT_FreeFragment(CAT_Fragment* frag) {
    free(frag->data);
    free(frag);
}

void CAT_FreeFragments(CAT_Fragments* frags) {
    for (int i = 0; i < frags->len; i++) {
        CAT_FreeFragment(frags->frags[i]);
    }

    free(frags);
}

size_t CAT_CalcFragsRequired(caFileHeader* fileHeader) {
    uint64_t totalCells = fileHeader->arraySizeI
                            * fileHeader->arraySizeJ
                            * fileHeader->arraySizeK
                            * fileHeader->arraySizeL;

    // Ceiling integer division
    return (totalCells + CAT_FragmentSize - 1) / CAT_FragmentSize;
}