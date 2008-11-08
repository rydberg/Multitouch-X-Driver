#include "match.h"
#include <string.h>
#include <stdio.h>

/**
 * MATLAB implementation of the hungarian algorithm (2008)
 *
 * modified by Henrik Rydberg (2008)
 */

typedef unsigned short col_t[1];
typedef unsigned short mat_t[DIM_FINGER];

#define GET1(m, x) ((m[0]>>(x))&1U)
#define SET1(m, x) (m[0]|=(1U<<(x)))
#define CLEAR1(m, x) (m[0]&=~(1U<<(x)))

#define GET2(m, row, col) ((m[col]>>(row))&1U)
#define SET2(m, row, col) (m[col]|=(1U<<(row)))
#define CLEAR2(m, row, col) (m[col]&=~(1U<<(row)))

/********************************************************/

static void buildixvector(int *ix, mat_t mstar, int nrows, int ncols)
{
	int row, col;
	for (row = 0; row < nrows; row++) {
		for (col = 0; col < ncols; col++) {
			if (GET2(mstar, row, col)) {
				ix[row] = col;
				break;
			}
		}
	}
}


/********************************************************/

static void step2a(int *ix, float *mdist, mat_t mstar, mat_t nmstar, mat_t mprime, col_t ccol, col_t crow, int nrows, int ncols, int dmin);
static void step2b(int *ix, float *mdist, mat_t mstar, mat_t nmstar, mat_t mprime, col_t ccol, col_t crow, int nrows, int ncols, int dmin);
static void step3(int *ix, float *mdist, mat_t mstar, mat_t nmstar, mat_t mprime, col_t ccol, col_t crow, int nrows, int ncols, int dmin);
static void step4(int *ix, float *mdist, mat_t mstar, mat_t nmstar, mat_t mprime, col_t ccol, col_t crow, int nrows, int ncols, int dmin, int row, int col);
static void step5(int *ix, float *mdist, mat_t mstar, mat_t nmstar, mat_t mprime, col_t ccol, col_t crow, int nrows, int ncols, int dmin);

static void ixoptimal(int *ix, float *mdist, int nrows, int ncols)
{
	float *mdistTemp, *mdistEnd, *columnEnd, value, minValue;
	int dmin, row, col;
	col_t ccol, crow;
	mat_t mstar, mprime, nmstar;

	memset(ccol, 0, sizeof(col_t));
	memset(crow, 0, sizeof(col_t));
	memset(mstar, 0, sizeof(mat_t));
	memset(mprime, 0, sizeof(mat_t));
	memset(nmstar, 0, sizeof(mat_t));

	/* initialization */
	for(row=0; row<nrows; row++)
		ix[row] = -1;
	
	mdistEnd = mdist + nrows * ncols;

	/* preliminary steps */
	if(nrows <= ncols) {
		dmin = nrows;
		
		for(row=0; row<nrows; row++) {
			/* find the smallest element in the row */
			mdistTemp = mdist + row;
			minValue = *mdistTemp;
			mdistTemp += nrows;			
			while(mdistTemp < mdistEnd) {
				value = *mdistTemp;
				if(value < minValue)
					minValue = value;
				mdistTemp += nrows;
			}
			
			/* subtract the smallest element from each element of the row */
			mdistTemp = mdist + row;
			while(mdistTemp < mdistEnd) {
				*mdistTemp -= minValue;
				mdistTemp += nrows;
			}
		}
		
		/* Steps 1 and 2a */
		for(row=0; row<nrows; row++)
			for(col=0; col<ncols; col++)
				if(mdist[row + nrows*col] == 0)
					if(!GET1(ccol, col)) {
						SET2(mstar, row, col);
						SET1(ccol, col);
						break;
					}
	} else {
		dmin = ncols;
		
		for(col=0; col<ncols; col++) {
			/* find the smallest element in the column */
			mdistTemp = mdist     + nrows*col;
			columnEnd      = mdistTemp + nrows;
			
			minValue = *mdistTemp++;			
			while(mdistTemp < columnEnd) {
				value = *mdistTemp++;
				if(value < minValue)
					minValue = value;
			}
			
			/* subtract the smallest element from each element of the column */
			mdistTemp = mdist + nrows*col;
			while(mdistTemp < columnEnd)
				*mdistTemp++ -= minValue;
		}
		
		/* Steps 1 and 2a */
		for(col=0; col<ncols; col++)
			for(row=0; row<nrows; row++)
				if(mdist[row + nrows*col] == 0)
					if(!GET1(crow, row)) {
						SET2(mstar, row, col);
						SET1(ccol, col);
						SET1(crow, row);
						break;
					}
		memset(crow, 0, sizeof(col_t));
	}	
	
	/* move to step 2b */
	step2b(ix, mdist, mstar, nmstar, mprime, ccol, crow, nrows, ncols, dmin);
}

/********************************************************/
static void step2a(int *ix, float *mdist, mat_t mstar, mat_t nmstar, mat_t mprime, col_t ccol, col_t crow, int nrows, int ncols, int dmin)
{
	int col, row;

	/* cover every column containing a starred zero */
	for(col = 0; col < ncols; col++) {
		for(row = 0; row < nrows; row++) {
			if(GET2(mstar, row, col)) {
				SET1(ccol, col);
				break;
			}
		}	
	}
	
	/* move to step 3 */
	step2b(ix, mdist, mstar, nmstar, mprime, ccol, crow, nrows, ncols, dmin);
}

/********************************************************/
static void step2b(int *ix, float *mdist, mat_t mstar, mat_t nmstar, mat_t mprime, col_t ccol, col_t crow, int nrows, int ncols, int dmin)
{
	int col, ncc;
	
	/* count covered columns */
	ncc = 0;
	for(col=0; col<ncols; col++)
		if(GET1(ccol, col))
			ncc++;
	
	if(ncc == dmin) {
		/* algorithm finished */
		buildixvector(ix, mstar, nrows, ncols);
	} else {
		/* move to step 3 */
		step3(ix, mdist, mstar, nmstar, mprime, ccol, crow, nrows, ncols, dmin);
	}
	
}

/********************************************************/
static void step3(int *ix, float *mdist, mat_t mstar, mat_t nmstar, mat_t mprime, col_t ccol, col_t crow, int nrows, int ncols, int dmin)
{
	bool zerosFound;
	int row, col, cstar;

	zerosFound = 1;
	while(zerosFound) {
		zerosFound = 0;		
		for(col=0; col<ncols; col++)
			if(!GET1(ccol,col))
				for(row=0; row<nrows; row++)
					if((!GET1(crow, row)) && (mdist[row + nrows*col] == 0)) {
						/* prime zero */
						SET2(mprime, row, col);
						
						/* find starred zero in current row */
						for(cstar=0; cstar<ncols; cstar++)
							if(GET2(mstar, row, cstar))
								break;
						
						if(cstar == ncols) { /* no starred zero found */
							/* move to step 4 */
							step4(ix, mdist, mstar, nmstar, mprime, ccol, crow, nrows, ncols, dmin, row, col);
							return;
						} else {
							SET1(crow, row);
							CLEAR1(ccol, cstar);
							zerosFound = 1;
							break;
						}
					}
	}
	
	/* move to step 5 */
	step5(ix, mdist, mstar, nmstar, mprime, ccol, crow, nrows, ncols, dmin);
}

/********************************************************/
static void step4(int *ix, float *mdist, mat_t mstar, mat_t nmstar, mat_t mprime, col_t ccol, col_t crow, int nrows, int ncols, int dmin, int row, int col)
{	
	int n, rstar, cstar, primeRow, primeCol;

	/* generate temporary copy of mstar */
	memcpy(nmstar, mstar, sizeof(mat_t));
	
	/* star current zero */
	SET2(nmstar, row, col);

	/* find starred zero in current column */
	cstar = col;
	for(rstar=0; rstar<nrows; rstar++)
		if(GET2(mstar, rstar, cstar))
			break;

	while(rstar<nrows) {
		/* unstar the starred zero */
		CLEAR2(nmstar, rstar, cstar);
		
		/* find primed zero in current row */
		primeRow = rstar;
		for(primeCol=0; primeCol<ncols; primeCol++)
			if(GET2(mprime, primeRow, primeCol))
				break;
		
		/* star the primed zero */
		SET2(nmstar, primeRow, primeCol);
		
		/* find starred zero in current column */
		cstar = primeCol;
		for(rstar=0; rstar<nrows; rstar++)
			if(GET2(mstar, rstar, cstar))
				break;
	}	
	
	/* use temporary copy as new mstar */
	/* delete all primes, uncover all rows */
	memcpy(mstar, nmstar, sizeof(mat_t));
	memset(mprime, 0, sizeof(mat_t));
	memset(crow, 0, sizeof(col_t));
	
	/* move to step 2a */
	step2a(ix, mdist, mstar, nmstar, mprime, ccol, crow, nrows, ncols, dmin);
}

/********************************************************/
static void step5(int *ix, float *mdist, mat_t mstar, mat_t nmstar, mat_t mprime, col_t ccol, col_t crow, int nrows, int ncols, int dmin)
{
	float h = 0, value;
	int row, col, found = 0;

	/* find smallest uncovered element h */
	for(row=0; row<nrows; row++)
		if(!GET1(crow, row))
			for(col=0; col<ncols; col++)
				if(!GET1(ccol,col)) {
					value = mdist[row + nrows*col];
					if(!found || value < h) {
						h = value;
						found = 1;
					}
				}
	
	/* where to go if nothing uncovered? */
	if (!found)
		return;
	
	/* add h to each covered row */
	for(row=0; row<nrows; row++)
		if(GET1(crow, row))
			for(col=0; col<ncols; col++)
				mdist[row + nrows*col] += h;
	
	/* subtract h from each uncovered column */
	for(col=0; col<ncols; col++)
		if(!GET1(ccol,col))
			for(row=0; row<nrows; row++)
				mdist[row + nrows*col] -= h;
	
	/* move to step 3 */
	step3(ix, mdist, mstar, nmstar, mprime, ccol, crow, nrows, ncols, dmin);
}

////////////////////////////////////////////////////////

void match_fingers(int ix[DIM_FINGER], float A[DIM2_FINGER], int nrow, int ncol)
{
	int i;
	float max = 1;
	for (i = 0; i < nrow * ncol; i++)
		if (A[i] > max)
			max = A[i];
	for (i = 0; i < nrow * ncol; i++)
		A[i] /= max;
	ixoptimal(ix, A, nrow, ncol);
}

////////////////////////////////////////////////////////

