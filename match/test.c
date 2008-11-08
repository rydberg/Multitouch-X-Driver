#include <stdio.h>
#include <time.h>
#include "match.c"

#define ITS 1000000

static void test1()
{
	float A[] = {
		1013.000000,
		3030660.000000,
		3559354.000000,
		12505925.000000,
		19008450.000000,
		6946421.000000,
		6118613.000000,
		698020.000000,
		3021800.000000,
		1017.000000,
		37573.000000,
		3242018.000000,
		8152794.000000,
		1266053.000000,
		942941.000000,
		462820.000000,
	};
	int index[DIM_FINGER];
	match_fingers(index, A, 4, 4);
}

static void speed1()
{
	// column-by-column matrix
	float A[DIM2_FINGER];
	float x1[DIM_FINGER]={1,5,2,3,4,5,6,7,8};
	float y1[DIM_FINGER]={1,5,2,3,4,5.1,6,7,8};
	float x2[DIM_FINGER]={1.1,3,2,4,5,6,7,8};
	float y2[DIM_FINGER]={1,3,2,4,5,6,7,8};
	int index[DIM_FINGER];
	int n1 = 4;
	int n2 = 7;

	int i, j;
	
	for (i = 0; i < n1; i++) {
		for (j = 0; j < n2; j++) {
			A[i + n1 * j] =
				(x1[i] - x2[j]) * (x1[i] - x2[j]) +
				(y1[i] - y2[j]) * (y1[i] - y2[j]);
		}
	}

	clock_t t1 = clock();
	for (i = 0; i < ITS; i++)
		match_fingers(index, A, n1, n2);
	clock_t t2 = clock();

	printf("%lf matches per second\n",
	       ITS * ((float)CLOCKS_PER_SEC / (t2 - t1)));

	for (i = 0; i < n1; i++)
		printf("match[%d] = %d\n", i, index[i]);
	
}

int main(int argc,char* argv[])
{
	test1();
	speed1();
	return 0;
}
