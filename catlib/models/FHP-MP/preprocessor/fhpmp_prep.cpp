#include "fhpmp.hpp"

int X = 0;//size of image
int Y = 0;//size of image
double K_X = 0;//size of hex cell
double K_Y = 0;//size of hex cell
int COUNT_THREADS = 0;

typedef struct threadDataInfo {
	uint16_t countCellsInXstart;
	uint16_t countCellsInXfinish;
	uint16_t countCellsInX;
	uint16_t countCellsInY;
	hexCell *hexMat;
	Mat* img;
} threadDataInfo;

double createSizeOfStep() {
	if(1 < K_X) {
		return 0.1;
	} else {
		return K_X / 12;
	}
}

void * calculateCellsValue(void * param) {
	//calculate value for each hex cell from input image
	Mat img = *((threadDataInfo*)param)->img;
	hexCell *hexMat = ((threadDataInfo*)param)->hexMat;
	double sizeOfStep = createSizeOfStep();
	int countOfStepX = 0;
	int countOfStepY = 0;
	int CountIsOnCells = 0;
	int countCellsInX = ((threadDataInfo*)param)->countCellsInX;

	//whole borders of each cell
	double upperBoundOfCell = 0;
	double lowerBoundOfCell = 0;
	double leftBoundOfCell = 0;
	double rightBoundOfCell = 0;
	double currentPointCoordX = 0;
	double currentPointCoordY = 0;
	double coordCentreCellX = 0;
	double coordCentreCellY = 0;

	unsigned long long int avrgCountParticleInCell = 0;
	//create special array for count of points of different types in each cell
	int * countPointsOfDifferentTypesInCell = (int*)malloc((COUNT_TYPE) * sizeof(int));
	if(NULL == countPointsOfDifferentTypesInCell) {
		cerr << "No memory";
		return NULL;
	}
	for (int i = 0; i < COUNT_TYPE; i++) {
		countPointsOfDifferentTypesInCell[i] = 0;
	}

	countOfStepX = K_X / sizeOfStep;
	countOfStepY = K_Y / sizeOfStep;

	for (int i = ((threadDataInfo*)param)->countCellsInXstart; i < ((threadDataInfo*)param)->countCellsInXfinish; i++) {
		for (int j = 0; j < ((threadDataInfo*)param)->countCellsInY; j++) {
			//calculate coordinates of centre of real cell
			coordCentreCellX = i * 0.75 * K_X;
			if(i % 2 == 0) {
				coordCentreCellY = j * K_Y;
			} else {
				coordCentreCellY = j * K_Y + K_Y/2;
			}

			upperBoundOfCell = coordCentreCellX - K_X*0.5;
			lowerBoundOfCell = coordCentreCellX + K_X*0.5;
			leftBoundOfCell = coordCentreCellY - K_Y*0.5;
			rightBoundOfCell = coordCentreCellY + K_Y*0.5;

			//start from upper left corner
			for(int k = 0; k <= countOfStepX; k++) {
				for(int m = 0; m <= countOfStepY; m++) {
					//in coordinate system
					currentPointCoordX = upperBoundOfCell + k * sizeOfStep - coordCentreCellX;
					currentPointCoordY = leftBoundOfCell + m * sizeOfStep - coordCentreCellY;
					//6 straight line of hex
					if((currentPointCoordY < SIZE_RATIO_DEPENDENCY * K_X * currentPointCoordX / 2) &&
						(currentPointCoordY > (-1) * SIZE_RATIO_DEPENDENCY * K_X * currentPointCoordX / 2) &&
						currentPointCoordY < (-2) * SIZE_RATIO_DEPENDENCY * currentPointCoordX + SIZE_RATIO_DEPENDENCY * K_X &&
						currentPointCoordY > 2 * SIZE_RATIO_DEPENDENCY * currentPointCoordX - SIZE_RATIO_DEPENDENCY * K_X &&
						currentPointCoordY > (-2) * SIZE_RATIO_DEPENDENCY * currentPointCoordX - SIZE_RATIO_DEPENDENCY * K_X &&
						currentPointCoordY < 2 * SIZE_RATIO_DEPENDENCY * currentPointCoordX + SIZE_RATIO_DEPENDENCY * K_X ) {
							//if our point in image
							if(currentPointCoordX + coordCentreCellX >= 0 &&
								currentPointCoordX + coordCentreCellX <= X &&
								currentPointCoordY + coordCentreCellY >= 0 &&
								currentPointCoordY + coordCentreCellY <= Y) {
								//define type of point and increment array
								//_____________________________
								//_____________________________
								countPointsOfDifferentTypesInCell[(img.at<Vec3b>(floor(currentPointCoordX + coordCentreCellX), floor(																	currentPointCoordY + coordCentreCellY))[0])/COUNT_TYPE]++;
								avrgCountParticleInCell+= (int)img.at<Vec3b>(floor(currentPointCoordX + coordCentreCellX), floor(																	currentPointCoordY + coordCentreCellY))[1];
								avrgCountParticleInCell+= (int)img.at<Vec3b>(floor(currentPointCoordX + coordCentreCellX), floor(																	currentPointCoordY + coordCentreCellY))[2] << 8;
								CountIsOnCells++;
								//_____________________________
								//_____________________________
							}
					}
				}
			}
			//find max element in countPointsOfDifferentTypesInCell; it will be type of all cell
			int maxValueOfType = countPointsOfDifferentTypesInCell[0];
			int maxType = 0;
			for (int k = 0; k < COUNT_TYPE; ++k) {
				if (countPointsOfDifferentTypesInCell[k] > maxValueOfType) {
					maxValueOfType = countPointsOfDifferentTypesInCell[k];
					maxType = k;
				}
			}
			hexMat[j*countCellsInX + i].cellType = maxType;
			hexMat[j*countCellsInX + i].countCellsDirection0 = 0;
			hexMat[j*countCellsInX + i].countCellsDirection1 = 0;
			hexMat[j*countCellsInX + i].countCellsDirection2 = 0;
			hexMat[j*countCellsInX + i].countCellsDirection3 = 0;
			hexMat[j*countCellsInX + i].countCellsDirection4 = 0;
			hexMat[j*countCellsInX + i].countCellsDirection5 = 0;

			CAT_PutCell(&hexMat[j*countCellsInX + i], i, j);

			//nullify countPointsOfDifferentTypesInCell
			for (int k = 0; k < COUNT_TYPE; k++) {
				countPointsOfDifferentTypesInCell[k] = 0;
			}
			avrgCountParticleInCell = 0;
			CountIsOnCells = 0;
		}
	}
	return NULL;
}

void generateHexMat(hexCell *hexMat, int countCellsInX, int countCellsInY, Mat* img, pthread_t * threads, threadDataInfo * threadsInfo) {
	assert(0 != hexMat && 0 != countCellsInX && 0 != countCellsInY);
	for(int i = 0; i < COUNT_THREADS; i++) {
		threadsInfo[i].countCellsInXstart = countCellsInX/COUNT_THREADS*(i) + 1;
		if(0 == i) {
			threadsInfo[i].countCellsInXstart = 0;
		}
		threadsInfo[i].countCellsInXfinish=countCellsInX/COUNT_THREADS*(i+1);
		if(COUNT_THREADS - 1 == i) {
			threadsInfo[i].countCellsInXfinish = countCellsInX;
		}
		threadsInfo[i].countCellsInX = countCellsInX;
		threadsInfo[i].countCellsInY = countCellsInY;
		threadsInfo[i].hexMat = hexMat;
		threadsInfo[i].img = img;
		pthread_create(threads+i, NULL, calculateCellsValue, (void*)(threadsInfo+i));
    }
	for(int i=0; i<COUNT_THREADS; i++) {
		pthread_join(threads[i], 0);
    }
}

int main(int argc, char *argv[])
{

    if(2 > argc) {
		cerr << "No way to config file";
		return -1;
	}

    const int globalSize = 0;
    double Kl = 0;
    char inputFilePath[FILE_PATH_LENGHT];

	getFileByName(argv[argc-1], "imageParametrs", "path_input_file", inputFilePath);
	Kl = getDoubleParamByName(argv[argc-1], "imageParametrs", "Kl");

	Mat img = imread(inputFilePath, IMREAD_UNCHANGED);
	if(img.empty()) {
		cerr << "Error while reading image";
		return -1;
	}
	X = img.rows;
	Y = img.cols;

    K_Y = 1 / Kl;
	K_X = K_Y / SIZE_RATIO_DEPENDENCY;

	//init preproc
	CAT_Coord coordMax;
    coordMax.x = X;
    coordMax.y = Y;
    coordMax = CAT_InitPreprocessor(cellSize, globalSize, Kl, coordMax);
    if (coordMax.x * coordMax.y == 0.0) {
        printf("Error: can't initialize preprocessor\n");
        return 1;
    }
    //--------------------------
    CAT_Index arraySize = CAT_GetArraySize();
    //calculate count of pseudo and real cells in X and Y
	int countCellsInX = arraySize.i;
	int countCellsInY = arraySize.j;
	printf("%d x %d meters => %d x %d cells => %f x %f meters\n", X, Y, countCellsInX, countCellsInY, coordMax.x, coordMax.y);
    //--------------------------
	//get types from config
	COUNT_TYPE = getIntParamByName(argv[argc-1], "imageParametrs", "countType");
	COUNT_THREADS = getIntParamByName(argv[argc-1], "imageParametrs", "countThreads");
	int * typesRange = (int*)malloc(COUNT_TYPE * 2 * sizeof(int));
	char typeNomber[20];
	char typeName[30];
	for (int i = 0; i < COUNT_TYPE; i++) {
		sprintf(typeNomber,"%d",i);
		snprintf(typeName, sizeof typeName, "%s%s", "type", typeNomber);
		getCharParamByName(argv[argc-1], "imageParametrs", typeName, &typesRange[i*2]);
	}

	//rendering pseudo hex image
    hexCell *hexMat = (hexCell*)malloc(countCellsInX * countCellsInY * sizeof(hexCell));
    if(NULL == hexMat) {
		cerr << "No memory";
		return -1;
    }

	pthread_t * threads = (pthread_t*)malloc(COUNT_THREADS*sizeof(pthread_t));
	if(NULL == threads) {
		cerr << "No memory";
		return -1;
    }

	threadDataInfo * threadsInfo = (threadDataInfo*)malloc(COUNT_THREADS*sizeof(threadDataInfo));
	if(NULL == threadsInfo) {
		cerr << "No memory";
		return -1;
    }

	generateHexMat(hexMat, countCellsInX, countCellsInY, &img, threads, threadsInfo);

	int flag = CAT_FinalizePreprocessor((char *)inputFileName);
    if (flag) {
        printf("Error: can't write output file: %s\n", inputFileName);
        return 1;
    }

    return 0;
}

