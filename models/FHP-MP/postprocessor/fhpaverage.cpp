// FHP averager functions
// Version 2.0
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

extern bool makeGasConcentration;
extern bool makeGasVelocity;
extern bool makePowderConcentration;
extern bool makeGasStreamlines;
extern bool mixPowders;
extern bool smoothLines;
extern bool symmetricVicinity;
extern double metricX, metricY, cellsInMillimeter, pixelsInMillimeter;
extern double minGasConcentration,maxGasConcentration,maxGasVelocity,minPowderConcentration,maxPowderConcentration,minGasStreamlines,maxGasStreamlines;	// Min & max values for scaling
extern double gasStreamlinesWidth;
extern int gasStreamlinesPeriod;
extern double gasStreamlinesX1,gasStreamlinesY1,gasStreamlinesX2,gasStreamlinesY2;
extern int gasStreamlinesN;
extern double mingc,maxgc,maxgv,minpc,maxpc,mings,maxgs;
extern int pixelVelocityStartX,pixelVelocityFinishX,pixelVelocityStartY,pixelVelocityFinishY;
//extern double excludeMeanVelocityX,excludeMeanVelocityY;

extern unsigned char *ca0,*cb0;
extern int X_size, Y_size, X2_size, Y2_size;				// Size of CA in massive elements
extern int pixelsX, pixelsY;
extern long xy2l(int,int,int);			// Gets index number from coordinates
extern double d2(int,int,int,int);		// Square distanse between two points
extern double d2(int,int,double,double);		// Square distanse between two points
extern double d2(double,double,int,int);		// Square distanse between two points
extern double d2(double,double,double,double);		// Square distanse between two points
extern double *mx, *my, *con, *pcon, *streamlines;			// Averaged momentum and concentration
extern int rus_fp_format;				// Format of floating point data
extern double avx1, avy1, avx2, avy2;				// Boundaries of averaging area
extern double avCosinus,avSinus;

extern double rotx(double,double);
extern double roty(double,double);
extern double momentum_x(long);			// Gets x-projection of momentum in a cell
extern double momentum_y(long);			// Gets y-projection of momentum in a cell
extern double momentum_x_bool(long);			// Gets x-projection of momentum O.L. method
extern double momentum_y_bool(long);			// Gets y-projection of momentum O.L. method
extern int concentration(long);			// Gets concentration of the particles in a cell
extern int pconcentration(long,int);			// Gets concentration of the particles in a cell
extern int pmconcentration(long);			// Gets concentration of the particles in a cell
extern int rang(int);

extern double i2x(int);
extern double j2y(int,int);
extern void xy2ij(double,double,int *,int *);

//extern int x2i(double);
//extern int y2j(int,double);
//extern int y2j(double,double);

int allocate_av(void)					// Memory allocating for averaged massives
{
	int i,j;
	long l;
	l = X2_size * Y2_size * 8;
	ca0 = (unsigned char *) malloc(l * sizeof(unsigned char));
	if(ca0==NULL)
		return 1000;
	cb0 = (unsigned char *) malloc(l * sizeof(unsigned char));
	if(cb0==NULL)
		return 1000;
	for(i=0;i<l;i++)
		ca0[i]=cb0[i]=0;

	l = pixelsX * pixelsY;
	if(makeGasConcentration)
	{
		con = (double *) malloc(l * sizeof(double));
		if(con==NULL)
			return 413;
		for(i=0;i<pixelsX;i++)
			for(j=0;j<pixelsY;j++)
				con[i*pixelsY+j]=-1;
	}
	if(makeGasVelocity)
	{
		mx = (double *) malloc(l * sizeof(double));
		if(mx==NULL)
			return 411;
		my = (double *) malloc(l * sizeof(double));
		if(my==NULL)
			return 412;
		for(i=0;i<pixelsX;i++)
			for(j=0;j<pixelsY;j++)
				mx[i*pixelsY+j]=my[i*pixelsY+j]=0;
	}
	if(makeGasStreamlines)
	{
		streamlines = (double *) malloc(l * sizeof(double));
		if(streamlines==NULL)
			return 413;
		for(i=0;i<pixelsX;i++)
			for(j=0;j<pixelsY;j++)
				streamlines[i*pixelsY+j]=0;
	}
	if(makePowderConcentration)
	{
		pcon = (double *) malloc((mixPowders?1:8) * l * sizeof(double));
		if(pcon==NULL)
			return 413;
		for(i=0;i<pixelsX;i++)
			for(j=0;j<pixelsY;j++)
				pcon[i*pixelsY+j]=-1;
	}
	return 0;
}
double minValue(double *arr)
{
	int i,j;
	double m=-1;
	for(i=0;i<pixelsX;i++)
		for(j=0;j<pixelsY;j++)
			if(m<0)
			{
				if(arr[i*pixelsY+j]>=0)
					m=arr[i*pixelsY+j];
			}
			else
				if((arr[i*pixelsY+j]>0)&&(arr[i*pixelsY+j]<m))
					m=arr[i*pixelsY+j];
	return m;
}

double maxValue(double *arr)
{
	int i,j;
	double m=-1;
	for(i=0;i<pixelsX;i++)
		for(j=0;j<pixelsY;j++)
		{
			if(m<0)
			{
				if(arr[i*pixelsY+j]>=0)
					m=arr[i*pixelsY+j];
			}
			else
				if((arr[i*pixelsY+j]>0)&&(arr[i*pixelsY+j]>m))
					m=arr[i*pixelsY+j];
		}
	return m;
}

double maxVelocity(double *mx,double *my,double r,int step)
{
	int i,j,coord;
	double m=-1,mx1,my1,mxy;
//	int b=(int)(r*pixelsInMillimeter+1);
//	int i0=b+((pixelsX-2*b)%step)/2;
//	int j0=b+((pixelsY-2*b)%step)/2;
	for(i=pixelVelocityStartX;i<=pixelVelocityFinishX;i+=step)
		for(j=pixelVelocityStartY;j<=pixelVelocityFinishY;j+=step)
		{
			coord=i*pixelsY+j;
			mx1=mx[coord];
			my1=my[coord];
			if((mx1>-99)&&(my1>-99))
			{
				mxy=sqrt(mx1*mx1+my1*my1);
				if(mxy>m)
					m=mxy;
			}
		}
	return m;
}

double averageGasConcentration1(double x,double y,double r)
{
	int i,j,k,con0=0,n=0,l;
	int i0,j0,i1,j1,i2,j2;
	xy2ij(x,y,&i0,&j0);
	xy2ij(x-r,y-r,&i1,&j1);
	xy2ij(x+r,y+r,&i2,&j2);
	i1--;j1--;i2++;j2++;
//	int i0=x2i(x),j0=y2j(x,y);
//	int i1=x2i(x-r)-1,j1=y2j(x-r,y-r)-1;
//	int i2=x2i(x+r)+1,j2=y2j(x+r,y+r)+1;
//printf("\nStart with (%g,%g) R%g\n",x,y,r);fflush(stdout);
	if(i1<0)i1=0;
	if(j1<0)j1=0;
	if(i2>X_size)i2=X_size;
	if(j2>Y_size)j2=Y_size;
//printf("i1=%d,i0=%d,i2=%d, j1=%d,j0=%d,j2=%d\n",i1,i0,i2,j1,j0,j2);fflush(stdout);
	for(i=i1;i<i2;i++)
		for(j=j1;j<j2;j++)
			if(d2(i,j,x,y)<=(r*r))
			{
//printf(".");fflush(stdout);
				l=xy2l(i,j,7);
				if(!ca0[l])
				{
//printf(":");fflush(stdout);
					for(k=l-7;k<l;k++)
						con0+=ca0[k];
					n++;
				}
				else
					return -1;
			}
//printf("n=%d, con0=%d\n",n,con0);fflush(stdout);
	if(!n)
	{
		xy2ij(x,y,&i,&j);
//		i=x2i(x);
//		j=y2j(x,y);
		n=1;
		if(!ca0[xy2l(i,j,7)])
			con0=concentration(xy2l(i,j,0));
		else
			return -1;
	}
//printf("n=%d, con0=%d\n",n,con0);fflush(stdout);
	return (double)con0/n;
}

double averagePowderConcentration1(double x,double y,double r,int code)
{
	int i,j,mx0=0,my0=0,pcon0=0,n=0;
	int i0,j0,i1,j1,i2,j2;
	xy2ij(x,y,&i0,&j0);
	xy2ij(x-r,y-r,&i1,&j1);
	xy2ij(x+r,y+r,&i2,&j2);
	i1--;j1--;i2++;j2++;
//	int i0=x2i(x),j0=y2j(x,y);
//	int i1=x2i(x-r)-1,j1=y2j(x-r,y-r)-1;
//	int i2=x2i(x+r)+1,j2=y2j(x+r,y+r)+1;
	if(i1<0)i1=0;
	if(j1<0)j1=0;
	if(i2>X_size)i2=X_size;
	if(j2>Y_size)j2=Y_size;

	for(i=i1;i<i2;i++)
		for(j=j1;j<j2;j++)
			if(d2(i,j,x,y)<=(r*r))
			{
				if(!cb0[xy2l(i,j,7)])
				{
					if(mixPowders)
						pcon0+=pmconcentration(xy2l(i,j,0));
					else
						pcon0+=pconcentration(xy2l(i,j,0),1<<code);
					n++;
				}
				else
					return -1;
			}
	if(!n)
	{
		xy2ij(x,y,&i,&j);
//		i=x2i(x);
//		j=y2j(x,y);
		n=1;
		if(!cb0[xy2l(i,j,7)])
				if(mixPowders)
					pcon0=pmconcentration(xy2l(i,j,0));
				else
					pcon0=pconcentration(xy2l(i,j,0),1<<code);
		else
			return -1;
	}
	return (double)pcon0/n;
}

double averageGasVelocityX1(double x,double y,double r)
{
	int i,j,con0=0,n=0;
	double mx0=0;
	int i0,j0,i1,j1,i2,j2;
	xy2ij(x,y,&i0,&j0);
	xy2ij(x-r,y-r,&i1,&j1);
	xy2ij(x+r,y+r,&i2,&j2);
	i1--;j1--;i2++;j2++;
//	int i0=x2i(x),j0=y2j(x,y);
//	int i1=x2i(x-r)-1,j1=y2j(x-r,y-r)-1;
//	int i2=x2i(x+r)+1,j2=y2j(x+r,y+r)+1;
	double r2=r*r;
	if(i1<0)i1=0;
	if(j1<0)j1=0;
	if(i2>X_size)i2=X_size;
	if(j2>Y_size)j2=Y_size;

	for(i=i1;i<i2;i++)
		for(j=j1;j<j2;j++)
			if(d2(i,j,x,y)<=r2)
			{
				if(!ca0[xy2l(i,j,7)])
				{
					con0=concentration(xy2l(i,j,0));
					if(con0)
						mx0+=momentum_x(xy2l(i,j,0))/con0;
//						mx0+=momentum_x_bool(xy2l(i,j,0));
					n++;
				}
				else
					return -100;
			}
	if(!n)
	{
		xy2ij(x,y,&i,&j);
//		i=x2i(x);
//		j=y2j(x,y);
		if(!ca0[xy2l(i,j,7)])
		{
			con0=concentration(xy2l(i,j,0));
			if(con0)
				mx0=momentum_x(xy2l(i,j,0))/con0;
		}
		else
			return -100;
		n=1;
	}
	return (double)mx0/((double)n);
//	return (double)mx0/((double)con0*n);
}

double averageGasVelocityY1(double x,double y,double r)
{
	int i,j,con0=0,n=0;
	double my0=0;
	int i0,j0,i1,j1,i2,j2;
	xy2ij(x,y,&i0,&j0);
	xy2ij(x-r,y-r,&i1,&j1);
	xy2ij(x+r,y+r,&i2,&j2);
	i1--;j1--;i2++;j2++;
//	int i0=x2i(x),j0=y2j(x,y);
//	int i1=x2i(x-r)-1,j1=y2j(x-r,y-r)-1;
//	int i2=x2i(x+r)+1,j2=y2j(x+r,y+r)+1;
	double r2=r*r;
	if(i1<0)i1=0;
	if(j1<0)j1=0;
	if(i2>X_size)i2=X_size;
	if(j2>Y_size)j2=Y_size;

	for(i=i1;i<i2;i++)
		for(j=j1;j<j2;j++)
			if(d2(i,j,x,y)<=r2)
			{
				if(!ca0[xy2l(i,j,7)])
				{
					con0=concentration(xy2l(i,j,0));
					if(con0)
						my0+=momentum_y(xy2l(i,j,0))/con0;
//						my0+=momentum_y_bool(xy2l(i,j,0));
					n++;
				}
				else
					return -100;
			}
	if(!n)
	{
		xy2ij(x,y,&i,&j);
//		i=x2i(x);
//		j=y2j(x,y);
		if(!ca0[xy2l(i,j,7)])
		{
			con0=concentration(xy2l(i,j,0));
			if(con0)
				my0=momentum_y(xy2l(i,j,0))/con0;
		}
		else
			return -100;
		n=1;
	}
	return (double)my0/((double)n);
//	return (double)my0/((double)con0*n);
}

double averageGasVelocityX1w(double x,double y,double r)
{
	int i,j,con0=0,n=0;
	double mx0=0;
	int i0,j0,i1,j1,i2,j2;
	xy2ij(x,y,&i0,&j0);
	xy2ij(x-r,y-r,&i1,&j1);
	xy2ij(x+r,y+r,&i2,&j2);
	i1--;j1--;i2++;j2++;
//	int i0=x2i(x),j0=y2j(x,y);
//	int i1=x2i(x-r)-1,j1=y2j(x-r,y-r)-1;
//	int i2=x2i(x+r)+1,j2=y2j(x+r,y+r)+1;
	double r2=r*r;
	if(i1<0)i1=0;
	if(j1<0)j1=0;
	if(i2>X_size)i2=X_size;
	if(j2>Y_size)j2=Y_size;

	for(i=i1;i<i2;i++)
		for(j=j1;j<j2;j++)
			if(d2(i,j,x,y)<=r2)
			{
				if(!ca0[xy2l(i,j,7)])
				{
					con0=concentration(xy2l(i,j,0));
					if(con0)
						mx0+=momentum_x(xy2l(i,j,0))/con0;
//						mx0+=momentum_x_bool(xy2l(i,j,0));
					n++;
				}
			}
	xy2ij(x,y,&i,&j);
	if((n==0)||(ca0[xy2l(i,j,7)]))
//	if((n==0)||(ca0[xy2l(x2i(x),y2j(x,y),7)]))
//	if((con0*n==0)||(ca0[xy2l(x2i(x),y2j(x,y),7)]))
		return -100;
	return (double)mx0/((double)n);
//	return (double)mx0/((double)con0*n);
}

double averageGasVelocityY1w(double x,double y,double r)
{
	int i,j,con0=0,n=0;
	double my0=0;
	int i0,j0,i1,j1,i2,j2;
	xy2ij(x,y,&i0,&j0);
	xy2ij(x-r,y-r,&i1,&j1);
	xy2ij(x+r,y+r,&i2,&j2);
	i1--;j1--;i2++;j2++;
//	int i0=x2i(x),j0=y2j(x,y);
//	int i1=x2i(x-r)-1,j1=y2j(x-r,y-r)-1;
//	int i2=x2i(x+r)+1,j2=y2j(x+r,y+r)+1;
	double r2=r*r;
	if(i1<0)i1=0;
	if(j1<0)j1=0;
	if(i2>X_size)i2=X_size;
	if(j2>Y_size)j2=Y_size;

	for(i=i1;i<i2;i++)
		for(j=j1;j<j2;j++)
			if(d2(i,j,x,y)<=r2)
			{
				if(!ca0[xy2l(i,j,7)])
				{
					con0=concentration(xy2l(i,j,0));
					if(con0)
						my0+=momentum_y(xy2l(i,j,0))/con0;
//						my0+=momentum_y_bool(xy2l(i,j,0));
					n++;
				}
			}
	xy2ij(x,y,&i,&j);
	if((n==0)||(ca0[xy2l(i,j,7)]))
//	if((n==0)||(ca0[xy2l(x2i(x),y2j(x,y),7)]))
//	if((con0*n==0)||(ca0[xy2l(x2i(x),y2j(x,y),7)]))
		return -100;
	return (double)my0/((double)n);
//	return (double)my0/((double)con0*n);
}

int averageGasConcentration(double r)
{
	int i,j;
//printf("\nStart %dx%d pixels, r=%g, scale=%g pix/mm, scale=%g cells/mm\n",pixelsX,pixelsY,r,pixelsInMillimeter,cellsInMillimeter);fflush(stdout);
	for(i=0;i<pixelsX;i++)
//i=j=200;
	{
//printf("%3d pixels, %g mm, \t%d cells: ",i,avx1+(double)i/pixelsInMillimeter,x2i(avx1+(double)i/pixelsInMillimeter));fflush(stdout);
		for(j=0;j<pixelsY;j++)
		{
			double x=avx1+(double)i/pixelsInMillimeter,y=avy1+(double)j/pixelsInMillimeter;	// ROT
			con[i*pixelsY+j]=averageGasConcentration1(rotx(x,y),roty(x,y),r);		// ROT
//			con[i*pixelsY+j]=averageGasConcentration1(avx1+(double)i/pixelsInMillimeter,avy1+(double)j/pixelsInMillimeter,r);
//printf(" %g ",con[i*pixelsY+j]);fflush(stdout);
		}
//printf("\n");fflush(stdout);
	}
//printf("\n");fflush(stdout);
	if(minGasConcentration>=0)
	{
//printf("1");fflush(stdout);
		mingc=minGasConcentration;
	}
	else
	{
//printf("2");fflush(stdout);
		mingc=minValue(con);
	}
	if(maxGasConcentration>=0)
	{
//printf("3");fflush(stdout);
		maxgc=maxGasConcentration;
	}
	else
	{
//printf("4");fflush(stdout);
		maxgc=maxValue(con);
	}
//printf(" min=%g, max=%g ",mingc,maxgc);fflush(stdout);
	return 0;
}

int averagePowderConcentration(double r,int powderCode)
{
	int i,j,k;
	for(i=0;i<pixelsX;i++)
		for(j=0;j<pixelsY;j++)
		{
//printf(".");fflush(stdout);
			if(!mixPowders)
				for(k=0;k<8;k++)
					if(powderCode&k)
					{
						double x=avx1+(double)i/pixelsInMillimeter,y=avy1+(double)j/pixelsInMillimeter;	// ROT
						pcon[i*pixelsY+j+k*pixelsX*pixelsY]=averagePowderConcentration1(rotx(x,y),roty(x,y),r,powderCode&k);	// ROT
//						pcon[i*pixelsY+j+k*pixelsX*pixelsY]=
//						averagePowderConcentration1(avx1+(double)i/pixelsInMillimeter,avy1+(double)j/pixelsInMillimeter,r,powderCode&k);
					}
					else
						pcon[i*pixelsY+j+k*pixelsX*pixelsY]=0;
			else
			{
				double x=avx1+(double)i/pixelsInMillimeter,y=avy1+(double)j/pixelsInMillimeter;	// ROT
				pcon[i*pixelsY+j]=averagePowderConcentration1(rotx(x,y),roty(x,y),r,powderCode);	// ROT
//				pcon[i*pixelsY+j]=averagePowderConcentration1(avx1+(double)i/pixelsInMillimeter,avy1+(double)j/pixelsInMillimeter,r,powderCode);
			}
		}
	if(minPowderConcentration>=0)
		minpc=minPowderConcentration;
	else
		minpc=minValue(pcon);
	if(maxPowderConcentration>=0)
		maxpc=maxPowderConcentration;
	else
		maxpc=maxValue(pcon);
	return 0;
}

int averageGasVelocity(double r, int step)
{
	int i,j;
/*	int bx1=(int)(((r-avx1)<(step/pixelsInMillimeter)?step/pixelsInMillimeter:r-avx1)*pixelsInMillimeter+1);
	int bx2=(int)(((r-(metricX-avx2))<(step/pixelsInMillimeter)?step/pixelsInMillimeter:r-(metricX-avx2))*pixelsInMillimeter+1);
	int by1=(int)(((r-avy1)<(step/pixelsInMillimeter)?step/pixelsInMillimeter:r-avy1)*pixelsInMillimeter+1);
	int by2=(int)(((r-(metricY-avy2))<(step/pixelsInMillimeter)?step/pixelsInMillimeter:r-(metricY-avy2))*pixelsInMillimeter+1);
	pixelVelocityStartX=bx1+((pixelsX-bx1-bx2)%step)/2;
	pixelVelocityStartY=by1+((pixelsY-by1-by2)%step)/2;
	pixelVelocityFinishX=pixelsX-bx2;
	pixelVelocityFinishY=pixelsY-by2;
*/
	pixelVelocityStartX=(pixelsX%step)/2;
	pixelVelocityStartY=(pixelsY%step)/2;
	pixelVelocityFinishX=pixelsX-1;
	pixelVelocityFinishY=pixelsY-1;
	for(i=pixelVelocityStartX;i<=pixelVelocityFinishX;i+=step)
		for(j=pixelVelocityStartY;j<=pixelVelocityFinishY;j+=step)
		{
			if(symmetricVicinity)		// dispose a center of average area to a center of nearest cell
			{
				double xCenteredToCell,yCenteredToCell;
				int iCenteredToCell,jCenteredToCell;
				double x=rotx(avx1+(double)i/pixelsInMillimeter,avy1+(double)j/pixelsInMillimeter);
				double y=roty(avx1+(double)i/pixelsInMillimeter,avy1+(double)j/pixelsInMillimeter);
				xy2ij(x,y,&iCenteredToCell,&jCenteredToCell);
				xCenteredToCell=i2x(iCenteredToCell);
				yCenteredToCell=j2y(iCenteredToCell,jCenteredToCell);
				mx[i*pixelsY+j]=averageGasVelocityX1(xCenteredToCell,yCenteredToCell,r);
				my[i*pixelsY+j]=averageGasVelocityY1(xCenteredToCell,yCenteredToCell,r);
			}
			else
			{
				double x=rotx(avx1+(double)i/pixelsInMillimeter,avy1+(double)j/pixelsInMillimeter);
				double y=roty(avx1+(double)i/pixelsInMillimeter,avy1+(double)j/pixelsInMillimeter);
				double mx0=averageGasVelocityX1(x,y,r);
				double my0=averageGasVelocityY1(x,y,r);
				mx[i*pixelsY+j]=mx0;
				my[i*pixelsY+j]=my0;
//				mx[i*pixelsY+j]=mx0*avSinus+my0*avCosinus;
//				my[i*pixelsY+j]=mx0*avCosinus-my0*avSinus;
			}
		}
	if(maxGasVelocity>=0)
		maxgv=maxGasVelocity;
	else
		maxgv=maxVelocity(mx,my,r,step);
	return 0;
}
/*
int averageGasVelocityZeroAngle(double r, int step)
{
	int i,j;
//	int b=(int)(r*pixelsInMillimeter+1);
//	pixelVelocityStartX=b+((pixelsX-2*b)%step)/2;
//	pixelVelocityStartY=b+((pixelsY-2*b)%step)/2;
//	pixelVelocityFinishX=pixelsX-b;
//	pixelVelocityFinishY=pixelsY-b;
	int bx1=(int)(((r-avx1)<(step/pixelsInMillimeter)?step/pixelsInMillimeter:r-avx1)*pixelsInMillimeter+1);
	int bx2=(int)(((r-(metricX-avx2))<(step/pixelsInMillimeter)?step/pixelsInMillimeter:r-(metricX-avx2))*pixelsInMillimeter+1);
	int by1=(int)(((r-avy1)<(step/pixelsInMillimeter)?step/pixelsInMillimeter:r-avy1)*pixelsInMillimeter+1);
	int by2=(int)(((r-(metricY-avy2))<(step/pixelsInMillimeter)?step/pixelsInMillimeter:r-(metricY-avy2))*pixelsInMillimeter+1);
	pixelVelocityStartX=bx1+((pixelsX-bx1-bx2)%step)/2;
	pixelVelocityStartY=by1+((pixelsY-by1-by2)%step)/2;
	pixelVelocityFinishX=pixelsX-bx2;
	pixelVelocityFinishY=pixelsY-by2;
//printf("\nBorders bx1=%d, bx2=%d, by1=%d, by2=%d, x=%d-%d pix, y=%d-%d pix\n",
//bx1,bx2,by1,by2,pixelVelocityStartX,pixelVelocityFinishX,pixelVelocityStartY,pixelVelocityFinishY);
	for(i=pixelVelocityStartX;i<=pixelVelocityFinishX;i+=step)
		for(j=pixelVelocityStartY;j<=pixelVelocityFinishY;j+=step)
		{
			if(symmetricVicinity)		// dispose a center of average area to a center of nearest cell
			{
				double xCenteredToCell,yCenteredToCell;
				int iCenteredToCell,jCenteredToCell;
				xy2ij(avx1+(double)i/pixelsInMillimeter,avy1+(double)j/pixelsInMillimeter,&iCenteredToCell,&jCenteredToCell);
//				iCenteredToCell=x2i(avx1+(double)i/pixelsInMillimeter);
//				jCenteredToCell=y2j(avx1+(double)i/pixelsInMillimeter,avy1+(double)j/pixelsInMillimeter);
				xCenteredToCell=i2x(iCenteredToCell);
				yCenteredToCell=j2y(iCenteredToCell,jCenteredToCell);
				mx[i*pixelsY+j]=averageGasVelocityX1(xCenteredToCell,yCenteredToCell,r);
				my[i*pixelsY+j]=averageGasVelocityY1(xCenteredToCell,yCenteredToCell,r);
			}
			else
			{
				mx[i*pixelsY+j]=averageGasVelocityX1(avx1+(double)i/pixelsInMillimeter,avy1+(double)j/pixelsInMillimeter,r);
				my[i*pixelsY+j]=averageGasVelocityY1(avx1+(double)i/pixelsInMillimeter,avy1+(double)j/pixelsInMillimeter,r);
			}
		}
	if(maxGasVelocity>=0)
		maxgv=maxGasVelocity;
	else
		maxgv=maxVelocity(mx,my,r,step);
	return 0;
}
*/

void calculateMeanVelocity(double r, int step, double *meanVelocityX, double *meanVelocityY)
{
	double sumX=0,sumY=0;
	long long nodesNumberX=0,nodesNumberY=0;
	int i,j;
	pixelVelocityStartX=(pixelsX%step)/2;
	pixelVelocityStartY=(pixelsY%step)/2;
	pixelVelocityFinishX=pixelsX-1;
	pixelVelocityFinishY=pixelsY-1;
	for(i=pixelVelocityStartX;i<=pixelVelocityFinishX;i+=step)
		for(j=pixelVelocityStartY;j<=pixelVelocityFinishY;j+=step)
		{
			if(my[i*pixelsY+j]>-100)
			{
				sumX+=mx[i*pixelsY+j];
				nodesNumberX++;
			}
			if(my[i*pixelsY+j]>-100)
			{
				sumY+=my[i*pixelsY+j];
				nodesNumberY++;
			}
		}
	(*meanVelocityX)=sumX/nodesNumberX;
	(*meanVelocityY)=sumY/nodesNumberY;
}

void excludeMeanVelocity(double r, int step, double meanVelocityX, double meanVelocityY)
{
	int i,j;
	pixelVelocityStartX=(pixelsX%step)/2;
	pixelVelocityStartY=(pixelsY%step)/2;
	pixelVelocityFinishX=pixelsX-1;
	pixelVelocityFinishY=pixelsY-1;
	for(i=pixelVelocityStartX;i<=pixelVelocityFinishX;i+=step)
		for(j=pixelVelocityStartY;j<=pixelVelocityFinishY;j+=step)
		{
			mx[i*pixelsY+j]-=meanVelocityX;
			my[i*pixelsY+j]-=meanVelocityY;
		}
	if(maxGasVelocity>=0)
		maxgv=maxGasVelocity;
	else
		maxgv=maxVelocity(mx,my,r,step);
}

void putSpot(double *arr,double x0,double y0,double r)
{
	double x,y,dx,dy,dx2,dy2,r2=r*r;
	int i,j,k,l,pixelColor;
	int i1=(int)(x0-r-1);
	if(i1<0)i1=0;
	int j1=(int)(y0-r-1);
	if(j1<0)j1=0;
	int i2=(int)(x0+r+2);
	if(i2>pixelsX)i2=pixelsX;
	int j2=(int)(y0+r+2);
	if(j2>pixelsY)j2=pixelsY;
	for(i=i1;i<i2;i++)
		for(j=j1;j<j2;j++)
		{
			pixelColor=0;
			k=i+1;
			l=j+1;
			for(x=i;x<k;x+=0.0625)
			{
				dx=x-x0;
				dx2=dx*dx;
				for(y=j;y<l;y+=0.0625)
				{
					dy=y-y0;
					dy2=dy*dy;
					pixelColor+=((dx2+dy2)<=r2);
				}
			}
			if(!smoothLines)pixelColor=pixelColor>127?255:0;
			arr[pixelsY*i+j]+=pixelColor*0.003921568627451;
		}
}

int trackGasStreamline(double x0, double y0, double r, double step,int maxStreamlineIterations)
{
	int i,j;
	double x=x0,y=y0,dx,dy,vx,vy,dv,w05=gasStreamlinesWidth*0.5;
//printf("Streamline (%g,%g), step %g mm\n",x0,y0,gasStreamlinesX2,gasStreamlinesY2,step);
	for(i=0;i<maxStreamlineIterations;i++)
	{
		if(!(i%gasStreamlinesPeriod))
			putSpot(streamlines,(x-avx1)*pixelsInMillimeter,(y-avy1)*pixelsInMillimeter,w05);
		vx=averageGasVelocityX1w(x,y,r);
		vy=averageGasVelocityY1w(x,y,r);
		if((vx<-99)||(vy<-99))
			break;
		dv=step/sqrt(vx*vx+vy*vy);
		dx=vx*dv;
		dy=vy*dv;
		x+=dx;
		y+=dy;
		if((x<avx1)||(y<avy1)||(x>avx2)||(y>avy2))
			break;
	}
	x=x0,y=y0;
	for(j=0;j<maxStreamlineIterations;j++)
	{
		if(!(j%gasStreamlinesPeriod))
			putSpot(streamlines,(x-avx1)*pixelsInMillimeter,(y-avy1)*pixelsInMillimeter,w05);
		vx=averageGasVelocityX1w(x,y,r);
		vy=averageGasVelocityY1w(x,y,r);
		if((vx<-99)||(vy<-99))
			break;
		dv=step/sqrt(vx*vx+vy*vy);
		dx=vx*dv;
		dy=vy*dv;
		x-=dx;
		y-=dy;
		if((x<avx1)||(y<avy1)||(x>avx2)||(y>avy2))
			break;
	}
	if(minGasStreamlines>=0)
		mings=minGasStreamlines;
	else
		mings=minValue(streamlines);
	if(maxGasStreamlines>=0)
		maxgs=maxGasStreamlines;
	else
		maxgs=maxValue(streamlines);
	return i+j;
}

int disposeGasStreamlines(double gasVelocityRadius, double gasStreamlineStep,int maxStreamlineIterations, int gasStreamlinesInterval)
{
	double x,y,vx,vy,vnorm,curvx,curvy,minvx=100,minvy=100,x0=0,y0=0,mip=1/pixelsInMillimeter,border=2/cellsInMillimeter;
	double step=5*mip;
	int numX,numY;
	for(x=avx1+gasVelocityRadius+border;x<avx2-gasVelocityRadius-border;x+=step)
	{
		numY=1;
		curvy=0;
		for(y=avy1+gasVelocityRadius+border;y<avy2-gasVelocityRadius-border;y+=step)
		{
			vx=averageGasVelocityX1(x,y,gasVelocityRadius);
			vy=averageGasVelocityY1(x,y,gasVelocityRadius);
			if((vx<-99)||(vy<-99)){curvy=101*numY;break;}
			if(vx!=0){numY++;vnorm=vy/vx;curvy+=vnorm*vnorm;}
		}
		curvy/=numY;
		if(curvy<minvy)
		{
			minvy=curvy;
			x0=x;
		}
	}
	for(y=avy1+gasVelocityRadius+border;y<avy2-gasVelocityRadius-border;y+=step)
	{
		numX=1;
		curvx=0;
		for(x=avx1+gasVelocityRadius+border;x<avx2-gasVelocityRadius-border;x+=step)
		{
			vx=averageGasVelocityX1(x,y,gasVelocityRadius);
			vy=averageGasVelocityY1(x,y,gasVelocityRadius);
			if((vx<-99)||(vy<-99)){curvy=101*numX;break;}
			if(vy!=0){numX++;vnorm=vx/vy;curvx+=vnorm*vnorm;}
		}
		curvx/=numX;
		if(curvx<minvx)
		{
			minvx=curvx;
			y0=y;
		}
	}
//printf(" minvx=%g (with y0=%g), minvy=%g (with x0=%g), ",minvx,y0,minvy,x0);fflush(stdout);
	if((minvx>99)&&(minvy>99))
		return 1;
	step=gasStreamlinesInterval*mip;
//printf(" step=%g ",step);fflush(stdout);
	if(minvx<minvy)
		for(x=avx1+mip*(pixelsX%gasStreamlinesInterval)*0.5;x<avx2;x+=step)
			trackGasStreamline(x,y0,gasVelocityRadius,gasStreamlineStep,maxStreamlineIterations);
	else
		for(y=avy1+mip*(pixelsY%gasStreamlinesInterval)*0.5;y<avy2;y+=step)
			trackGasStreamline(x0,y,gasVelocityRadius,gasStreamlineStep,maxStreamlineIterations);
	return 0;
}

