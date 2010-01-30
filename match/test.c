/***************************************************************************
 *
 * Multitouch X driver
 * Copyright (C) 2008 Henrik Rydberg <rydberg@euromail.se>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 **************************************************************************/

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
	int index[DIM_FINGER], i;
	match_fingers(index, A, 4, 4);
	for (i = 0; i < 4; i++)
		printf("match[%d] = %d\n", i, index[i]);
}

static void test2()
{
	float A[]={
		0.000000,
		4534330.000000,
		22653552.000000,
		12252500.000000,
		685352.000000,
		4534330.000000,
		0.000000,
		9619317.000000,
		28409530.000000,
		6710170.000000,
		22653552.000000,
		9619317.000000,
		0.000000,
		47015292.000000,
		29788572.000000,
		2809040.000000,
		10428866.000000,
		38615920.000000,
		17732500.000000,
		719528.000000,
		12113945.000000,
		28196220.000000,
		46778656.000000,
		405.000000,
		14175493.000000,
	};
	int index[DIM_FINGER], i;
	match_fingers(index, A, 5, 5);
	for (i = 0; i < 5; i++)
		printf("match[%d] = %d\n", i, index[i]);
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
	printf("test1\n");
	test1();
	printf("test2\n");
	test2();
	printf("speed1\n");
	speed1();
	printf("done\n");
	return 0;
}
