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

#ifndef HWSTATE_H
#define HWSTATE_H

#include "mtdev-caps.h"
#include "hwdata.h"

/* zero id means not mapped (not touching) */
struct FingerState {
	struct FingerData hw;
	int id;
};

struct HWState {
	struct FingerState finger[DIM_FINGER];
	bitmask_t button;
	int nfinger;
	mstime_t evtime;
	int lastid;
};

void init_hwstate(struct HWState *s);
void modify_hwstate(struct HWState *s,
		    const struct HWData *hw,
		    const struct Capabilities *caps);

#endif
