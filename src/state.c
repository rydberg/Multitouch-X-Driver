#include "state.h"
#include <stdlib.h>
#include <limits.h>

/******************************************************/

void init_state(struct State *s)
{
	memset(s, 0, sizeof(struct State));
}

/******************************************************/

static int fincmp(const void* a,const void* b)
{
	return ((struct FingerState *)a)->id - ((struct FingerState *)b)->id;
}

inline float dist2(const struct FingerData* a,const struct FingerData* b)
{
	float dx = a->position_x - b->position_x;
	float dy = a->position_y - b->position_y;

	return dx * dx + dy * dy;
}

static void set_finger(struct FingerState* fs,
		       const struct FingerData* hw, int id,
		       const struct Capabilities* caps)
{
	fs->hw = *hw;
	fs->id = id;
	if (!caps->has_touch_minor)
		fs->hw.touch_minor = hw->touch_major;
	if (!caps->has_width_minor)
		fs->hw.width_minor = hw->width_major;
}

inline bool good_finger(const struct FingerState* fs)
{
	return fs->hw.touch_major > 0 && fs->hw.width_major > 0 &&
		fs->hw.touch_minor > 0 && fs->hw.width_minor > 0;
}

/******************************************************/

void modify_state(struct State *s,
		  const struct HWData* hw,
		  const struct Capabilities* caps)
{
	float A[DIM2_FINGER], *row;
	int id[DIM_FINGER], index[DIM_FINGER], i, j;
	struct FingerState *fs = s->finger;

	for (j = 0; j < s->nfinger; j++) {
		id[j] = s->finger[j].id;
		row = A + hw->nfinger * j;
		for (i = 0; i < hw->nfinger; i++)
			row[i] = dist2(&hw->finger[i], &s->finger[j].hw);
	}

	match_fingers(index, A, hw->nfinger, s->nfinger);

	/* update matched fingers and create new ones */
	for (i = 0; i < hw->nfinger; i++) {
		j = index[i];
		if (j >= 0)
			set_finger(fs, hw->finger + i, id[j], caps);
		else
			set_finger(fs, hw->finger + i, ++s->lastid, caps);
		if (good_finger(fs))
			fs++;
	}
	s->nfinger = fs - s->finger;

	/* sort fingers in touching order */
	qsort(s->finger, s->nfinger, sizeof(struct FingerState), fincmp);

	/* make sure wrap-around does not create very strange effects */
	if (s->lastid > INT_MAX / 2) {
		s->lastid = 0;
		for (j = 0; j < s->nfinger; j++)
			s->finger[j].id = ++s->lastid;
	}

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
	xf86Msg(X_INFO, "buttons: %d%d%d\n",
		s->button[0], s->button[1], s->button[2]);
	xf86Msg(X_INFO, "fingers: %d\n",
		s->nfinger);
	for (i = 0; i < s->nfinger; i++) {
		xf86Msg(X_INFO,
			"  %+02d %+05d:%+05d +%05d:%+05d %+06d %+05d:%+05d\n",
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
