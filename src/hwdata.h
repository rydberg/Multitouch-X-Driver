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

#ifndef HWDATA_H
#define HWDATA_H

#include "common.h"

#define DIM_BUTTON 7

#define MT_BUTTON_LEFT 0
#define MT_BUTTON_MIDDLE 1
#define MT_BUTTON_RIGHT 2
#define MT_BUTTON_WHEEL_UP 3
#define MT_BUTTON_WHEEL_DOWN 4
#define MT_BUTTON_HWHEEL_LEFT 5
#define MT_BUTTON_HWHEEL_RIGHT 6

struct FingerData {
	int touch_major, touch_minor;
	int width_major, width_minor;
	int orientation;
	int position_x, position_y;
};

struct HWData {
	struct FingerData finger[DIM_FINGER];
	unsigned button;
	int nfinger, nread;
};

void init_hwdata(struct HWData *hw);
int read_hwdata(struct HWData *hw, const struct input_event* ev);
void output_hwdata(const struct HWData *hw);

#endif
