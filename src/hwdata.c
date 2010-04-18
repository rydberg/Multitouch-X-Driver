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

#include "hwdata.h"

void init_hwdata(struct HWData *hw)
{
	memset(hw, 0, sizeof(struct HWData));
}

static void set_value(struct HWData *hw, int code, int value)
{
	if (hw->nread < DIM_FINGER) {
		(&hw->finger[hw->nread].touch_major)[code] = value;
		SETBIT(hw->mread[hw->nread], code);
	}
	hw->mtread++;
}

static void accept_finger(struct HWData *hw)
{
	if (hw->nread < DIM_FINGER &&
	    GETBIT(hw->mread[hw->nread], BIT_MT_POSITION_X) &&
	    GETBIT(hw->mread[hw->nread], BIT_MT_POSITION_Y)) {
		hw->mask[hw->nread] = hw->mread[hw->nread];
		hw->nread++;
	}
	if (hw->nread < DIM_FINGER)
		hw->mread[hw->nread] = 0;
}

static void accept_packet(struct HWData *hw, const struct timeval* tv)
{
	static const mstime_t ms = 1000;
	if (hw->mtread)
		hw->nfinger = hw->nread;
	hw->mtread = 0;
	hw->nread = 0;
	hw->mread[hw->nread] = 0;
	hw->evtime = tv->tv_usec / ms + tv->tv_sec * ms;
}

int read_hwdata(struct HWData *hw, const struct input_event* ev)
{
	switch (ev->type) {
	case EV_SYN:
		switch (ev->code) {
		case SYN_REPORT:
			accept_packet(hw, &ev->time);
			return 1;
		case SYN_MT_REPORT:
			accept_finger(hw);
			break;
		}
		break;
	case EV_KEY:
		switch (ev->code) {
		case BTN_TOUCH:
			hw->mtread++;
			break;
		case BTN_LEFT:
			if (ev->value)
				SETBIT(hw->button, MT_BUTTON_LEFT);
			else
				CLEARBIT(hw->button, MT_BUTTON_LEFT);
			break;
		case BTN_MIDDLE:
			if (ev->value)
				SETBIT(hw->button, MT_BUTTON_MIDDLE);
			else
				CLEARBIT(hw->button, MT_BUTTON_MIDDLE);
			break;
		case BTN_RIGHT:
			if (ev->value)
				SETBIT(hw->button, MT_BUTTON_RIGHT);
			else
				CLEARBIT(hw->button, MT_BUTTON_RIGHT);
			break;
		}
		break;
	case EV_ABS:
		switch (ev->code) {
		case ABS_MT_TOUCH_MAJOR:
			set_value(hw, BIT_MT_TOUCH_MAJOR, ev->value);
			break;
		case ABS_MT_TOUCH_MINOR:
			set_value(hw, BIT_MT_TOUCH_MINOR, ev->value);
			break;
		case ABS_MT_WIDTH_MAJOR:
			set_value(hw, BIT_MT_WIDTH_MAJOR, ev->value);
			break;
		case ABS_MT_WIDTH_MINOR:
			set_value(hw, BIT_MT_WIDTH_MINOR, ev->value);
			break;
		case ABS_MT_ORIENTATION:
			set_value(hw, BIT_MT_ORIENTATION, ev->value);
			break;
		case ABS_MT_PRESSURE:
			set_value(hw, BIT_MT_PRESSURE, ev->value);
			break;
		case ABS_MT_POSITION_X:
			set_value(hw, BIT_MT_POSITION_X, ev->value);
			break;
		case ABS_MT_POSITION_Y:
			set_value(hw, BIT_MT_POSITION_Y, ev->value);
			break;
		}
		break;
	}
	return 0;
}

void output_hwdata(const struct HWData *hw)
{
}
