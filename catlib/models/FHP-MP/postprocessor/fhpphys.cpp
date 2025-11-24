// FHP physics
// Version 2.0
//

extern unsigned char *ca0, *cb0;
extern int rang(int);
extern double yVelocityCorrection;

double projection_x[]={0,0,0.86602540378443864676372317075294,0.86602540378443864676372317075294,
						0,-0.86602540378443864676372317075294,-0.86602540378443864676372317075294};
double projection_y[]={0,-1,-0.5,0.5,1,0.5,-0.5};

int int_projection_x[]={0,0,1,1,0,-1,-1};		// should be *0.86602540378443864676372317075294
int int_projection_y[]={0,-2,-1,1,2,1,-1};		// should be /2

double momentum_x_bool(long l)		// Gets x-projection of momentum O.L. method
{
	double m1,m2,m3;
	if(ca0[l+1]>ca0[l+4])
		m1=projection_x[1];
	if(ca0[l+1]<ca0[l+4])
		m1=projection_x[4];
	if(ca0[l+1]==ca0[l+4])
		m1=0;
	if(ca0[l+2]>ca0[l+5])
		m2=projection_x[2];
	if(ca0[l+2]<ca0[l+5])
		m2=projection_x[5];
	if(ca0[l+2]==ca0[l+5])
		m2=0;
	if(ca0[l+3]>ca0[l+6])
		m3=projection_x[3];
	if(ca0[l+3]<ca0[l+6])
		m3=projection_x[6];
	if(ca0[l+3]==ca0[l+6])
		m3=0;
	return m1+m2+m3;
}

double momentum_y_bool(long l)		// Gets y-projection of momentum O.L. method
{
	double m1,m2,m3;
	if(ca0[l+1]>ca0[l+4])
		m1=projection_y[1];
	if(ca0[l+1]<ca0[l+4])
		m1=projection_y[4];
	if(ca0[l+1]==ca0[l+4])
		m1=0;
	if(ca0[l+2]>ca0[l+5])
		m2=projection_y[2];
	if(ca0[l+2]<ca0[l+5])
		m2=projection_y[5];
	if(ca0[l+2]==ca0[l+5])
		m2=0;
	if(ca0[l+3]>ca0[l+6])
		m3=projection_y[3];
	if(ca0[l+3]<ca0[l+6])
		m3=projection_y[6];
	if(ca0[l+3]==ca0[l+6])
		m3=0;
	return m1+m2+m3;
}

double momentum_x(long l)		// Gets x-projection of momentum in a cell
{
	long i;
	double m=0;
	for(i=1;i<7;i++)
		m+=ca0[l+i]*projection_x[i];
	return m;
}

double momentum_y(long l)		// Gets y-projection of momentum in a cell
{
	long i;
	double m=0;
	for(i=1;i<7;i++)
		m+=ca0[l+i]*projection_y[i];
//	return m*0.9;				// 0.9 is an experimentally founded correction for the Y-axis
	return m*yVelocityCorrection;		// with preset correction
}

/*
int int_momentum_x(long l)		// Gets x-projection of momentum in a cell; should be *0.86602540378443864676372317075294
{
	int i,m=0;
	for(i=1;i<7;i++)
		m+=ca0[l+i]*int_projection_x[i];
	return m;
}

int int_momentum_y(long l)		// Gets y-projection of momentum in a cell; should be /2
{
	int i,m=0;
	for(i=1;i<7;i++)
		m+=ca0[l+i]*int_projection_y[i];
//	return m*0.9;				// 0.9 is an experimentally founded correction for the Y-axis
	return m;				// without correction
}
*/

int concentration(long l)	// Gets concentration of the particles in a cell
{
	long i,i2=l+7;
	int m=0;
	for(i=l;i<i2;i++)
		m+=ca0[i];
	return m;
}

int pconcentration(long l,int code)	// Gets concentration of the particles in a cell
{
	long i;
	int m=0;
	for(i=0;i<7;i++)
		m+=(rang(code&cb0[l+i]));
	return m;
}

int pmconcentration(long l)	// Gets concentration of the particles in a cell
{
	long i;
	int m=0;
	for(i=0;i<7;i++)
		m+=cb0[l+i];
	return m;
}
