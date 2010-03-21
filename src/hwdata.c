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

int read_hwdata(struct HWData *hw, const struct input_event* ev)
{
	switch (ev->type) {
	case EV_SYN:
		switch (ev->code) {
		case SYN_REPORT:
			hw->nread = 0;
			return 1;
		case SYN_MT_REPORT:
			hw->nread++;
			hw->nfinger = hw->nread;
			break;
		}
		break;
	case EV_KEY:
		switch (ev->code) {
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
		if (hw->nread == DIM_FINGER)
			break;
		switch (ev->code) {
		case ABS_MT_TOUCH_MAJOR:
			hw->finger[hw->nread].touch_major = ev->value;
			break;
		case ABS_MT_TOUCH_MINOR:
			hw->finger[hw->nread].touch_minor = ev->value;
			break;
		case ABS_MT_WIDTH_MAJOR:
			hw->finger[hw->nread].width_major = ev->value;
			break;
		case ABS_MT_WIDTH_MINOR:
			hw->finger[hw->nread].width_minor = ev->value;
			break;
		case ABS_MT_ORIENTATION:
			hw->finger[hw->nread].orientation = ev->value;
			break;
		case ABS_MT_PRESSURE:
			hw->finger[hw->nread].pressure = ev->value;
			break;
		case ABS_MT_POSITION_X:
			hw->finger[hw->nread].position_x = ev->value;
			break;
		case ABS_MT_POSITION_Y:
			hw->finger[hw->nread].position_y = ev->value;
			break;
		}
		break;
	}
	return 0;
}

void output_hwdata(const struct HWData *hw)
{
}
