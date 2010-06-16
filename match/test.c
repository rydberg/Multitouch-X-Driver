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

#include <match.h>
#include <xbypass.h>
#include <stdio.h>
#include <time.h>

#define ITS 1000000

static void test1()
{
	int A[] = {
		1013,
		3030660,
		3559354,
		12505925,
		19008450,
		6946421,
		6118613,
		698020,
		3021800,
		1017,
		37573,
		3242018,
		8152794,
		1266053,
		942941,
		462820,
	};
	int index[DIM_FINGER], i;
	match_fingers(index, A, 4, 4);
	for (i = 0; i < 4; i++)
		printf("match[%d] = %d\n", i, index[i]);
}

static void test2()
{
	int A[] = {
		0,
		4534330,
		22653552,
		12252500,
		685352,
		4534330,
		0,
		9619317,
		28409530,
		6710170,
		22653552,
		9619317,
		0,
		47015292,
		29788572,
		2809040,
		10428866,
		38615920,
		17732500,
		719528,
		12113945,
		28196220,
		46778656,
		405,
		14175493,
	};
	int index[DIM_FINGER], i;
	match_fingers(index, A, 5, 5);
	for (i = 0; i < 5; i++)
		printf("match[%d] = %d\n", i, index[i]);
}

static void speed1()
{
	/* column-by-column matrix */
	int A[DIM2_FINGER];
	int x1[DIM_FINGER] = { 1, 5, 2, 3, 4, 5, 6, 7, 8 };
	int y1[DIM_FINGER] = { 1, 5, 2, 3, 4, 6, 6, 7, 8 };
	int x2[DIM_FINGER] = { 1.1, 3, 2, 4, 5, 6, 7, 8 };
	int y2[DIM_FINGER] = { 1, 3, 2, 4, 5, 6, 7, 8 };
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

int main(int argc, char *argv[])
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
