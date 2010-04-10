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

#include "hwstate.h"
#include <stdlib.h>
#include <limits.h>

const int XMAX = 32767;

void init_hwstate(struct HWState *s)
{
	memset(s, 0, sizeof(struct HWState));
}

static inline int clamp15(int x)
{
	return x < -XMAX ? -XMAX : x > XMAX ? XMAX : x;
}

/* abslute scale is assumed to fit in 15 bits */
inline int dist2(const struct FingerData *a, const struct FingerData *b)
{
	int dx = clamp15(a->position_x - b->position_x);
	int dy = clamp15(a->position_y - b->position_y);

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

void modify_hwstate(struct HWState *s,
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
		while (!id)
			id = ++s->lastid;
		set_finger(&s->finger[hwk], &hw->finger[hwk], id, caps);
	}

	s->button = hw->button;
	s->nfinger = hw->nfinger;
	s->evtime = hw->evtime;
}
