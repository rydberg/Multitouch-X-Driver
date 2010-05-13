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

#define DIM_BUTTON 13

#define MT_BUTTON_LEFT 0
#define MT_BUTTON_MIDDLE 1
#define MT_BUTTON_RIGHT 2
#define MT_BUTTON_WHEEL_UP 3
#define MT_BUTTON_WHEEL_DOWN 4
#define MT_BUTTON_HWHEEL_LEFT 5
#define MT_BUTTON_HWHEEL_RIGHT 6
#define MT_BUTTON_SWIPE_UP 7
#define MT_BUTTON_SWIPE_DOWN 8
#define MT_BUTTON_SWIPE_LEFT 9
#define MT_BUTTON_SWIPE_RIGHT 10
#define MT_BUTTON_SCALE_DOWN 11
#define MT_BUTTON_SCALE_UP 12

#define BIT_MT_TOUCH_MAJOR 0
#define BIT_MT_TOUCH_MINOR 1
#define BIT_MT_WIDTH_MAJOR 2
#define BIT_MT_WIDTH_MINOR 3
#define BIT_MT_ORIENTATION 4
#define BIT_MT_PRESSURE 5
#define BIT_MT_POSITION_X 6
#define BIT_MT_POSITION_Y 7
#define BIT_MT_CNT 8

struct FingerData {
	int touch_major, touch_minor;
	int width_major, width_minor;
	int orientation, pressure;
	int position_x, position_y;
};

/* year-proof millisecond event time */
typedef __u64 mstime_t;

/**
 * struct HWData - hardware reads
 *
 * @finger: finger data
 * @mask: bits corresponding to data actually read (readonly)
 * @mread: bits corresponding to data in progress (writeonly)
 * @button: bitmask of buttons
 * @nfinger: number of fingers actually read (readonly)
 * @nread: number of fingers in progress (writeonly)
 *
 */
struct HWData {
	struct FingerData finger[DIM_FINGER];
	unsigned mask[DIM_FINGER], mread[DIM_FINGER];
	unsigned button;
	int nfinger, mtread, nread;
	mstime_t evtime;
};

void init_hwdata(struct HWData *hw);
int read_hwdata(struct HWData *hw, const struct input_event* ev);
void output_hwdata(const struct HWData *hw);

static inline int finger_dist2(const struct FingerData *a,
			       const struct FingerData *b)
{
	return dist2(a->position_x - b->position_x,
		     a->position_y - b->position_y);
}

#endif
