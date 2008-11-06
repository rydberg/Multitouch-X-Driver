#include "state.h"
#include <stdlib.h>

/******************************************************/

void init_state(struct State *s)
{
	memset(s, 0, sizeof(struct State));
}

/******************************************************/

inline int fincomp(const struct FingerState* a,const struct FingerState* b)
{
	return a->id - b->id;
}

inline float dist2(const struct FingerData* a,const struct FingerData* b)
{
	float dx = a->position_x - b->position_x;
	float dy = a->position_y - b->position_y;

	return dx * dx + dy * dy;
}

void modify_state(struct State *s, const struct HWData* hw)
{
	float A[DIM2_FINGER], *row;
	int id[DIM_FINGER], index[DIM_FINGER], i, j;

	for (j = 0; j < s->nfinger; j++) {
		id[j] = s->finger[j].id;
		row = A + hw->nfinger * j;
		for (i = 0; i < hw->nfinger; i++)
			row[i] = dist2(&hw->finger[i], &s->finger[j].hw);
	}

	match_fingers(index, A, hw->nfinger, s->nfinger);

	s->nfinger = 0;

	/* update matched fingers */
	for (i = 0; i < hw->nfinger; i++) {
		if ((j = index[i]) >= 0) {
			s->finger[s->nfinger].id = id[j];
			s->finger[s->nfinger].hw = hw->finger[i];
			s->nfinger++;
		}
	}

	/* create new fingers */
	for (i = 0; i < hw->nfinger; i++) {
		if (index[i] < 0) {
			s->finger[s->nfinger].id = ++s->lastid;
			s->finger[s->nfinger].hw = hw->finger[i];
			s->nfinger++;
		}
	}

	/* sort fingers in touching order */
	qsort(s->finger, s->nfinger, sizeof(struct FingerState),
	      (int (*)(const void*,const void*))fincomp);

	/* copy buttons */
	for (i = 0; i < DIM_BUTTON; i++)
		s->button[i] = hw->button[i];
}

/******************************************************/

const struct FingerState *find_finger(const struct State *s, int id)
{
	int i;

	for (i = 0; i < s->nfinger; i++)
		if (s->finger[i].id == id)
			return s->finger + i;

	return NULL;
}

/******************************************************/

void output_state(const struct State *s)
{
	int i;
	printf("buttons: %d%d%d\n", s->button[0], s->button[1], s->button[2]);
	printf("fingers: %d\n", s->nfinger);
	for (i = 0; i < s->nfinger; i++) {
		printf("  %+02d %+05d:%+05d +%05d:%+05d %+05d %+05d:%+05d\n",
		       s->finger[i].id,
		       s->finger[i].hw.touch_major,
		       s->finger[i].hw.touch_minor,
		       s->finger[i].hw.width_major,
		       s->finger[i].hw.width_minor,
		       s->finger[i].hw.orientation,
		       s->finger[i].hw.position_x,
		       s->finger[i].hw.position_y);
	}
}

/******************************************************/
