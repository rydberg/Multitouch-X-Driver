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

#include "state.h"
#include <stdlib.h>
#include <limits.h>

const double FTW = 0.05;
const double FTS = 0.05;

void init_state(struct State *s)
{
	memset(s, 0, sizeof(struct State));
}

static int fincmp(const void *a, const void *b)
{
	return ((struct FingerState *)a)->id - ((struct FingerState *)b)->id;
}

/* seander@cs.stanford.edu */
inline unsigned abs32(int x)
{
	int const m = x >> 31;
	return (x + m) ^ m;
}

inline int abs15(int x)
{
	return 32767 & abs32(x);
}

/* abslute scale is assumed to fit in 15 bits */
inline int dist2(const struct FingerData *a, const struct FingerData *b)
{
	int dx = abs15(a->position_x - b->position_x);
	int dy = abs15(a->position_y - b->position_y);

	return dx * dx + dy * dy;
}

static void set_finger(struct FingerState *fs,
		       const struct FingerData *hw, int id,
		       const struct Capabilities *caps)
{
	fs->hw = *hw;
	fs->id = id;
	if (!caps->has_touch_minor)
		fs->hw.touch_minor = hw->touch_major;
	if (!caps->has_width_minor)
		fs->hw.width_minor = hw->width_major;
}

static int touching_finger(const struct FingerData *hw,
			   const struct Capabilities *caps)
{
	if (caps->has_touch_major && caps->has_width_major)
		return hw->width_major > 0 &&
			hw->touch_major > FTW * hw->width_major;
	if (caps->has_touch_major)
		return hw->touch_major > FTS * caps->abs_touch_major.maximum;
	return 1;
}

void modify_state(struct State *s,
		  const struct HWData *hw,
		  const struct Capabilities *caps)
{
	int A[DIM2_FINGER], *row;
	int sid[DIM_FINGER], hw2s[DIM_FINGER];
	int id, sk, hwk;

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
		id = sk >= 0 ? sid[sk] : 0;
		if (!touching_finger(&hw->finger[hwk], caps))
			id = 0;
		else
			while (!id)
				id = ++s->lastid;
		set_finger(&s->finger[hwk], &hw->finger[hwk], id, caps);
	}

	s->button = hw->button;
	s->nfinger = hw->nfinger;

	/* sort fingers in touching order */
	qsort(s->finger, s->nfinger, sizeof(struct FingerState), fincmp);
}

const struct FingerState *find_finger(const struct State *s, int id)
{
	int i;

	if (!id)
		return NULL;
	for (i = 0; i < s->nfinger; i++)
		if (s->finger[i].id == id)
			return s->finger + i;

	return NULL;
}

int count_fingers(const struct State *s)
{
	int i, n = 0;
	for (i = 0; i < s->nfinger; i++)
		if (s->finger[i].id)
			n++;
	return n;
}

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
			"  %+02d %+05d:%+05d +%05d:%+05d "
			"%+06d %+06d %+05d:%+05d\n",
			s->finger[i].id,
			s->finger[i].hw.touch_major,
			s->finger[i].hw.touch_minor,
			s->finger[i].hw.width_major,
			s->finger[i].hw.width_minor,
			s->finger[i].hw.orientation,
			s->finger[i].hw.pressure,
			s->finger[i].hw.position_x,
			s->finger[i].hw.position_y);
	}
}
