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

/******************************************************/

void modify_state(struct State *s,
		  const struct HWData* hw,
		  const struct Capabilities* caps)
{
	float A[DIM2_FINGER], *row;
	int sid[DIM_FINGER], hw2s[DIM_FINGER], id, sk, hwk;

	/* setup distance matrix for finger id matching */
	for (sk = 0; sk < s->nfinger; sk++) {
		sid[sk] = s->finger[sk].id;
		row = A + hw->nfinger * sk;
		for (hwk = 0; hwk < hw->nfinger; hwk++)
			row[hwk] = dist2(&hw->finger[hwk], &s->finger[sk].hw);
	}

	match_fingers(hw2s, A, hw->nfinger, s->nfinger);

	/* update matched fingers and create new ones */
	for (hwk = 0; hwk < hw->nfinger; hwk++) {
		sk = hw2s[hwk];
		id = sk < 0 ? s->nextid++ : sid[sk];
		set_finger(s->finger + hwk, hw->finger + hwk, id, caps);
	}

	s->button = hw->button;
	s->nfinger = hw->nfinger;

	/* sort fingers in touching order */
	qsort(s->finger, s->nfinger, sizeof(struct FingerState), fincmp);
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
		GETBIT(s->button, MT_BUTTON_LEFT),
		GETBIT(s->button, MT_BUTTON_MIDDLE),
		GETBIT(s->button, MT_BUTTON_RIGHT));
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
