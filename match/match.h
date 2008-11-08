#ifndef MATCHER_H
#define MATCHER_H

/**
 * Special implementation of the hungarian algorithm.
 * The maximum number of fingers matches a uint32.
 * Bitmasks are used extensively.
 */

#define DIM_FINGER 16
#define DIM2_FINGER (DIM_FINGER * DIM_FINGER)

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) < (b) ? (b) : (a))

#define GETBIT(m, x) ((m>>(x))&1U)
#define SETBIT(m, x) (m|=(1U<<(x)))
#define CLEARBIT(m, x) (m&=~(1U<<(x)))

typedef int bool;

////////////////////////////////////////////////////////

void match_fingers(int index[DIM_FINGER], float A[DIM2_FINGER],
		   int nrow, int ncol);

////////////////////////////////////////////////////////

#endif
