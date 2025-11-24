// FHP coordinate transmission
// Version 2.0
//

int neigb_x[8]={0,0,1,1,0,-1,-1,0};
int neigb_y[2][8]={{0,-1,-1,0,1,0,-1,0},{0,-1,0,1,1,1,0,0}};

extern int X_size, Y_size, X2_size, Y2_size;
extern double cellsInMillimeter;
extern double avxCenter,avyCenter,avCosinus,avSinus;

double rotx(double x,double y)			// Gets x-coord after rotation relative to center of the av.area by avAngle
{
	return avxCenter+(x-avxCenter)*avCosinus-(y-avyCenter)*avSinus;
}

double roty(double x,double y)			// Gets y-coord after rotation relative to center of the av.area by avAngle
{
	return avyCenter+(x-avxCenter)*avSinus+(y-avyCenter)*avCosinus;
}

long xy2l(int x, int y, int k)			// Gets index number from coordinates
{
	return ((x+1)*Y2_size+y+1)*8+k;
}

int l2x(long l)							// Gets x-coordinate from index number
					// UNDER DEBUGGING
{
	return l/8/Y2_size-1;
}

int l2y(long l)							// Gets y-coordinate from index number
					// UNDER DEBUGGING
{
	return l%(Y2_size*8)-1;
}

long xyn2l(int x, int y, int n, int k)	// Gets neighbor's index number from coordinates
{
	return xy2l(x+neigb_x[n],y+neigb_y[x%2][n],k);
}

double d2_old(int x1,int y1,int x2,int y2)	// Square distanse between two points
{
	int a=x1-x2;
	double b,c=0;						// both x1 and x2 odd or even
	if((x1&1)&&!(x2&1))					// x1 - odd, x2 - even
		c=-0.5;
	if(!(x1&1)&&(x2&1))					// x1 - even, x2 - odd
		c=0.5;
	b=(y2-y1+c);

//	if((x1>=10)&&(x2>=10)&&(y1>=10)&&(y2>=10)&&(x1<=11)&&(x2<=11)&&(y1<=11)&&(y2<=11))
//		printf("d((%d,%d)-(%d,%d))=%f;   a,b,c=%2d,%9f,%9f\n",x1-10,y1-10,x2-10,y2-10,sqrt(a*a+1.333333333333*b*b),a,b,c);

	return a*a+1.333333333333*b*b;
}

double d2(int x1,int y1,int x2,int y2)	// Square distanse between two cells
{
	double phys_x1,phys_y1,phys_x2,phys_y2;
	phys_x1=(double)x1*0.86602540378443864676372317075294/cellsInMillimeter;
	phys_y1=((double)y1+0.5*(double)(x1&1))/cellsInMillimeter;
	phys_x2=(double)x2*0.86602540378443864676372317075294/cellsInMillimeter;
	phys_y2=((double)y2+0.5*(double)(x2&1))/cellsInMillimeter;
	return (phys_x1-phys_x2)*(phys_x1-phys_x2)+(phys_y1-phys_y2)*(phys_y1-phys_y2);
}

double d2(int x1,int y1,double phys_x2,double phys_y2)	// Square distanse between two cells
{
	double phys_x1,phys_y1;
	phys_x1=(double)x1*0.86602540378443864676372317075294/cellsInMillimeter;
	phys_y1=((double)y1+0.5*(double)(x1&1))/cellsInMillimeter;
	return (phys_x1-phys_x2)*(phys_x1-phys_x2)+(phys_y1-phys_y2)*(phys_y1-phys_y2);
}

double d2(double phys_x1,double phys_y1,int x2,int y2)	// Square distanse between two cells
{
	double phys_x2,phys_y2;
	phys_x2=(double)x2*0.86602540378443864676372317075294/cellsInMillimeter;
	phys_y2=((double)y2+0.5*(double)(x2&1))/cellsInMillimeter;
	return (phys_x1-phys_x2)*(phys_x1-phys_x2)+(phys_y1-phys_y2)*(phys_y1-phys_y2);
}

double d2(double phys_x1,double phys_y1,double phys_x2,double phys_y2)	// Square distanse between two cells
{
	return (phys_x1-phys_x2)*(phys_x1-phys_x2)+(phys_y1-phys_y2)*(phys_y1-phys_y2);
}

inline double cross_product(double x1,double y1,double x2,double y2,double x,double y)
{
	return (x - x2) * (y2 - y1) - (y - y2) * (x2 - x1);
}

bool point_in_triangle(double x1,double y1,double x2,double y2,double x3,double y3,double x,double y)
{
	bool cp1 = cross_product(x1, y1, x2, y2, x, y) < 0.0;
	bool cp2 = cross_product(x2, y2, x3, y3, x, y) < 0.0;
	bool cp3 = cross_product(x3, y3, x1, y1, x, y) < 0.0;
	return cp1 == cp2 && cp1 == cp3;
}

bool point_in_triangle1(double x1,double y1,double x2,double y2,double x3,double y3,double x,double y)
{
	bool cp1 = (x - x2) * (y2 - y1) - (y - y2) * (x2 - x1) < 0.0;
	bool cp2 = (x - x3) * (y3 - y2) - (y - y3) * (x3 - x2) < 0.0;
	bool cp3 = (x - x1) * (y1 - y3) - (y - y1) * (x1 - x3) < 0.0;
	return cp1 == cp2 && cp1 == cp3;
}

double i2x(int i)
{
	return i*0.86602540378443864676372317075294/cellsInMillimeter;
}

double j2y(int i,int j)
{
	return (j+0.5*(i&1))/cellsInMillimeter;
}

void xy2ij(double x,double y,int *i,int *j)
{
    const double sqrt3=1.732050807568877293527446341505872366942805253810380628055;
    x*=cellsInMillimeter;
    y*=cellsInMillimeter;
    int i0=2*(int)(x/sqrt3),j0=(int)y;
    double dx=x-i0*sqrt3*0.5,dy=y-j0;
    if(dy<(3-sqrt3*dx) && dy<(sqrt3*dx) && dy>=(1-sqrt3*dx) && dy>=(sqrt3*dx-2)){*i=(i0+1);*j=j0;return;}
    if(dx>=0.5*sqrt3)i0+=2;
    if(dy>=0.5)j0++;
    *i=i0;*j=j0;return;
}
/*
int x2i(double x)
{
	return (int)(x*1.1547005383792515290182975610039*cellsInMillimeter+0.5);
}

int y2j(int i,double y)
{
	return (int)(y*cellsInMillimeter+0.5*!(i&1));
}

int y2j(double x,double y)
{
	int i=x2i(x);
	return (int)(y*cellsInMillimeter+0.5*!(i&1));
}
*/
