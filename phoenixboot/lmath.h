extern "C" int printk(const char *fmt, ...);
#define dprintf printk
#define CUTOFF 10

#define BIG_360_MULTIPLE (360*40)
#define SIN(angle)     sine_table[(angle)%360000]
#define COS(angle)     sine_table[(angle + 90000)%360000]

#include "bigsin.h"

float inline sin(float d)
{
	if (d < 0)
		return -(SIN(-int(d*100)));
	else
		return SIN(int(d*100));
}

float inline cos(float d)
{
	if (d < 0)
		return -(COS(-int(d*100)));
	else
		return COS(int(d*100));
}


/*



float inline cos(float degrees)
{
	//dprintf("c");
	float pi = 3.1415926535897932384626433832795f;
	float radians = pi*degrees/180;
	int max_iterations = CUTOFF;
	int n = 0;
	int count;
	float rcos = 0;
	int neg1;
	float x2n;
	int factorial;
	while(n < max_iterations)
	{
		//dprintf("cl");

		count = 0;
		neg1 = 1;
		while(count < n)
		{
			neg1 *= -1;
			count++;
		}
		count = 0;
		x2n = 1;
		while(count < 2*n)
		{
			x2n *= radians;
			count++;
		}
		count = 2*n;
		factorial = 1;
		while(count > 0)
		{
			factorial *= count;
			count--;
		}
		rcos = rcos + neg1 * (x2n/factorial);
		n++;
	}
	//dprintf("ec");
	return rcos;
}


float inline sin(float degrees)
{
	//dprintf("s");
	float pi = 3.1415926535897932384626433832795f;
	float radians = pi*degrees/180;
	int max_iterations = CUTOFF;
	int n = 0;
	int count;
	float rsin = 0;
	int neg1;
	float x2nplus1;
	int factorial;
	while(n < max_iterations)
	{
		//dprintf("sl");
		count = 0;
		neg1 = 1;
		while(count < n)
		{
			neg1 *= -1;
			count++;
		}
		count = 0;
		x2nplus1=1;
		while(count < 2*n+1)
		{
			x2nplus1 *= radians;
			count++;
		}
		count = 2*n+1;
		factorial = 1;
		while(count>0)
		{
			factorial *= count;
			count--;
		}
		rsin = rsin + neg1 * (x2nplus1/factorial);
		n++;
	}
	//dprintf("es");
	return rsin;
}
		
		 


float xsqrt(float square)
{
	if(square==0)
		return 0;
	float root = 1;
	int max_iteration = CUTOFF;
	int count = 0;
	while(count < max_iteration)
	{
		root = (root/2)+(square/(2*root));
		count++;
	}
	return root;
}


int inline fact(int n)
{
	int rfact=1;
	int count = n;
	while(count > 0)
	{
		rfact *= count;
		count--;
	}
	return rfact;
}

float inline exp(float x, int n)
{
	int count = 0;
	float rexp = 1;
	while(count < n)
	{
		rexp *= x;
		count++;
	}
	return rexp;
}
*/

float inline fabs(float i)
{
	if(i<0)
		return -i;
	return i;
}


double inline sqrt(double r) {
double x,y;
double tempf;
unsigned long *tfptr = ((unsigned long *)&tempf)+1;

    tempf = r;
    *tfptr=(0xbfcd4600-*tfptr)>>1;
    x=tempf;
    y=r*0.5; 
    x*=1.5-x*x*y; 
    x*=1.5-x*x*y;
    x*=1.5-x*x*y; 
    x*=1.5-x*x*y; 
    return x*r;
}


