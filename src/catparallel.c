#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "catlib.h"
#include "deps/md5.h"
#include "catparallel.h"
#include "catfragment.h"
#include "catlzma.h"
#include "catfile.h"
#include <errno.h>

#ifdef USE_MPI
#include <mpi.h>
int CAT_Proc_size = 1;
int CAT_Proc_rank = 0;
int CAT_Bound_size = 2;
#else
const int CAT_Proc_size = 1;
const int CAT_Proc_rank = 0;
#endif

#ifdef USE_MPI
void CAT_MPIAbort(int errCode, const char* fmt, ...) {
    printf("[catlib/rank%d] ", CAT_Proc_rank);
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    printf("\n");

    fflush(stdout);

    MPI_Abort(MPI_COMM_WORLD, errCode);
}

void mpiprintf(const char* fmt, ...) {
    printf("[rank%d] ", CAT_Proc_rank);
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    printf("\n");
    fflush(stdout);
}

int CAT_InitParallelRun() {
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &CAT_Proc_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &CAT_Proc_size);
}
#endif


unsigned int CAT_GetPartSize(uint64_t num, int procRank, int period) {
    uint64_t left = (num / period) % CAT_Proc_size;
    uint64_t base = (num / period) / CAT_Proc_size;

    return (base + (procRank < left)) * period;
}

#ifdef USE_MPI
int CAT_BoundSend(char* caArray, MPI_Request* request, MPI_Status* status,
                  unsigned int arraySizeWithBound, unsigned int cellSize, unsigned int shift) {
    int flag = 0;
    int rankUp = CAT_Proc_rank != 0 ? CAT_Proc_rank - 1 : CAT_Proc_size - 1;
    int rankDown = (CAT_Proc_rank + 1) % CAT_Proc_size;
    flag = MPI_Isend(caArray + shift * cellSize, cellSize * shift, MPI_CHAR,
                     rankUp, 0, MPI_COMM_WORLD, &request[0]);
    if (flag != MPI_SUCCESS) {
        CAT_MPIAbort(MPI_ERROR, "Couldn't send bounds (-> %d)", rankUp);
    }
    flag = MPI_Isend(caArray + (arraySizeWithBound - 2 * shift) * cellSize, cellSize * shift, MPI_CHAR,
                     rankDown, 0, MPI_COMM_WORLD, &request[1]);
    if (flag != MPI_SUCCESS) {
        CAT_MPIAbort(MPI_ERROR, "Couldn't send bounds (-> %d)", rankDown);
    }
}

int CAT_BoundRecv(char* caArray, MPI_Request* request, MPI_Status* status,
                  unsigned int arraySizeWithBound, unsigned int cellSize, unsigned int shift) {
    int flag = 0;
    int rankUp = CAT_Proc_rank != 0 ? CAT_Proc_rank - 1 : CAT_Proc_size - 1;
    int rankDown = (CAT_Proc_rank + 1) % CAT_Proc_size;
    flag = MPI_Irecv(caArray + (arraySizeWithBound - shift) * cellSize, cellSize * shift, MPI_CHAR,
              rankDown, 0, MPI_COMM_WORLD, &request[2]);
    if (flag != MPI_SUCCESS) {
        CAT_MPIAbort(MPI_ERROR, "Couldn't receive bounds (<- %d)", rankDown);
    }
    flag = MPI_Irecv(caArray, cellSize * shift, MPI_CHAR,
              rankUp, 0, MPI_COMM_WORLD, &request[3]);
    if (flag != MPI_SUCCESS) {
        CAT_MPIAbort(MPI_ERROR, "Couldn't receive bounds (<- %d)", rankUp);
    }

}

// Shift is measured in cells, not bytes
int CAT_BoundExchange(CAT_Globals_t* glob, unsigned int shift, unsigned int partArraySize) {
    int flag = 0;
    MPI_Request request[4];
    MPI_Status status[4];
    int rankUp = CAT_Proc_rank != 0 ? CAT_Proc_rank - 1 : CAT_Proc_size - 1;
    int rankDown = (CAT_Proc_rank + 1) % CAT_Proc_size;

    size_t shiftBytes = shift * glob->fileHeader->cellSize;
    char* field = glob->cellularArray;
    size_t fieldSize = partArraySize * glob->fileHeader->cellSize;

    /*
    mpiprintf("Bound exchange: ranks %d/%d, shift = %d & %d",
        rankDown, rankUp,
        shiftBytes, fieldSize - shiftBytes);
    */

    // We offset `shiftBytes` from our field on both sides
    //    so we send our main chunk's edges, not bounds we got from someone else
    flag = MPI_Isend(field + shiftBytes, shiftBytes, MPI_CHAR,
              rankUp, 0, MPI_COMM_WORLD, &request[0]);
    if (flag != MPI_SUCCESS) {
        CAT_MPIAbort(MPI_ERROR, "Failed to exchange bounds up (-> %d)", rankUp);
    }
    flag = MPI_Isend(field + (fieldSize - 2 * shiftBytes), shiftBytes, MPI_CHAR,
                     rankDown, 0, MPI_COMM_WORLD, &request[1]);
    if (flag != MPI_SUCCESS) {
        CAT_MPIAbort(MPI_ERROR, "Failed to exchange bounds down (-> %d)", rankDown);
    }

    // But we receive into our bounds
    flag = MPI_Irecv(field + (fieldSize - shiftBytes), shiftBytes, MPI_CHAR,
              rankDown, 0, MPI_COMM_WORLD, &request[3]);
    if (flag != MPI_SUCCESS) {
        CAT_MPIAbort(MPI_ERROR, "Failed to exchange bounds down (<- %d)", rankDown);
    }
    flag = MPI_Irecv(field, shiftBytes, MPI_CHAR,
              rankUp, 0, MPI_COMM_WORLD, &request[2]);
    if (flag != MPI_SUCCESS) {
        CAT_MPIAbort(MPI_ERROR, "Failed to exchange bounds up (<- %d)", rankUp);
    }
    // mpiprintf("Waiting...");

    flag = MPI_Waitall(4, request, status);
    if (flag != MPI_SUCCESS) {
        CAT_MPIAbort(MPI_ERROR, "Failed to wait for bounds");
    }

    return 0;
}
#endif

size_t CAT_GetProcSize(caFileHeader* hdr, int rank, int period) {
    return CAT_GetPartSize(hdr->arraySizeI, rank, period) * hdr->arraySizeJ * hdr->cellSize;
}

#ifdef USE_MPI
size_t CAT_MPISendFragment(CAT_CompressedFragment* frag, size_t canAccept, size_t cursor, int destRank) {
    size_t fragConsume = frag->decompressedSize - cursor; // How much data from this fragment will be actually consumed
    uint64_t consumeCount = canAccept;
    if (fragConsume < consumeCount) {
        consumeCount = fragConsume;
    }

    uint64_t numbers[3] = {
        frag->comp.bufSz,   // Send compressed size;
        frag->decompressedSize, // decompressed size;
        (uint64_t)cursor        // offset they should read from (in decompressed data)
    };

    // Send the compressed/decompressed size, then the compressed data
    
    int flag = MPI_Send(numbers, sizeof(numbers) / sizeof(numbers[0]), MPI_UINT64_T,
        destRank, 0, MPI_COMM_WORLD);
    if (flag != MPI_SUCCESS) {
        CAT_MPIAbort(MPI_ERROR, "Failed to send fragment(size) to rank %d", destRank);
    }

    flag = MPI_Send(frag->comp.buf, frag->comp.bufSz, MPI_CHAR, destRank, 0, MPI_COMM_WORLD);
    if (flag != MPI_SUCCESS) {
        CAT_MPIAbort(MPI_ERROR, "Failed to send fragment(data) to rank %d", destRank);
    }
    
    return consumeCount;
}

size_t CAT_MPIRecvFragment(CAT_CompressedFragment* frag, int fromRank, int tag) {
    uint64_t numbers[3];
    
    int flag = MPI_Recv(numbers, sizeof(numbers) / sizeof(numbers[0]), MPI_UINT64_T,
        fromRank, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    if (flag != MPI_SUCCESS) {
        CAT_MPIAbort(MPI_ERROR, "Failed to receive frag size");
    }

    frag->comp.bufSz = numbers[0];
    frag->decompressedSize = numbers[1];
    size_t cursor = numbers[2];

    frag->comp.buf = malloc(frag->comp.bufSz);
    if (frag->comp.buf == NULL) {
        CAT_MPIAbort(ALLOC_ERROR, "Failed to allocate buffer for fragment");
    }
    
    flag = MPI_Recv(frag->comp.buf, frag->comp.bufSz,
            MPI_CHAR, fromRank, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    if (flag != MPI_SUCCESS) {
        CAT_MPIAbort(MPI_ERROR, "Failed to receive frag data");
    }

    return cursor;
}

int CAT_FileReadParallel2D(char *filename, CAT_Globals_t* glob, CAT_Index borders, CAT_Index period) {
    int flag = 0;
    FILE *fp;

    int CAT_Bound_size = borders.i;
    int CAT_Period = period.i;
    glob->fileHeader = malloc(sizeof(caFileHeader));
    caFileHeader* fileHeader = glob->fileHeader;

    if (fileHeader == NULL) {
        CAT_MPIAbort(ALLOC_ERROR, "Couldn't allocate memory for the file header");
    }

    if (CAT_Proc_rank == 0) {
        if ((fp = fopen(filename, "rb")) == NULL) {
            CAT_MPIAbort(IO_ERROR, "Couldn't open input file `%s`", filename);
        }
        flag = fread(fileHeader, sizeof(caFileHeader), 1, fp);
        if (flag != 1) {
            fclose(fp);
            CAT_MPIAbort(IO_ERROR, "Failed to read the header from file `%s`", filename);
        }
    }
    
    flag = MPI_Bcast(fileHeader, sizeof(caFileHeader), MPI_CHAR, 0, MPI_COMM_WORLD);
    if (flag != MPI_SUCCESS) {
        CAT_MPIAbort(MPI_ERROR, "Couldn't broadcast file header");
    }

    // We'll be calculating the digest while reading
	MD5Context ctx;
	MD5Hash calcHash;

    md5Init(&ctx);
	md5Update(&ctx, (uint8_t*)fileHeader, sizeof(caFileHeader));
    // Allocate the field data itself
    unsigned int cellSize = (fileHeader)->cellSize;
    unsigned int fullSizeI = (fileHeader)->arraySizeI;
    unsigned int fullSizeJ = (fileHeader)->arraySizeJ;

    //If size of field can't divide on period throw error.
    if (fullSizeI % CAT_Period != 0 || fullSizeI / CAT_Period < CAT_Proc_size) {
        CAT_MPIAbort(INAPPROPRIATE_SIZE, "This size of field can't divide on CA period.");
    }
    unsigned int partArraySize = CAT_GetProcSize(fileHeader, CAT_Proc_rank, CAT_Period);

    //If size of array part is less then borders size throw error.
    if (partArraySize < (2 * CAT_Bound_size * fullSizeJ * cellSize)) {
        CAT_MPIAbort(INAPPROPRIATE_SIZE, "Inappropriate number of process for this field.\n");
    }
    // Allocate more than what we need to store our field,
    // to account for storing the surrounding bounds as well
    size_t boundSize = CAT_Bound_size * fullSizeJ * cellSize;
    size_t arraySizeWithBound = partArraySize + boundSize * 2;

    char* caArray = malloc(arraySizeWithBound);
    char* caArray2 = malloc(arraySizeWithBound);

    glob->cellularArray = caArray;
    glob->cellularArraySwap = caArray2;
//    glob->fieldBytes = arraySizeWithBound;
//    glob->boundBytes = boundSize * 2;

    // Read out fragments from the file and send them to processes
    size_t totalFrags = CAT_CalcFragsRequired(fileHeader);
    size_t fragCount = CAT_GetPartSize(totalFrags, CAT_Proc_rank, CAT_Period);

    if (CAT_Proc_rank == 0) {
        uint64_t fragCursor = 0; // Offset along the uncompressed fragment (for receivers)
        CAT_CompressedFragment inplaceRead;

        for (int destRank = 0; destRank < CAT_Proc_size; ++destRank) {
            if (destRank == 0) {
                /*
                 Root: reading fragments from a file, either for itself or to send to others

                 The logic is this: root reads and decompresses fragments for itself until it's full
                 If there is data from a fragment leftover, it gets carried to the "broadcasting" phase
                */
                
                uint64_t bufCursor = 0; 

                while (partArraySize > bufCursor) {
                    // There is space in root: read for ourselves...
                    size_t spaceLeft = partArraySize - bufCursor;

                    flag = CAT_FileReadCompressedFragment(fp, fileHeader, &inplaceRead);
                    if (flag) {
                        CAT_MPIAbort(IO_ERROR, "Error while reading compressed fragment");
                    }

                    CAT_HashCompressedFragment(&ctx, &inplaceRead);

                    CAT_Fragment frag;
                    flag = CAT_DecompressFragment(fileHeader, &inplaceRead, &frag);
                    if (flag) {
                        CAT_MPIAbort(ALLOC_ERROR, "Error during fragment decompression");
                    }

                    size_t fragBytes = frag.cellAmt * frag.cellSize;
                    size_t writeCursor = bufCursor + boundSize; // Write into main chunk, not bound

                    if (spaceLeft >= fragBytes) {
                        // We can fit the entire fragment, just copy it
                        memcpy(&caArray[writeCursor], frag.data, fragBytes);
                        bufCursor += fragBytes;

                        fragCursor = fragBytes;
                        free(frag.data); // TODO: It'd be nice to reuse the same memory...
                    } else {
                        // We can fit the fragment only partially; the rest should be sent to other processes
                        size_t toCopy = fragBytes;
                        if (spaceLeft < toCopy) {
                            toCopy = spaceLeft;
                        }
                        memcpy(&caArray[writeCursor], frag.data, toCopy);
                        bufCursor += toCopy;
                        
                        if (CAT_Proc_size == 1) {
                            // If we're running in a single process, we MUST fit all the fragments into our memory
                            assert(fragBytes == spaceLeft);
                        }

                        // We filled up the root's buffer, now send the leftover data to other processes
                        fragCursor = toCopy;
                        free(frag.data);
                        break;
                    }
                }

                // mpiprintf("Root: read %p[%d]/%d bytes", caArray, bufCursor, partArraySize);
            } else {
                /*  Reading fragments for someone else
                    As long as they have space available, they will send whole, but compressed, fragments
                    They can handle decompressing and picking out the data themselves
                */
                
                size_t sendCount = CAT_GetPartSize(totalFrags, destRank, CAT_Period);
                size_t wantsBytes = CAT_GetProcSize(fileHeader, destRank, CAT_Period);

                while (wantsBytes > 0) {
                    size_t fragSizeBytes = inplaceRead.decompressedSize;

                    // If the cursor is bigger than the fragment size, then that means it's done
                    // We need to read a new one out
                    if (fragSizeBytes <= fragCursor) {
                        flag = CAT_FileReadCompressedFragment(fp, fileHeader, &inplaceRead);
                        if (flag) {
                            CAT_MPIAbort(IO_ERROR, "Error while reading compressed fragment");
                        }

                        CAT_HashCompressedFragment(&ctx, &inplaceRead);

                        fragCursor = 0;
                        fragSizeBytes = inplaceRead.decompressedSize;
                    }

                    // `fragCursor` is a cursor along each fragment's data (ie [0; tempFrag->cellSize])
                    if (fragSizeBytes <= fragCursor) {
                        assert(0); // This should never happen
                    }

                    // The process can fit SOME data, so send the entire compressed fragment
                    // If they cant fit the entire fragment, they'll handle picking out the data themselves
                    size_t consumed = CAT_MPISendFragment(&inplaceRead, wantsBytes, fragCursor, destRank);

                    fragCursor += consumed;
                    wantsBytes -= consumed;
                }
            }
        }

        md5Finalize(&ctx);
        CAT_ToMD5Hash(&ctx, &calcHash);

        MD5Hash fileHash;
        flag = fread(fileHash.hash, 1, sizeof(fileHash.hash), fp);

        if (flag != sizeof(fileHash.hash)) {
            fclose(fp);
            return -3;
        }
        

        if (memcmp(fileHash.hash, calcHash.hash, sizeof(fileHash.hash)) != 0) {
            CAT_MPIAbort(IO_ERROR, "MD5 mismatch");
            return -3;
        }

        fclose(fp);
    } else {
        // We're not the root process; wait for data from root

        uint64_t cursor = 0;
    
        CAT_CompressedFragment tempFrag;
        CAT_Fragment tempFragUncomp;
        
        while (cursor < partArraySize) {
            size_t readOffset = CAT_MPIRecvFragment(&tempFrag, 0, 0);
            flag = CAT_DecompressFragment(fileHeader, &tempFrag, &tempFragUncomp);
            if (flag) {
                CAT_MPIAbort(ALLOC_ERROR, "Failed to decompress received fragment");
            }

            size_t toCopy = tempFrag.decompressedSize - readOffset;
            size_t canFit = partArraySize - cursor;
            if ( canFit < toCopy ) {
                toCopy = canFit;
            }

            // Bound exchange will be done separately; just write into main chunk
            size_t writeCursor = cursor + boundSize;

            /*mpiprintf("copying [%d-%d] bytes into %p[%d-%d]... (SZ = %d) (%d < %d)",
                readOffset, readOffset + toCopy, caArray, writeCursor, writeCursor + toCopy,
                arraySizeWithBound,
                cursor + toCopy, partArraySize);*/

            memcpy(&caArray[writeCursor], &tempFragUncomp.data[readOffset], toCopy);
            cursor += toCopy;

            free(tempFrag.comp.buf);
        }
    }
    
    CAT_BoundExchange(glob, CAT_Bound_size * fileHeader->arraySizeJ, arraySizeWithBound); // caArray, arraySizeWithBound, cellSize, CAT_Bound_size * (fileHeader)->arraySizeJ);

    return 0;
}

int CAT_FileSaveParallel2D(char *filename, CAT_Globals_t* glob, CAT_Index period)
{
    int flag = 0;
    caFileHeader *fileHeader = glob->fileHeader;
    char *caArray = glob->cellularArray;

    unsigned int cellSize = (fileHeader)->cellSize;
    unsigned int fullSizeI = (fileHeader)->arraySizeI;
    unsigned int fullSizeJ = (fileHeader)->arraySizeJ;
    unsigned int shift = CAT_Bound_size * fullSizeJ; // in cells
    unsigned int CAT_Period = period.i;
    unsigned int arrayCells = CAT_GetPartSize(fullSizeI, CAT_Proc_rank, CAT_Period) * fullSizeJ;

    // Wrap our array of data (caArray) in a fragment, then use that to exchange data with other processes
    // (either receive into it if we're the root process, or send it to root if we're a child process)
    CAT_Fragment exchangeFrag;
    exchangeFrag.cellSize = fileHeader->cellSize;
    exchangeFrag.data = &caArray[shift * cellSize]; // Add an offset to not write/send the "left" bound
    exchangeFrag.cellAmt = arrayCells;
    
    /*
    mpiprintf("CAT_Save: %p %d * %d = %d bytes", exchangeFrag.data, exchangeFrag.cellAmt, exchangeFrag.cellSize,
        exchangeFrag.cellAmt * exchangeFrag.cellSize);
    */

    CAT_CompressedFragment compFrag;
    MD5Context ctx;
	MD5Hash calcHash;
    
    md5Init(&ctx);

    if (CAT_Proc_rank == 0) {
        FILE* fp = fopen(filename, "wb");
        if (fp == NULL) {
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        unsigned int headerSize = sizeof(caFileHeader);

        fileHeader->compressionType = LZMA_FRAGMENTS;
        flag = fwrite(fileHeader, headerSize, 1, fp);
        if (flag != 1) {
            fclose(fp);
            MPI_Abort(MPI_COMM_WORLD, 2);
        }

        md5Update(&ctx, (uint8_t*)fileHeader, sizeof(caFileHeader));

        for (int sourceRank = 0; sourceRank < CAT_Proc_size; ++sourceRank) {
            if (sourceRank != 0) {
                CAT_MPIRecvFragment(&compFrag, sourceRank, 0);
            } else {
                // just compress the whole array rofl lmao
                // TODO: do it proper
                CAT_CompressFragment(fileHeader, &exchangeFrag, &compFrag);
            }

            CAT_HashCompressedFragment(&ctx, &compFrag);
            flag = CAT_FileWriteCompressedFragment(fp, fileHeader, &compFrag);
            free(compFrag.comp.buf);

            if (flag) {
                fclose(fp);
                CAT_MPIAbort(IO_ERROR, "Failed to write the fragment (from rank %d) to file",
                    sourceRank);
            }
        }

        md5Finalize(&ctx);
        CAT_ToMD5Hash(&ctx, &calcHash);

        fwrite(calcHash.hash, sizeof(calcHash.hash), 1, fp);
        fclose(fp);
    } else {
        // just compress the whole array rofl lmao
        // TODO: do it proper

        CAT_CompressFragment(fileHeader, &exchangeFrag, &compFrag);
        uint64_t sent = CAT_MPISendFragment(&compFrag,
            INT64_MAX, // let's assume the root process can fit (and will consume) the entire fragment
            0, 0);

        free(compFrag.comp.buf);
        assert(sent == (exchangeFrag.cellAmt * exchangeFrag.cellSize)); // Make sure we could send ALL of our data
    }
    return 0;
}
#endif
