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

#ifndef MEMORY_H
#define MEMORY_H

#include "mtstate.h"

/**
 * struct Memory - parsing state
 *
 * @btdata: logical finger state
 * @same: true if the finger configuration is unchanged
 * @pointing: bitmask of pointing fingers
 * @pending: bitmask of tentatively moving fingers
 * @moving: bitmask of moving fingers
 * @ybar: vertical position on pad marking the clicking area
 * @move_time: movement before this point in time is accumulated
 * @dx: array of accumulated horiontal movement per finger
 * @dy: array of accumulated vertical movement per finger
 *
 */
struct Memory {
	unsigned btdata, same;
	unsigned pointing, pending, moving;
	int ybar;
	mstime_t move_time;
	int dx[DIM_FINGER], dy[DIM_FINGER];
};

void init_memory(struct Memory *mem);
void refresh_memory(struct Memory *m,
		    const struct MTState *prev_state,
		    const struct MTState *state,
		    const struct Capabilities *caps);
void output_memory(const struct Memory *m);

#endif
