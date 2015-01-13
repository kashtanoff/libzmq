// Test.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include <stdio.h>
#include <iostream>
//#include <unistd.h>
#include <time.h>
using namespace std;

int sign = -1;
int ss[2] = {1, -1};
int type = 0;

double geta(double a, double b)
{
	return type? a-b: b-a;
}
double getb(double a, double b)
{
	return(ss[type]*a - ss[type]*b);
}
class B;
class A
{
	B*	b;
};
class B
{
	A*	a;
};
int _tmain(int argc, _TCHAR* argv[])
{
	cout << "hello world" << endl;
	double a = 1.234;
	double b = 1.345;
	double* c = 0;
	type = 1;
	int i;
	long t1 = clock();
	for (i=0; i<1000000000; i++)
	{
		type = 1-type;
		a = geta(a,b);
	}
	long t2 = clock();
	cout << "time: " << t2-t1 << endl;
	cout << "res: " << a << endl;

	t1 = clock();
	for (i=0; i<1000000000; i++)
	{
		type = 1-type;
		a = getb(a,b);
	}
	t2 = clock();
	cout << "time: " << t2-t1 << endl;
	cout << "res: " << a << endl;
	cout << "ptr: " << c << endl;

	int s;
	cin >> s;
	return 0;
}




