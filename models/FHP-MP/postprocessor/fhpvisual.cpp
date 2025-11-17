// FHP visualizer entry point
// Version 2.0
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "iniparser/iniparser.cpp"
#include "iniparser/dictionary.cpp"

int X_size, Y_size, X2_size, Y2_size;			// Size of CA in array elements
int pixelsX=0, pixelsY=0;				// Size of bitmap
double metricX, metricY, cellsInMillimeter, pixelsInMillimeter;
int N_iter=0, N_old, S_int=0;				// Numbers of iterations and sources intencity
unsigned char *ca0, *cb0;				// CA arrays
double *mx, *my, *con, *pcon, *streamlines;		// Averaged momentum and concentration
double yVelocityCorrection;				// Momentum y-projection value correction
double excludeMeanVelocityX,excludeMeanVelocityY;	// Ratio shows which part of mean velocity should be removed
double avx1, avy1, avx2, avy2;				// Boundaries of an averaging area
double avAngle,avxCenter,avyCenter,avxSize,avySize;	// Boundaries of a tilted averaging area
double avCosinus=1;					// Cos of angle of averaging area
double avSinus=0;					// Sin of angle of averaging area

double minGasConcentration,maxGasConcentration;
double maxGasVelocity;
double minGasStreamlines,maxGasStreamlines;
int maxStreamlineIterations;
double minPowderConcentration,maxPowderConcentration;
double mingc,maxgc,maxgv,minpc,maxpc,mings,maxgs;
char inputFileName[1000]="",outputFileName[1100],gasVelocityFileName[1200];

bool makeGasConcentration;
bool makeGasVelocity;
bool makePowderConcentration;
bool makeGasStreamlines;
bool makeGasVelocityValue=false;
bool makeTextOutput=true;
bool makeAveragedColumns;
bool makeImage;
bool batchModeEnable;
bool transposeImage;
bool symmetricVicinity=false;
int batchModeStep,batchModeStart,batchModeFinish;
bool mixPowders;
bool smoothArrows,smoothLines;
int powderCode;
double gasConcentrationRadius;
double gasVelocityRadius;
double powderConcentrationRadius;
double gasStreamlinesWidth;
int gasStreamlinesPeriod,gasStreamlinesInterval;
double gasStreamlinesX1,gasStreamlinesY1,gasStreamlinesX2,gasStreamlinesY2;
int gasStreamlinesN;
double gasVelocityValueX1,gasVelocityValueY1,gasVelocityValueX2,gasVelocityValueY2;
int gasVelocityValueN;
double gasStreamlinesX[100],gasStreamlinesY[100];
double gasStreamlineStep;
int gasVelocityStep,gasStreamlinesFlag=0;
double arrowShaftWidthConst,arrowShaftWidth,arrowHeadLength,arrowHeadWidthConst,arrowHeadWidth,arrowHeadDepth,arrowLengthScale;

int gasConcentrationColor,gasVelocityColor,powderConcentrationColor,gasStreamlinesColor,wallColor;
double gasConcentrationGammaCorrection,powderConcentrationGammaCorrection,gasStreamlinesGammaCorrection,gasVelocityMagnitudeCorrection;

int read_ca_file(char *, unsigned char *);
int read_header(char *);

extern int allocate_av(void);				// Memory allocating for averaged massives
//extern int average(int, int);				// Averaging momentum and concentration
//extern int save_averaged(char *,int,int,int);	// Saving averaged momentum and concentration
extern int str2int(char*);					// Converting string to integer
extern long p_number_all(void);				// Geting number of the particles in the CA
extern int save_image(char *,double,int,int);	// Saving averaged velocity and concentration fields to file
extern int save_text(char *,double *,int,int,int,int,int);	// Saving averaged velocity and concentration fields to file
extern int save_text_transpose(char *,double *,int,int,int,int,int);	// Saving averaged velocity and concentration fields to file
extern int pixelVelocityStartX,pixelVelocityStartY,pixelVelocityFinishX,pixelVelocityFinishY;

extern int averageGasConcentration(double);
extern int averageGasVelocity(double,int);
//extern int averageGasVelocityZeroAngle(double,int);
extern void calculateMeanVelocity(double,int,double *,double *);
extern void excludeMeanVelocity(double,int,double,double);
extern int averagePowderConcentration(double,int);
extern int disposeGasStreamlines(double,double,int,int);
extern int trackGasStreamline(double,double,double,double,int);

extern void xy2ij(double,double,int *,int *);
extern double rotx(double,double);
extern double roty(double,double);

//extern int x2i(double);
//extern int y2j(int,double);
//extern int y2j(double,double);

extern void printlog(void);

int parseConfig(char * ini_name)
{
	int i,flag;
	dictionary * ini;
	char * s;
	ini = iniparser_load(ini_name);
	if (ini==NULL) {
		fprintf(stderr, "cannot parse file: %s\n", ini_name);
		return -1 ;
	}
//	iniparser_dump(ini, stderr);
	s = iniparser_getstring(ini, (char*)":inputFileName", NULL);
	if(s!=NULL)
		sprintf(inputFileName,"%s",s);
	else
		return 10000;
	flag=read_header(s);
	if(flag)
	{
		printf("\nERROR: can't open file %s\n",s);
		return -1;
	}
	s = iniparser_getstring(ini, (char*)":outputFileName", NULL);
	if(s!=NULL)
		sprintf(outputFileName,"%s",s);
	else
		sprintf(outputFileName,"%s.gif",inputFileName);
	
/*	metricX = iniparser_getdouble(ini, (char*)":X", 0.0);
	if(metricX<=0)
	{
		fprintf(stderr, "\nERROR: X<=0\n");
		return 1;
	}
	metricY = iniparser_getdouble(ini, (char*)":Y", 0.0);
	if(metricY<=0)
	{
		fprintf(stderr, "\nERROR: Y<=0\n");
		return 1;
	}
	cellsInMillimeter = iniparser_getdouble(ini, (char*)":cellsInMillimeter", 0.0);
	if(cellsInMillimeter<=0)
	{
		fprintf(stderr, "\nERROR: cellsInMillimeter <= 0 or undefined\n");
		return 1;
	}
	if((cellsInMillimeter*metricX>1000000)||(cellsInMillimeter*metricY>1000000))
	{
		fprintf(stderr, "\nERROR: cellular array too large\n");
		return 1;
	}
// Calculating size of CA in cells
	int tmp_size;
	xy2ij(metricX,0,&X_size,&tmp_size);
	xy2ij(0,metricY,&tmp_size,&Y_size);
	X_size++;
	Y_size++;
//	X_size=x2i(metricX);
//	Y_size=y2j(X_size,metricY);*/
	X2_size=X_size+2;
	Y2_size=Y_size+2;
	avx1 = iniparser_getdouble(ini, (char*)":x1", 0.0);
	avy1 = iniparser_getdouble(ini, (char*)":y1", 0.0);
	avx2 = iniparser_getdouble(ini, (char*)":x2", metricX);
	avy2 = iniparser_getdouble(ini, (char*)":y2", metricY);
	avAngle = iniparser_getdouble(ini, (char*)":angleDeg", 0.0);
	avxCenter = iniparser_getdouble(ini, (char*)":xCenter", metricX/2);
	avyCenter = iniparser_getdouble(ini, (char*)":yCenter", metricY/2);
	avxSize = iniparser_getdouble(ini, (char*)":xSize", 0.0);
	avySize = iniparser_getdouble(ini, (char*)":ySize", 0.0);
	if(avAngle)
	{
		if (avAngle<-45 || avAngle>45)
		{
			fprintf(stderr, "\nERROR: angleDeg out of range (-45..+45 deg)\n");
			return 1;
		}
		avCosinus = cos(avAngle*M_PI/180);
		avSinus = sin(avAngle*M_PI/180);
		if(avxSize<(2/cellsInMillimeter))
		{
			fprintf(stderr, "\nERROR: xSize should be at less %g mm\n",2/cellsInMillimeter);
			return 1;
		}
		if(avySize<(2/cellsInMillimeter))
		{
			fprintf(stderr, "\nERROR: ySize should be at less %g mm\n",2/cellsInMillimeter);
			return 1;
		}
		if(avx1!=0)
			fprintf(stderr, "\nWARNING: angleDeg is nonzero but x1 is defined; the definition is ignored\n");
		if(avy1!=0)
			fprintf(stderr, "\nWARNING: angleDeg is nonzero but y1 is defined; the definition is ignored\n");
		if(avx2!=metricX)
			fprintf(stderr, "\nWARNING: angleDeg is nonzero but x2 is defined; the definition is ignored\n");
		if(avy2!=metricY)
			fprintf(stderr, "\nWARNING: angleDeg is nonzero but y2 is defined; the definition is ignored\n");
		avx1 = avxCenter - avxSize*0.5;
		avy1 = avyCenter - avySize*0.5;
		avx2 = avxCenter + avxSize*0.5;
		avy2 = avyCenter + avySize*0.5;
		double x11=rotx(avx1,avy1),y11=roty(avx1,avy1);
		double x12=rotx(avx1,avy2),y12=roty(avx1,avy2);
		double x21=rotx(avx2,avy1),y21=roty(avx2,avy1);
		double x22=rotx(avx2,avy2),y22=roty(avx2,avy2);
//printf("\nDIAG: angle=%g deg, center=(%g,%g), size=(%g,%g)\n",avAngle,avxCenter,avyCenter,avxSize,avySize);
//printf("\nDIAG: x1=%g, y1=%g, x2=%g, y2=%g\n",avx1,avy1,avx2,avy2);
//printf("\nDIAG: xy11=(%g,%g), xy12=(%g,%g), xy21=(%g,%g), xy22=(%g,%g)\n",x11,y11,x12,y12,x21,y21,x22,y22);
		if (x11<0 || y11<0 || x11>metricX || y11>metricY)
		{
			fprintf(stderr, "\nERROR: corner11 (%g,%g) of the averaging area out of the range\n",x11,y11);
			return 1;
		}
		if (x12<0 || y12<0 || x12>metricX || y12>metricY)
		{
			fprintf(stderr, "\nERROR: corner21 (%g,%g) of the averaging area out of the range\n",x12,y12);
			return 1;
		}
		if (x21<0 || y21<0 || x21>metricX || y21>metricY)
		{
			fprintf(stderr, "\nERROR: corner21 (%g,%g) of the averaging area out of the range\n",x21,y21);
			return 1;
		}
		if (x22<0 || y22<0 || x22>metricX || y22>metricY)
		{
			fprintf(stderr, "\nERROR: corner22 (%g,%g) of the averaging area out of the range\n",x22,y22);
			return 1;
		}
	}
	else
	{
		if(avx1<0)
		{
			fprintf(stderr, "\nERROR: x1<0\n");
			return 1;
		}
		if(avy1<0)
		{
			fprintf(stderr, "\nERROR: y1<0\n");
			return 1;
		}
		if(avx2>metricX)
		{
			fprintf(stderr, "\nERROR: x2>X\n");
			return 1;
		}
		if(avy2>metricY)
		{
			fprintf(stderr, "\nERROR: y2>Y\n");
			return 1;
		}
		if((avx2-avx1)<(2/cellsInMillimeter))
		{
			fprintf(stderr, "\nERROR: x2-x1 should be at less %g mm\n",2/cellsInMillimeter);
			return 1;
		}
		if((avy2-avy1)<(2/cellsInMillimeter))
		{
			fprintf(stderr, "\nERROR: y2-y1 should be at less %g mm\n",2/cellsInMillimeter);
				return 1;
		}
		if(avxCenter!=metricX/2)
			fprintf(stderr, "\nWARNING: angleDeg is zero but xCenter is defined; the definition is ignored\n");
		if(avyCenter!=metricY/2)
			fprintf(stderr, "\nWARNING: angleDeg is zero but yCenter is defined; the definition is ignored\n");
		if(avxSize!=0)
			fprintf(stderr, "\nWARNING: angleDeg is zero but xSize is defined; the definition is ignored\n");
		if(avySize!=0)
			fprintf(stderr, "\nWARNING: angleDeg is zero but ySize is defined; the definition is ignored\n");
	}
	pixelsX = iniparser_getint(ini, (char*)":pixelsX", 0);
	pixelsY = iniparser_getint(ini, (char*)":pixelsY", 0);
	if((pixelsX<=0)&&(pixelsY<=0))
	{
		fprintf(stderr, "\nERROR: undefined size of bitmap\n");
		return 1;
	}
	if((pixelsX>0)&&(pixelsY>0))
	{
		pixelsInMillimeter=pixelsX/(avx2-avx1);
		flag=(int)(pixelsInMillimeter*(avy2-avy1)+0.5);
		if(flag!=pixelsY)
		{
			fprintf(stderr, "\nERROR: wrong bitmap aspect ratio\n");
			return 1;
		}
	}
	if(pixelsX<=0)
	{
		pixelsInMillimeter=pixelsY/(avy2-avy1);
		pixelsX=(int)(pixelsInMillimeter*(avx2-avx1)+0.5);
	}
	if(pixelsY<=0)
	{
		pixelsInMillimeter=pixelsX/(avx2-avx1);
		pixelsY=(int)(pixelsInMillimeter*(avy2-avy1)+0.5);
	}
	if((pixelsX<40)||(pixelsY<40)||(pixelsX>20000)||(pixelsY>20000))
	{
		fprintf(stderr, "\nERROR: wrong bitmap size %dx%d (min 40x40, max 20000x20000)\n",pixelsX,pixelsY);
		return 1;
	}

	minGasConcentration = iniparser_getdouble(ini, (char*)":minGasConcentration", -1.0);
	maxGasConcentration = iniparser_getdouble(ini, (char*)":maxGasConcentration", -1.0);
	if((minGasConcentration>=0)&&(maxGasConcentration>=0)&&(maxGasConcentration<=minGasConcentration))
	{
		printf("\nERROR: Incorrect min or max gas concentration\n");
		return 10000;
	}
	maxGasVelocity = iniparser_getdouble(ini, (char*)":maxGasVelocity", -1.0);
	if(maxGasVelocity==0)
	{
		printf("\nERROR: max gas velocity = 0\n");
		return 10000;
	}
	yVelocityCorrection = iniparser_getdouble(ini, (char*)":yVelocityCorrection", 0.9);
	if(yVelocityCorrection<0.5||yVelocityCorrection>2)
	{
		printf("\nERROR: yVelocityCorrection out of range (0.5 - 2.0)\n");
		return 10000;
	}
	excludeMeanVelocityX = iniparser_getdouble(ini, (char*)":excludeMeanVelocityX", 0.0);
	if(excludeMeanVelocityX<0.0||excludeMeanVelocityX>1.0)
	{
		printf("\nERROR: excludeMeanVelocityX out of range (0.0 - 1.0)\n");
		return 10000;
	}
	excludeMeanVelocityY = iniparser_getdouble(ini, (char*)":excludeMeanVelocityY", 0.0);
	if(excludeMeanVelocityY<0.0||excludeMeanVelocityY>1.0)
	{
		printf("\nERROR: excludeMeanVelocityY out of range (0.0 - 1.0)\n");
		return 10000;
	}

	minPowderConcentration = iniparser_getdouble(ini, (char*)":minPowderConcentration", -1.0);
	maxPowderConcentration = iniparser_getdouble(ini, (char*)":maxPowderConcentration", -1.0);
	if((minPowderConcentration>=0)&&(maxPowderConcentration>=0)&&(maxPowderConcentration<=minPowderConcentration))
	{
		printf("\nERROR: Incorrect min or max powder concentration\n");
		return 10000;
	}
	makeGasConcentration = !!iniparser_getboolean(ini, (char*)":makeGasConcentration", 1);
	makeGasVelocity = !!iniparser_getboolean(ini, (char*)":makeGasVelocity", 1);
	makeGasStreamlines = !!iniparser_getboolean(ini, (char*)":makeGasStreamlines", 1);
	if(makeGasStreamlines&&avAngle)
	{
		printf("\nWARNING: GasStreamlines is unabled because the angle is not zero\n");
		makeGasStreamlines=0;
	}
	makePowderConcentration = !!iniparser_getboolean(ini, (char*)":makePowderConcentration", 1);
	if(!makeGasConcentration&&!makeGasVelocity&&!makeGasStreamlines&&!makePowderConcentration)
	{
		printf("\nERROR: empty task\n");
		return 10000;
	}
	makeImage = !!iniparser_getboolean(ini, (char*)":makeImage", 1);
	makeTextOutput = !!iniparser_getboolean(ini, (char*)":makeTextOutput", 1);
	if(!makeTextOutput&&!makeImage)
	{
		printf("\nERROR: empty output\n");
		return 10000;
	}
	makeAveragedColumns = !!iniparser_getboolean(ini, (char*)":makeAveragedColumns", false);
	symmetricVicinity = !!iniparser_getboolean(ini, (char*)":symmetricVicinity", false);
	transposeImage = !!iniparser_getboolean(ini, (char*)":transposeImage", false);
	mixPowders = !!iniparser_getboolean(ini, (char*)":mixPowders", 1);
	smoothArrows = !!iniparser_getboolean(ini, (char*)":smoothArrows", 1);
	smoothLines = !!iniparser_getboolean(ini, (char*)":smoothLines", 1);
	powderCode = iniparser_getint(ini, (char*)":powderCode", 1);
	if((powderCode<0)||(powderCode>7))
	{
		fprintf(stderr, "\nERROR: Powder code is incorrect\n");
		return 1;
	}
	gasConcentrationRadius = iniparser_getdouble(ini, (char*)":gasConcentrationRadius", 5/cellsInMillimeter);
	if(gasConcentrationRadius<0)
	{
		fprintf(stderr, "\nERROR: gasConcentrationRadius < 0\n");
		return 1;
	}
	gasVelocityRadius = iniparser_getdouble(ini, (char*)":gasVelocityRadius", 10/cellsInMillimeter);
	if(gasVelocityRadius<0)
	{
		fprintf(stderr, "\nERROR: gasVelocityRadius < 0\n");
		return 1;
	}
	powderConcentrationRadius = iniparser_getdouble(ini, (char*)":powderConcentrationRadius", 3/cellsInMillimeter);
	if(powderConcentrationRadius<0)
	{
		fprintf(stderr, "\nERROR: powderConcentrationRadius < 0\n");
		return 1;
	}
	gasVelocityStep = iniparser_getint(ini, (char*)":gasVelocityStep", 20);
	if(gasVelocityStep<=0)
	{
		fprintf(stderr, "\nERROR: gasVelocityStep <= 0\n");
		return 1;
	}
	arrowLengthScale = iniparser_getdouble(ini, (char*)":arrowLengthScale", 1.2);
	if((arrowLengthScale<0.5)||(arrowLengthScale>2))
	{
		fprintf(stderr, "\nERROR: arrowLengthScale is wrong\n");
		return 1;
	}
	arrowShaftWidthConst = iniparser_getdouble(ini, (char*)":arrowShaftWidthConst", 1.0);
	if((arrowShaftWidthConst<0)||(arrowShaftWidthConst>10))
	{
		fprintf(stderr, "\nERROR: arrowShaftWidthConst is wrong\n");
		return 1;
	}
	arrowShaftWidth = iniparser_getdouble(ini, (char*)":arrowShaftWidth", 4);
	if((arrowShaftWidth<0)||(arrowShaftWidth>10))
	{
		fprintf(stderr, "\nERROR: arrowShaftWidth is wrong\n");
		return 1;
	}
	arrowHeadLength = iniparser_getdouble(ini, (char*)":arrowHeadLength", 0.5);
	if((arrowHeadLength<0)||(arrowHeadLength>1))
	{
		fprintf(stderr, "\nERROR: arrowHeadLength is wrong\n");
		return 1;
	}
	arrowHeadWidthConst = iniparser_getdouble(ini, (char*)":arrowHeadWidthConst", 1.0);
	if((arrowHeadWidthConst<0)||(arrowHeadWidthConst>10))
	{
		fprintf(stderr, "\nERROR: arrowHeadWidthConst is wrong\n");
		return 1;
	}
	arrowHeadWidth = iniparser_getdouble(ini, (char*)":arrowHeadWidth", 0.2);
	if((arrowHeadWidth<0)||(arrowHeadWidth>1))
	{
		fprintf(stderr, "\nERROR: arrowHeadWidth is wrong\n");
		return 1;
	}
	arrowHeadDepth = iniparser_getdouble(ini, (char*)":arrowHeadDepth", 0.1);
	if((arrowHeadDepth<0)||(arrowHeadDepth>0.5))
	{
		fprintf(stderr, "\nERROR: arrowHeadDepth is wrong\n");
		return 1;
	}
	gasStreamlinesWidth = iniparser_getdouble(ini, (char*)":gasStreamlinesWidth", 2.0);
	if((gasStreamlinesWidth<0)||(gasStreamlinesWidth>10))
	{
		fprintf(stderr, "\nERROR: gasStreamlinesWidth is wrong\n");
		return 1;
	}
	gasStreamlineStep = iniparser_getdouble(ini, (char*)":gasStreamlineStep", 0.1/cellsInMillimeter);
	if((gasStreamlineStep<(0.01/cellsInMillimeter))||(gasStreamlineStep>(1/cellsInMillimeter)))
	{
		fprintf(stderr, "\nERROR: gasStreamlineStep is wrong\n");
		return 1;
	}
	flag=(int)(10*(X_size+Y_size)*(avx2-avx1)/metricX*(avy2-avy1)/metricY);
	if(flag<(10*(pixelsX+pixelsY)))
		flag=10*(pixelsX+pixelsY);
	maxStreamlineIterations = iniparser_getint(ini, (char*)":maxStreamlineIterations", flag);
	if((maxStreamlineIterations<0)||(maxStreamlineIterations>(100*flag)))
	{
		fprintf(stderr, "\nERROR: maxStreamlineIterations out of range\n");
		return 1;
	}
	gasStreamlinesInterval = iniparser_getint(ini, (char*)":gasStreamlinesInterval", 20);
	if((gasStreamlinesInterval<2)||(gasStreamlinesInterval>(pixelsX/2))||(gasStreamlinesInterval>(pixelsY/2)))
	{
		fprintf(stderr, "\nERROR: gasStreamlinesInterval out of range\n");
		return 1;
	}
	flag=(int)(0.2*gasStreamlinesWidth/gasStreamlineStep/pixelsInMillimeter);
	flag+=!flag;
	gasStreamlinesPeriod = iniparser_getint(ini, (char*)":gasStreamlinesPeriod", flag);
	if((gasStreamlinesPeriod<1)||(gasStreamlinesPeriod>flag))
	{
		fprintf(stderr, "\nERROR: gasStreamlinesPeriod out of range\n");
		return 1;
	}
	minGasStreamlines = iniparser_getdouble(ini, (char*)":minGasStreamlines", -1.0);
	maxGasStreamlines = iniparser_getdouble(ini, (char*)":maxGasStreamlines", 0.5*gasStreamlinesWidth/((double)gasStreamlinesPeriod*gasStreamlinesPeriod));
	if((minGasStreamlines>=0)&&(maxGasStreamlines>=0)&&(maxGasStreamlines<=minGasStreamlines))
	{
		printf("\nERROR: Incorrect min or max gas streamlines\n");
		return 10000;
	}
	for(i=0;i<100;i++)
	{
		char s[1200];
		sprintf(s,":gasStreamlineX%d",i);
		gasStreamlinesX[i] = iniparser_getdouble(ini, s, -1.0);
		sprintf(s,":gasStreamlineY%d",i);
		gasStreamlinesY[i] = iniparser_getdouble(ini, s, -1.0);
		if((gasStreamlinesX[i]>=0)||(gasStreamlinesX[i]>=0))
		{
			if((gasStreamlinesX[i]<avx1)||(gasStreamlinesX[i]>avx2))
			{
				fprintf(stderr, "\nERROR: gasStreamlineX%d out of range\n",i);
				return 1;
			}
			if((gasStreamlinesY[i]<avy1)||(gasStreamlinesY[i]>avy2))
			{
				fprintf(stderr, "\nERROR: gasStreamlineY%d out of range\n",i);
				return 1;
			}
			gasStreamlinesFlag++;
		}
	}
	gasStreamlinesX1 = iniparser_getdouble(ini, (char*)":gasStreamlinesX1", -1.0);
	gasStreamlinesY1 = iniparser_getdouble(ini, (char*)":gasStreamlinesY1", -1.0);
	gasStreamlinesX2 = iniparser_getdouble(ini, (char*)":gasStreamlinesX2", -1.0);
	gasStreamlinesY2 = iniparser_getdouble(ini, (char*)":gasStreamlinesY2", -1.0);
	gasStreamlinesN = iniparser_getint(ini, (char*)":gasStreamlinesN", -1);
	if((gasStreamlinesX1>=0)||(gasStreamlinesY1>=0)||(gasStreamlinesX2>=0)||(gasStreamlinesY2>=0)||(gasStreamlinesN>=0))
	{
		if((gasStreamlinesX1<avx1)||(gasStreamlinesX1>avx2))
		{
			fprintf(stderr, "\nERROR: gasStreamlinesX1 out of range\n");
			return 1;
		}
		if((gasStreamlinesY1<avy1)||(gasStreamlinesY1>avy2))
		{
			fprintf(stderr, "\nERROR: gasStreamlinesY1 out of range\n");
			return 1;
		}
		if((gasStreamlinesX2<avx1)||(gasStreamlinesX2>avx2))
		{
			fprintf(stderr, "\nERROR: gasStreamlinesX2 out of range\n");
			return 1;
		}
		if((gasStreamlinesY2<avy1)||(gasStreamlinesY2>avy2))
		{
			fprintf(stderr, "\nERROR: gasStreamlinesY2 out of range\n");
			return 1;
		}
		if((gasStreamlinesX1==gasStreamlinesX2)&&(gasStreamlinesY1==gasStreamlinesY2))
		{
			if(gasStreamlinesN!=1)
			{
				fprintf(stderr, "\nERROR: streamlines diapason is empty\n");
				return 1;
			}
		}
		else
			if((gasStreamlinesN<2)||(gasStreamlinesN>100))
			{
				fprintf(stderr, "\nERROR: number of streamlines out of range\n");
				return 1;
			}
	}
	gasConcentrationColor = iniparser_getint(ini, (char*)":gasConcentrationColor", 0xff0000);
	if((gasConcentrationColor<0)||(gasConcentrationColor>0xffffff))
	{
		fprintf(stderr, "\nERROR: gasConcentrationColor out of range\n");
		return 1;
	}
	gasVelocityColor = iniparser_getint(ini, (char*)":gasVelocityColor", 0xffff00);
	if((gasVelocityColor<0)||(gasVelocityColor>0xffffff))
	{
		fprintf(stderr, "\nERROR: gasVelocityColor out of range\n");
		return 1;
	}
	gasStreamlinesColor = iniparser_getint(ini, (char*)":gasStreamlinesColor", 0xffff);
	if((gasStreamlinesColor<0)||(gasStreamlinesColor>0xffffff))
	{
		fprintf(stderr, "\nERROR: gasStreamlinesColor out of range\n");
		return 1;
	}
	powderConcentrationColor = iniparser_getint(ini, (char*)":powderConcentrationColor", 0x0000ff);
	if((powderConcentrationColor<0)||(powderConcentrationColor>0xffffff))
	{
		fprintf(stderr, "\nERROR: powderConcentrationColor out of range\n");
		return 1;
	}
	wallColor = iniparser_getint(ini, (char*)":wallColor", 0x006400);
	if((wallColor<0)||(wallColor>0xffffff))
	{
		fprintf(stderr, "\nERROR: wallColor out of range\n");
		return 1;
	}
	gasConcentrationGammaCorrection = iniparser_getdouble(ini, (char*)":gasConcentrationGammaCorrection", 2.0);
	if((gasConcentrationGammaCorrection<0.01)||(gasConcentrationGammaCorrection>100))
	{
		fprintf(stderr, "\nERROR: gasConcentrationGammaCorrection is wrong\n");
		return 1;
	}
	gasStreamlinesGammaCorrection = iniparser_getdouble(ini, (char*)":gasStreamlinesGammaCorrection", 1.0);
	if((gasStreamlinesGammaCorrection<0.01)||(gasStreamlinesGammaCorrection>100))
	{
		fprintf(stderr, "\nERROR: gasStreamlinesGammaCorrection is wrong\n");
		return 1;
	}
	gasVelocityMagnitudeCorrection = iniparser_getdouble(ini, (char*)":gasVelocityMagnitudeCorrection", 2.0);
	if((gasVelocityMagnitudeCorrection<0.01)||(gasVelocityMagnitudeCorrection>100))
	{
		fprintf(stderr, "\nERROR: gasVelocityMagnitudeCorrection is wrong\n");
		return 1;
	}
	powderConcentrationGammaCorrection = iniparser_getdouble(ini, (char*)":powderConcentrationGammaCorrection", 5.0);
	if((powderConcentrationGammaCorrection<0.01)||(powderConcentrationGammaCorrection>100))
	{
		fprintf(stderr, "\nERROR: powderConcentrationGammaCorrection is wrong\n");
		return 1;
	}
	batchModeStart = iniparser_getint(ini, (char*)":batchModeStart", 0);
	if((batchModeStart<0)||(batchModeStart>999999))
	{
		fprintf(stderr, "\nERROR: batchModeStart out of range\n");
		return 1;
	}
	batchModeFinish = iniparser_getint(ini, (char*)":batchModeFinish", 0);
	if((batchModeFinish<0)||(batchModeFinish>999999))
	{
		fprintf(stderr, "\nERROR: batchModeFinish out of range\n");
		return 1;
	}
	if(batchModeStart>batchModeFinish)
	{
		fprintf(stderr, "\nERROR: batchModeStart > batchModeFinish\n");
		return 1;
	}
	batchModeStep = iniparser_getint(ini, (char*)":batchModeStep", 1);
	batchModeEnable=!((batchModeStart==0)&&(batchModeFinish==0)&&(batchModeStep==1));
	if(batchModeEnable)
		makeGasStreamlines=false;
// fprintf(stderr, "\nbatchModeStep = %d, batchModeStart = %d, batchModeFinish = %d, batchModeEnable = %d\n", batchModeStep,batchModeStart,batchModeFinish,batchModeEnable);
	if((batchModeStep<=0)&&batchModeEnable)
	{
		fprintf(stderr, "\nERROR: batchModeStep out of range\n");
		return 1;
	}

	gasVelocityValueX1 = iniparser_getdouble(ini, (char*)":gasVelocityValueX1", -1.0);
	gasVelocityValueY1 = iniparser_getdouble(ini, (char*)":gasVelocityValueY1", -1.0);
	gasVelocityValueX2 = iniparser_getdouble(ini, (char*)":gasVelocityValueX2", -1.0);
	gasVelocityValueY2 = iniparser_getdouble(ini, (char*)":gasVelocityValueY2", -1.0);
	gasVelocityValueN = iniparser_getint(ini, (char*)":gasVelocityValueN", -1);
	if((gasVelocityValueX1>=0)||(gasVelocityValueY1>=0)||(gasVelocityValueX2>=0)||(gasVelocityValueY2>=0)||(gasVelocityValueN>=0))
	{
		makeGasVelocityValue=true;
		sprintf(gasVelocityFileName,"%s.txt",outputFileName);
		if((gasVelocityValueX1<avx1)||(gasVelocityValueX1>avx2))
		{
			fprintf(stderr, "\nERROR: gasVelocityValueX1 out of range\n");
			return 1;
		}
		if((gasVelocityValueY1<avy1)||(gasVelocityValueY1>avy2))
		{
			fprintf(stderr, "\nERROR: gasVelocityValueY1 out of range\n");
			return 1;
		}
		if((gasVelocityValueX2<avx1)||(gasVelocityValueX2>avx2))
		{
			fprintf(stderr, "\nERROR: gasVelocityValueX2 out of range\n");
			return 1;
		}
		if((gasVelocityValueY2<avy1)||(gasVelocityValueY2>avy2))
		{
			fprintf(stderr, "\nERROR: gasVelocityValueY2 out of range\n");
			return 1;
		}
		if((gasVelocityValueX1==gasVelocityValueX2)&&(gasVelocityValueY1==gasVelocityValueY2))
		{
			if(gasVelocityValueN!=1)
			{
				fprintf(stderr, "\nERROR: streamlines diapason is empty\n");
				return 1;
			}
		}
		else
			if((gasVelocityValueN<2)||(gasVelocityValueN>100))
			{
				fprintf(stderr, "\nERROR: number of streamlines out of range\n");
				return 1;
			}
	}


	iniparser_freedict(ini);
	return 0 ;
}

int main(int argc, char* argv[])
{
	int i, j, flag=0;
//	int flag_fn=0, flag_r=0, flag_s=0;		// File name, radius, and step flags
	char s[1200];

	printf("FHP visualizer v.2.0\n(c) Yuri Medvedev, ICM&MG SB RAS, 2006-2011\n");
//	printf("\nfloat=%d, double=%d, long double=%d\n\n",sizeof(float),sizeof(double),sizeof(long double));

// checking command line
	if(argc!=2)
	{
		printf("usage: fhpvisual filename.conf\n");
		return 1;
	}
	flag=parseConfig(argv[1]);
	if(flag)
		return 1;

	printf("\nMetric size %gx%g mm, accuracy %g mm\n",metricX,metricY,10/cellsInMillimeter);
	printf("Model size %dx%d cells, scale %g cells/mm\n",X_size,Y_size,cellsInMillimeter);
	printf("Averaging area (%g,%g)-(%g,%g), size %gx%g mm, angle %f deg\n",avx1,avy1,avx2,avy2,avx2-avx1,avy2-avy1,avAngle);
	printf("Bitmap size %dx%d pixels, scale %g pixels/mm\n",pixelsX,pixelsY,pixelsInMillimeter);
	if(batchModeEnable)
		printf("Batch mode start %d, finish %d, step %d\n",batchModeStart,batchModeFinish,batchModeStep);
	else
		printf("Batch mode is disabled\n");

// allocating memory
	flag=allocate_av();
	if(flag)
	{
		printf("\nERROR: not enough memory\n");
		return 1;
	}

	for(i=batchModeStart;i<=batchModeFinish;i+=batchModeStep)
	{
// reading data
		printf("\nReading input files");
		if(batchModeEnable)
			sprintf(s,"i%08d-%s",i,inputFileName);
		else
			sprintf(s,"%s",inputFileName);
		printf("\n- %s",s);
		flag=read_ca_file(s,ca0);
		switch(flag)
		{
		case 0:break;
		case 1:if(batchModeEnable){printf("\nWARNING: can't open file %s\n",s);continue;}
				printf("\nERROR: can't open file %s\n",s);return 1;
		case 2:printf("\nERROR: unknown format in file %s\n",s);return 1;
		case 3:printf("\nERROR: wrong version of file %s\n",s);return 1;
		case 4:printf("\nERROR: can't read file %s\n",s);return 1;
		case 5:printf("\nERROR: wrong size X or wrong parameter cellsInMillimeter with file %s\n",s);return 1;
		case 6:printf("\nERROR: wrong size Y or wrong parameter cellsInMillimeter with file %s\n",s);return 1;
		default:printf("\nERROR: unknown error with file %s\n",s);return 1;
		}
		if(makePowderConcentration)
		{
			if(batchModeEnable)
				sprintf(s,"i%08d-%s2",i,inputFileName);
			else
				sprintf(s,"%s2",inputFileName);
			printf("\n- %s",s);
			flag=read_ca_file(s,cb0);
			switch(flag)
			{
			case 0:break;
			case 1:if(batchModeEnable){printf("\nWARNING: can't open file %s\n",s);continue;}
					printf("\nERROR: can't open file %s\n",s);return 1;
			case 2:printf("\nERROR: unknown format in file %s\n",s);return 1;
			case 3:printf("\nERROR: wrong version of file %s\n",s);return 1;
			case 4:printf("\nERROR: can't read file %s\n",s);return 1;
			case 5:printf("\nERROR: wrong size X or wrong parameter cellsInMillimeter with file %s\n",s);return 1;
			case 6:printf("\nERROR: wrong size Y or wrong parameter cellsInMillimeter with file %s\n",s);return 1;
			default:printf("\nERROR: unknown error with file %s\n",s);return 1;
			}
		}
//		printf("\n");

// averaging
		printf("\nAveraging start");
		fflush(stdout);
		if(makeGasConcentration)
		{
			printf("\n- gas concentration (r=%g mm)",gasConcentrationRadius);
			fflush(stdout);
			averageGasConcentration(gasConcentrationRadius);
			printf(" [%g, %g]",mingc,maxgc);
			fflush(stdout);
			if(makeTextOutput)
			{
				if(batchModeEnable)
					sprintf(s,"i%08d-%s-concentr.xls",i,outputFileName);
				else
					sprintf(s,"%s-concentr.xls",outputFileName);
				if(transposeImage)
					flag=save_text_transpose(s,con,0,pixelsX,0,pixelsY,gasVelocityStep);
				else
					flag=save_text(s,con,0,pixelsX,0,pixelsY,gasVelocityStep);
				if(flag)
				{
					printf("\nERROR: can't write file %s\n",s);
					return 1;
				}
			}
		}
		if(makeGasStreamlines)
		{
			printf("\n- gas streamlines");
			fflush(stdout);
			if((gasStreamlinesN<1)&&!gasStreamlinesFlag)
				disposeGasStreamlines(gasVelocityRadius,gasStreamlineStep,maxStreamlineIterations,gasStreamlinesInterval);
			if(gasStreamlinesN>1)
			{
				double x=gasStreamlinesX1,y=gasStreamlinesY1;
				double stepX=(gasStreamlinesX2-gasStreamlinesX1)/(gasStreamlinesN-1);
				double stepY=(gasStreamlinesY2-gasStreamlinesY1)/(gasStreamlinesN-1);
//printf("\nStreamlines (%g,%g)-(%g,%g), steps %g and %g mm\n",gasStreamlinesX1,gasStreamlinesY1,gasStreamlinesX2,gasStreamlinesY2,stepX,stepY);
				for(j=0;j<gasStreamlinesN;j++)
				{
					trackGasStreamline(x,y,gasVelocityRadius,gasStreamlineStep,maxStreamlineIterations);
					x+=stepX;
					y+=stepY;
				}
			}
			if(gasStreamlinesN==1)
				trackGasStreamline(gasStreamlinesX1,gasStreamlinesY1,gasVelocityRadius,gasStreamlineStep,maxStreamlineIterations);
			if(gasStreamlinesFlag)
				for(j=0;j<100;j++)
					if(gasStreamlinesX[j]>=0)
						trackGasStreamline(gasStreamlinesX[j],gasStreamlinesY[j],gasVelocityRadius,gasStreamlineStep,maxStreamlineIterations);
			printf(" [%g, %g]",mings,maxgs);
			fflush(stdout);
		}
		if(makeGasVelocity)
		{
			printf("\n- gas velocity (r=%g mm, step=%d pixels, yCorrection=%g",
				gasVelocityRadius,gasVelocityStep,yVelocityCorrection);
			fflush(stdout);
//			if(avAngle)
				averageGasVelocity(gasVelocityRadius,gasVelocityStep);
//			else
//				averageGasVelocityZeroAngle(gasVelocityRadius,gasVelocityStep);
			printf(", max=%g)",maxgv);
			fflush(stdout);

			if(excludeMeanVelocityX||excludeMeanVelocityY)
			{
				double meanVelocityX, meanVelocityY;
				calculateMeanVelocity(gasVelocityRadius,gasVelocityStep,&meanVelocityX,&meanVelocityY);
				excludeMeanVelocity(gasVelocityRadius,gasVelocityStep,
					meanVelocityX*excludeMeanVelocityX,meanVelocityY*excludeMeanVelocityY);
				printf("\n- gas velocity correction (excludeX=%g*%g, excludeY=%g*%g, new max=%g)",
					excludeMeanVelocityX,meanVelocityX,excludeMeanVelocityY,meanVelocityY,maxgv);
				fflush(stdout);
			}

			if(makeTextOutput)
			{
				if(batchModeEnable)
					sprintf(s,"i%08d-%s-vel-x.xls",i,outputFileName);
				else
					sprintf(s,"%s-vel-x.xls",outputFileName);
				if(transposeImage)
					flag=save_text_transpose(s,mx,pixelVelocityStartX,pixelVelocityFinishX,pixelVelocityStartY,pixelVelocityFinishY,gasVelocityStep);
				else
					flag=save_text(s,mx,pixelVelocityStartX,pixelVelocityFinishX,pixelVelocityStartY,pixelVelocityFinishY,gasVelocityStep);
				if(flag)
				{
					printf("\nERROR: can't write file %s\n",s);
					return 1;
				}
				if(batchModeEnable)
					sprintf(s,"i%08d-%s-vel-y.xls",i,outputFileName);
				else
					sprintf(s,"%s-vel-y.xls",outputFileName);
				if(transposeImage)
				flag=save_text_transpose(s,my,pixelVelocityStartX,pixelVelocityFinishX,pixelVelocityStartY,pixelVelocityFinishY,gasVelocityStep);
				else
				flag=save_text(s,my,pixelVelocityStartX,pixelVelocityFinishX,pixelVelocityStartY,pixelVelocityFinishY,gasVelocityStep);
				if(flag)
				{
					printf("\nERROR: can't write file %s\n",s);
					return 1;
				}
			}
		}
		if(makePowderConcentration)
		{
			printf("\n- powder concentration (r=%g mm)",powderConcentrationRadius);
			fflush(stdout);
			averagePowderConcentration(powderConcentrationRadius,powderCode);
			printf(" [%g, %g]",minpc,maxpc);
			fflush(stdout);
		}
/*		if(makeGasVelocityValue)
		{
			printf("\n- gas velocity value");
			averageGasVelocityValue(gvf,gasVelocityValueX1,gasVelocityValueY1,gasVelocityValueX2,gasVelocityValueY2,gasVelocityValueN,gasVelocityRadius,gasVelocityStep,);
		}*/
//		printf("\n");

		if(batchModeEnable)
			sprintf(s,"i%08d-%s",i,outputFileName);
		else
			sprintf(s,"%s",outputFileName);
		if(makeImage)
		{
			flag=save_image(s,gasVelocityRadius,gasVelocityStep,powderCode);
			if(flag)
			{
				printf("\nERROR: can't write file %s\n",s);
				return 1;
			}
			if(system("mogrify -format png *.tga;rm *.tga"))
			{
				printf("\nERROR: can't convert tga to png\n");
				return 1;
			}
		}
	}
	if(batchModeEnable)
		printf("\nVisualizer batch mode finished\n");
	else
		printf("\nVisualizer finished\n");
	fflush(stdout);

//	sprintf(s,"start %s",outputFileName);
//	system(s);

	return 0;
}

