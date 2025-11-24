#include <stdint.h>
#include <stdio.h>
#include "configParser.h"
#include <cassert>
#include <pthread.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui.hpp>
extern "C" {
#include "catlib.h"
}
#define SIZE_RATIO_DEPENDENCY 0.86602540378
#define CV_IMWRITE_JPEG_QUALITY 100

using namespace std;
using namespace cv;

typedef struct hexCell {
    uint16_t countCellsDirection0;
    uint16_t countCellsDirection1;
    uint16_t countCellsDirection2;
    uint16_t countCellsDirection3;
    uint16_t countCellsDirection4;
    uint16_t countCellsDirection5;
    uint32_t cellType;
} hexCell;

const int cellSize = sizeof(hexCell);
const char inputFileName[] = "HEXAGON-IN.dat";
const char outputFileName[] = "HEXAGON-OUT.dat";
int COUNT_TYPE = 0;

