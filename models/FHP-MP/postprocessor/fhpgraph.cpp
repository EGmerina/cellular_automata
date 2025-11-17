// FHP graphycs
// Version 2.0
//

#include <climits>
#include <cstring>
#include <memory>
#include "ImageStone.h"

#ifdef _WIN32
#include <windows.h>
#define uint64_t __int64			// Visual C++ v.6.0
#else
#include <inttypes.h>				// gcc++
#endif

#define CONVENTIONAL 0
#define WALL 1
#define SOURCE 2
#define ATMOSPHERE 3
#define INLET 4
#define OUTLET 5
#define ARRLENGTH 1.5

unsigned char *bmp_r,*bmp_g,*bmp_b;
int pixelVelocityStartX,pixelVelocityFinishX,pixelVelocityStartY,pixelVelocityFinishY;
long l0;

extern double d2(int,int,int,int);		// Square distanse between two cells
extern long xy2l(int,int,int);			// Gets index number from coordinates
extern int X_size, Y_size;
extern int pixelsX, pixelsY;
extern bool transposeImage;
extern unsigned char *ca0;
extern double *mx, *my, *con, *pcon, *streamlines;			// Averaged momentum and concentration
extern double minGasConcentration,maxGasConcentration,maxGasVelocity,minPowderConcentration,maxPowderConcentration;	// Min & max values for scaling
extern bool makeGasConcentration;
extern bool makeGasVelocity;
extern bool makePowderConcentration;
extern bool makeGasStreamlines;
extern bool mixPowders;
extern bool smoothArrows;
extern int gasConcentrationColor,gasVelocityColor,powderConcentrationColor,gasStreamlinesColor,wallColor;
extern bool point_in_triangle(double,double,double,double,double,double,double,double);
extern double arrowShaftWidthConst,arrowShaftWidth,arrowHeadLength,arrowHeadWidthConst,arrowHeadWidth,arrowHeadDepth,arrowLengthScale,pixelsInMillimeter;
extern double mingc,maxgc,maxgv,minpc,maxpc,mings,maxgs;
extern double gasConcentrationGammaCorrection,powderConcentrationGammaCorrection,gasStreamlinesGammaCorrection,gasVelocityMagnitudeCorrection;

void bmp_erase(void)
{
	long l;
	for(l=0;l<pixelsX*pixelsY;l++)
		bmp_r[l]=bmp_g[l]=bmp_b[l]=0;
}

int bmp_init(void)
{
	uint64_t l=pixelsX*pixelsY*sizeof(int);
	if(l>=(((uint64_t)1)<<(sizeof(int)*8-1)))
		return 2400;
	l0=pixelsX*pixelsY;
	bmp_r=(unsigned char *)malloc(pixelsX*pixelsY*sizeof(unsigned char));
	if(bmp_r==NULL)
		return 2401;
	bmp_g=(unsigned char *)malloc(pixelsX*pixelsY*sizeof(unsigned char));
	if(bmp_g==NULL)
		return 2402;
	bmp_b=(unsigned char *)malloc(pixelsX*pixelsY*sizeof(unsigned char));
	if(bmp_b==NULL)
		return 2403;
	bmp_erase();
	return 0;
}

void bmp_write_color(char *fname)
{
	int i,j;
	long l;
	FCObjImage bmp;
	bmp.Create(pixelsX,pixelsY,24);
	for(i=0;i<pixelsX;i++)
		for(j=0;j<pixelsY;j++)
		{
			l=pixelsY*i+j;
			bmp.SetPixelData(i,j,bmp_b[l]+256*bmp_g[l]+65536*bmp_r[l]);
		}
	bmp.ConvertTo24Bit();
	bmp.Save(fname);
}

void bmp_write_color_transpose(char *fname)
{
	int i,j;
	long l;
	FCObjImage bmp;
	bmp.Create(pixelsY,pixelsX,24);
	for(i=0;i<pixelsX;i++)
		for(j=0;j<pixelsY;j++)
		{
			l=pixelsY*i+j;
			bmp.SetPixelData(j,i,bmp_b[l]+256*bmp_g[l]+65536*bmp_r[l]);
		}
	bmp.ConvertTo24Bit();
	bmp.Save(fname);
}

void bmp_fin_color(char *fname)
{
	if(transposeImage)
		bmp_write_color_transpose(fname);
	else
		bmp_write_color(fname);
	free(bmp_b);
	free(bmp_g);
	free(bmp_r);
}

void drawArrow(double x0,double y0,double dx, double dy)
{
	int i,j,k,l,pixelColor,color;
	double x,y,transp;
	double arrowLength=sqrt(dx*dx+dy*dy);
	double shaftWidth=arrowShaftWidthConst+arrowLength*arrowShaftWidth*0.01;
	double shaftLength=arrowLength+arrowLength*arrowHeadLength*(arrowHeadDepth-1);
	double nx=dy/arrowLength*0.5;
	double ny=dx/arrowLength*0.5;
	double x9=x0+dx*(1-arrowHeadLength);
	double y9=y0+dy*(1-arrowHeadLength);

	double x1=x0+shaftWidth*nx;
	double y1=y0-shaftWidth*ny;
	double x2=x0-shaftWidth*nx;
	double y2=y0+shaftWidth*ny;
	double x3=x1+dx*shaftLength/arrowLength;
	double y3=y1+dy*shaftLength/arrowLength;
	double x4=x2+dx*shaftLength/arrowLength;
	double y4=y2+dy*shaftLength/arrowLength;
	double x5=x9+(shaftWidth+arrowHeadWidth*arrowLength+arrowHeadWidthConst)*nx;
	double y5=y9-(shaftWidth+arrowHeadWidth*arrowLength+arrowHeadWidthConst)*ny;
	double x6=x9-(shaftWidth+arrowHeadWidth*arrowLength+arrowHeadWidthConst)*nx;
	double y6=y9+(shaftWidth+arrowHeadWidth*arrowLength+arrowHeadWidthConst)*ny;
	double x7=x0+dx;
	double y7=y0+dy;
	double x8=x0+dx*shaftLength/arrowLength;
	double y8=y0+dy*shaftLength/arrowLength;

	double m1=x1<x2?x1:x2;
	double m2=x3<x4?x3:x4;
	double m3=x5<x6?x5:x6;
	m1=m1<m2?m1:m2;
	m3=m3<x7?m3:x7;
	m1=m1<m3?m1:m3;
	int i1=m1-1;
	if(i1<0)i1=0;

	m1=y1<y2?y1:y2;
	m2=y3<y4?y3:y4;
	m3=y5<y6?y5:y6;
	m1=m1<m2?m1:m2;
	m3=m3<y7?m3:y7;
	m1=m1<m3?m1:m3;
	int j1=m1-1;
	if(j1<0)j1=0;

	m1=x1>x2?x1:x2;
	m2=x3>x4?x3:x4;
	m3=x5>x6?x5:x6;
	m1=m1>m2?m1:m2;
	m3=m3>x7?m3:x7;
	m1=m1>m3?m1:m3;
	int i2=m1+2;
	if(i2>pixelsX)i2=pixelsX;

	m1=y1>y2?y1:y2;
	m2=y3>y4?y3:y4;
	m3=y5>y6?y5:y6;
	m1=m1>m2?m1:m2;
	m3=m3>y7?m3:y7;
	m1=m1>m3?m1:m3;
	int j2=m1+2;
	if(j2>pixelsY)j2=pixelsY;

//printf("\n\n(%g, %g) - x0, y0\n(%g, %g) - dx, dy\n(%g, %g) - nx, ny\n",x0,y0,dx,dy,nx,ny);
//printf("\n\n%g - length\n%g - width\n%g - shaft\n(%g, %g) - point\n",arrowLength,arrowWidth,shaftLength,x7,y7);
//printf("\n\n(%g, %g)\n(%g, %g)\n(%g, %g)\n(%g, %g)\n(%g, %g)\n(%g, %g)\n(%g, %g)\n(%d, %d) - min\n(%d, %d) - max\n",
//		x1,y1,x2,y2,x3,y3,x4,y4,x5,y5,x6,y6,x7,y7,i1,j1,i2,j2);
//printf("\n\n(%d, %d) - i1, j1\n(%d, %d) - i2, j2\n",i1,j1,i2,j2);
//return;
//printf(":");fflush(stdout);
//	arrowCounter++;

	for(i=i1;i<i2;i++)
		for(j=j1;j<j2;j++)
		{
			pixelColor=0;
			k=i+1;
			l=j+1;
			for(x=i;x<k;x+=0.0625)
				for(y=j;y<l;y+=0.0625)
					pixelColor+=(point_in_triangle(x1,y1,x2,y2,x3,y3,x,y)||
								point_in_triangle(x4,y4,x2,y2,x3,y3,x,y)||
								point_in_triangle(x5,y5,x7,y7,x8,y8,x,y)||
								point_in_triangle(x6,y6,x7,y7,x8,y8,x,y));
//			if(pixelColor>255)pixelColor=255;
			if(!smoothArrows)pixelColor=pixelColor>127?255:0;
//			if(1)			// without transparency
			{
				transp=1-(int)(pixelColor*0.003921568627451);
				bmp_r[pixelsY*i+j]*=transp;
				bmp_g[pixelsY*i+j]*=transp;
				bmp_b[pixelsY*i+j]*=transp;
			}
			color=bmp_r[pixelsY*i+j]+((((gasVelocityColor>>16)&0xff)*pixelColor)>>8);
			bmp_r[pixelsY*i+j]=color>255?255:color;
			color=bmp_g[pixelsY*i+j]+((((gasVelocityColor>>8)&0xff)*pixelColor)>>8);
			bmp_g[pixelsY*i+j]=color>255?255:color;
			color=bmp_b[pixelsY*i+j]+(((gasVelocityColor&0xff)*pixelColor)>>8);
			bmp_b[pixelsY*i+j]=color>255?255:color;
		}
	return;
}

int save_image(char *fn,double gasVelocityRadius,int gasVelocityStep,int kind)	// Saving averaged velocity and concentration fields to file
{
	int i,j,flag,coord;
	double curgc,curgv,curpc,curgs,curBright,curColor;
	printf("\nPreparing bitmap");
	flag=bmp_init();	
	if(flag)
		return flag;
//printf(" \ngas con [%f,%f], gas vel [%f,%f], pow con [%f,%f]\n\n",mingc,maxgc,mingv,maxgv,minpc,maxpc);fflush(stdout);
	if(makeGasConcentration&&(maxgc>=mingc))
	{
		int gasConcentrationColorR=(gasConcentrationColor>>16)&0xff;
		int gasConcentrationColorG=(gasConcentrationColor>>8)&0xff;
		int gasConcentrationColorB=gasConcentrationColor&0xff;
		printf("\n- gas concentration");
		for(i=0;i<pixelsX;i++)
			for(j=0;j<pixelsY;j++)
			{
				coord=pixelsY*i+j;
				curgc=con[coord];
				if(curgc<0)
				{
					bmp_r[coord]=(wallColor>>16)&0xff;
					bmp_g[coord]=(wallColor>>8)&0xff;
					bmp_b[coord]=wallColor&0xff;
				}
				else
				if(maxgc>mingc)
				{
					if(curgc<mingc)curgc=mingc;
					if(curgc>maxgc)curgc=maxgc;
					curBright=(curgc-mingc)/(maxgc-mingc);
					curBright=pow(curBright,1/gasConcentrationGammaCorrection);
					curBright=curBright>1?1:curBright;
					bmp_r[coord]=gasConcentrationColorR*curBright;
					bmp_g[coord]=gasConcentrationColorG*curBright;
					bmp_b[coord]=gasConcentrationColorB*curBright;
				}
			}
	}
	if(makePowderConcentration&&(maxpc>=minpc))
	{
		int powderConcentrationColorR=(powderConcentrationColor>>16)&0xff;
		int powderConcentrationColorG=(powderConcentrationColor>>8)&0xff;
		int powderConcentrationColorB=powderConcentrationColor&0xff;
		printf("\n- powder concentration");
		for(i=0;i<pixelsX;i++)
			for(j=0;j<pixelsY;j++)
			{
				coord=pixelsY*i+j;
				curpc=pcon[coord];
				if(curpc<0)
				{
					bmp_r[coord]=(wallColor>>16)&0xff;
					bmp_g[coord]=(wallColor>>8)&0xff;
					bmp_b[coord]=wallColor&0xff;
				}
				else
				if(maxpc>minpc)
				{
					if(curpc<minpc)curpc=minpc;
					if(curpc>maxpc)curpc=maxpc;
					curBright=(curpc-minpc)/(maxpc-minpc);
					curBright=pow(curBright,1/powderConcentrationGammaCorrection);
					curColor=powderConcentrationColorR*curBright+bmp_r[coord];
					bmp_r[coord]=curColor<256?curColor:255;
					curColor=powderConcentrationColorG*curBright+bmp_g[coord];
					bmp_g[coord]=curColor<256?curColor:255;
					curColor=powderConcentrationColorB*curBright+bmp_b[coord];
					bmp_b[coord]=curColor<256?curColor:255;
				}
			}
	}
	if(makeGasStreamlines&&(maxgs>mings))
	{
		int gasStreamlinesColorR=(gasStreamlinesColor>>16)&0xff;
		int gasStreamlinesColorG=(gasStreamlinesColor>>8)&0xff;
		int gasStreamlinesColorB=gasStreamlinesColor&0xff;
		printf("\n- gas streamlines");
		for(i=0;i<pixelsX;i++)
			for(j=0;j<pixelsY;j++)
			{
				coord=pixelsY*i+j;
				curgs=streamlines[coord];
				{
					if(curgs<mings)curgs=mings;
					if(curgs>maxgs)curgs=maxgs;
					curBright=(curgs-mings)/(maxgs-mings);
					curBright=pow(curBright,1/gasStreamlinesGammaCorrection);
					curColor=gasStreamlinesColorR*curBright+bmp_r[coord];
					bmp_r[coord]=curColor<256?curColor:255;
					curColor=gasStreamlinesColorG*curBright+bmp_g[coord];
					bmp_g[coord]=curColor<256?curColor:255;
					curColor=gasStreamlinesColorB*curBright+bmp_b[coord];
					bmp_b[coord]=curColor<256?curColor:255;
				}
			}
	}

	if(makeGasVelocity&&(maxgv>0))
	{
		double scale=arrowLengthScale*gasVelocityStep/maxgv;
		double mx1,my1,normgv;
//		int b=(int)(gasVelocityRadius*pixelsInMillimeter+1);
//		int i0=b+((pixelsX-2*b)%gasVelocityStep)/2;
//		int j0=b+((pixelsY-2*b)%gasVelocityStep)/2;
		printf("\n- gas velocity");
		for(i=pixelVelocityStartX;i<=pixelVelocityFinishX;i+=gasVelocityStep)
			for(j=pixelVelocityStartY;j<=pixelVelocityFinishY;j+=gasVelocityStep)
			{
				coord=pixelsY*i+j;
				mx1=mx[coord];
				my1=my[coord];
				if((mx1>-99)&&(my1>-99)&&((mx1!=0)||(my1!=0)))
				{
					curgv=sqrt(mx1*mx1+my1*my1);
					if(curgv>maxgv)
					{
						normgv=maxgv/curgv;
						mx1*=normgv;
						my1*=normgv;
					}
					else
					{
						normgv=pow(curgv/maxgv,1/gasVelocityMagnitudeCorrection)*maxgv/curgv;
						mx1*=normgv;
						my1*=normgv;
					}
					drawArrow(i,j,mx1*scale,my1*scale);
				}
			}
	}
	printf("\nSaving file");
	printf("\n- %s",fn);
	bmp_fin_color(fn);
	printf("\n");
	return 0;
}
