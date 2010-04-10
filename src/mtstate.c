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

#include "mtstate.h"

#define TOUCH_WIDTH(hw) (0.05 * hw->width_major)
#define TOUCH_SCALE(caps) (0.05 * caps->abs_touch_major.maximum)

void init_mtstate(struct MTState *s)
{
	memset(s, 0, sizeof(struct MTState));
}

static int touching_finger(const struct FingerData *hw,
			   const struct Capabilities *caps)
{
	if (caps->has_touch_major && caps->has_width_major)
		return hw->touch_major > TOUCH_WIDTH(hw);
	if (caps->has_touch_major)
		return hw->touch_major > TOUCH_SCALE(caps);
	return 1;
}

void extract_mtstate(struct MTState *s,
		     const struct HWState *hs,
		     const struct Capabilities *caps)
{
	int i;

	s->nfinger = 0;
	for (i = 0; i < hs->nfinger; i++)
		if (touching_finger(&hs->finger[i].hw, caps))
			s->finger[s->nfinger++] = hs->finger[i];

	s->button = hs->button;
	s->evtime = hs->evtime;
}

const struct FingerState *find_finger(const struct MTState *s, int id)
{
	int i;

	for (i = 0; i < s->nfinger; i++)
		if (s->finger[i].id == id)
			return s->finger + i;

	return NULL;
}

void output_mtstate(const struct MTState *s)
{
	int i;
	xf86Msg(X_INFO, "buttons: %d%d%d\n",
		GETBIT(s->button, MT_BUTTON_LEFT),
		GETBIT(s->button, MT_BUTTON_MIDDLE),
		GETBIT(s->button, MT_BUTTON_RIGHT));
	xf86Msg(X_INFO, "fingers: %d\n",
		s->nfinger);
	xf86Msg(X_INFO, "evtime: %lld\n",
		s->evtime);
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
