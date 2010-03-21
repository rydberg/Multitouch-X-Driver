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

#ifndef MTEVENT_H
#define MTEVENT_H

#include "capabilities.h"
#include "hwdata.h"

/* zero id means not mapped (not touching) */
struct FingerState {
	struct FingerData hw;
	int id;
};

struct State {
	struct FingerState finger[DIM_FINGER];
	unsigned button;
	int nfinger;
	mstime_t evtime;
	int lastid;
};

void init_state(struct State *s);
void modify_state(struct State *s,
		  const struct HWData *hw,
		  const struct Capabilities *caps);
void output_state(const struct State *s);

const struct FingerState *find_finger(const struct State *s, int id);
int count_fingers(const struct State *s);

#endif
