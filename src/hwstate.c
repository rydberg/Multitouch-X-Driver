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

#define NOTOUCH(hw, c) ((hw)->touch_major == 0 && (c)->has_touch_major)

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

/* Dmitry Torokhov's code from kernel/driver/input/input.c */
static int defuzz(int value, int old_val, int fuzz)
{
	if (fuzz) {
		if (value > old_val - fuzz / 2 && value < old_val + fuzz / 2)
			return old_val;

		if (value > old_val - fuzz && value < old_val + fuzz)
			return (old_val * 3 + value) / 4;

		if (value > old_val - fuzz * 2 && value < old_val + fuzz * 2)
			return (old_val + value) / 2;
	}

	return value;
}

static void set_finger(struct FingerState *fs,
		       const struct FingerData *hw, int id,
		       const struct Capabilities *caps)
{
	int x = defuzz(hw->position_x, fs->hw.position_x, caps->xfuzz);
	int y = defuzz(hw->position_y, fs->hw.position_y, caps->yfuzz);
	int tj = defuzz(hw->touch_major, fs->hw.touch_major, caps->wfuzz);
	int tn = defuzz(hw->touch_minor, fs->hw.touch_minor, caps->wfuzz);
	int wj = defuzz(hw->width_major, fs->hw.width_major, caps->wfuzz);
	int wn = defuzz(hw->width_minor, fs->hw.width_minor, caps->wfuzz);
	fs->id = id;
	fs->hw = *hw;
	fs->hw.position_x = x;
	fs->hw.position_y = y;
	if (hw->touch_major) {
		fs->hw.touch_major = tj;
		fs->hw.touch_minor = tn;
	}
	fs->hw.width_major = wj;
	fs->hw.width_minor = wn;
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
	int id, i, j;

	/* setup distance matrix for finger id matching */
	for (j = 0; j < s->nfinger; j++) {
		sid[j] = s->finger[j].id;
		if (NOTOUCH(&s->finger[j].hw, caps))
			sid[j] = 0;
		row = A + hw->nfinger * j;
		for (i = 0; i < hw->nfinger; i++)
			row[i] = dist2(&hw->finger[i], &s->finger[j].hw);
	}

	match_fingers(hw2s, A, hw->nfinger, s->nfinger);

	/* update matched fingers and create new ones */
	for (i = 0; i < hw->nfinger; i++) {
		j = hw2s[i];
		id = j >= 0 ? sid[j] : 0;
		if (!NOTOUCH(&hw->finger[i], caps))
			while (!id)
				id = ++s->lastid;
		set_finger(&s->finger[i], &hw->finger[i], id, caps);
	}

	/* clear remaining finger ids */
	for (i = hw->nfinger; i < s->nfinger; i++)
		s->finger[i].id = 0;

	s->button = hw->button;
	s->nfinger = hw->nfinger;
	s->evtime = hw->evtime;
}
