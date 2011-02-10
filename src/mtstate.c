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
#define TOUCH_SCALE(caps) (0.05 * caps->abs[MTDEV_TOUCH_MAJOR].maximum)

#define THUMB_TOUCH(hw) (1.2 * hw->touch_minor)
#define THUMB_TOUCH_SIZE(hw, caps) (0.14 * get_cap_xsize(caps))
#define THUMB_WIDTH_TOUCH(hw) (4 * hw->touch_major)
#define THUMB_WIDTH_WIDTH(hw) (1.2 * hw->width_minor)
#define THUMB_WIDTH_SIZE(hw, caps) (0.25 * get_cap_xsize(caps))

void init_mtstate(struct MTState *s)
{
	memset(s, 0, sizeof(struct MTState));
}

static int touching_finger(const struct FingerState *hw,
			   const struct Capabilities *caps)
{
	if (caps->has_abs[MTDEV_TOUCH_MAJOR] && caps->has_abs[MTDEV_WIDTH_MAJOR])
		return hw->touch_major > TOUCH_WIDTH(hw);
	if (caps->has_abs[MTDEV_TOUCH_MAJOR])
		return hw->touch_major > TOUCH_SCALE(caps);
	return 1;
}

/*
 * Thumb detection:
 *
 * The thumb is large and oval, even when not pressed hard against the
 * surface. The width_major parameter is therefore bounded from below
 * by all three values touch_major, width_minor, and trackpad size.
 *
 */
static int is_thumb(const struct FingerState *hw,
		    const struct Capabilities *caps)
{
	if (!caps->has_abs[MTDEV_TOUCH_MAJOR] ||
	    !caps->has_abs[MTDEV_TOUCH_MINOR])
		return 0;
	if (!caps->has_abs[MTDEV_WIDTH_MAJOR] ||
	    !caps->has_abs[MTDEV_WIDTH_MINOR]) {
		return  hw->touch_major > THUMB_TOUCH(hw) &&
			hw->touch_major > THUMB_TOUCH_SIZE(hw, caps);
	}
	return	hw->touch_major > THUMB_TOUCH(hw) &&
		hw->width_major > THUMB_WIDTH_TOUCH(hw) &&
		hw->width_major > THUMB_WIDTH_WIDTH(hw) &&
		hw->width_major > THUMB_WIDTH_SIZE(hw, caps);
}

void extract_mtstate(struct MTState *s,
		     const struct HWState *hs,
		     const struct Capabilities *caps)
{
	int i;
	s->nfinger = 0;
	s->thumb = 0;
	foreach_bit(i, hs->used) {
		if (!touching_finger(&hs->data[i], caps))
			continue;
		s->finger[s->nfinger] = hs->data[i];
		MODBIT(s->thumb, s->nfinger, is_thumb(&hs->data[i], caps));
		s->nfinger++;
	}

	s->button = hs->button;
	s->evtime = hs->evtime;
}

const struct FingerState *find_finger(const struct MTState *s, int id)
{
	int i;

	for (i = 0; i < s->nfinger; i++)
		if (s->finger[i].tracking_id == id)
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
			s->finger[i].tracking_id,
			s->finger[i].touch_major,
			s->finger[i].touch_minor,
			s->finger[i].width_major,
			s->finger[i].width_minor,
			s->finger[i].orientation,
			s->finger[i].pressure,
			s->finger[i].position_x,
			s->finger[i].position_y);
	}
}
