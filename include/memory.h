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
 * @fingers: bitmask of fingers on the pad
 * @added: bitmask of new fingers on the pad
 * @thumb: bitmask of thumbs on the pad
 * @pointing: bitmask of pointing fingers
 * @pending: bitmask of tentatively moving fingers
 * @moving: bitmask of moving fingers
 * @ybar: vertical position on pad marking the clicking area
 * @mvhold: movement before this point in time is accumulated
 * @mvforget: movement before this point in time is discarded
 * @dx: array of accumulated horiontal movement per finger
 * @dy: array of accumulated vertical movement per finger
 * @tpdown: time of first touch
 * @tpup: time of last release
 * @tprelax: time of next possible touch
 * @xdown: x position of first touch
 * @ydown: y position of first touch
 * @xup: x position of last release
 * @yup: y position of last release
 * @wait: time to wait for a second tap
 * @maxtap: max number of pointing fingers during touch
 * @ntap: number of taps in sequence
 *
 */
struct Memory {
	bitmask_t btdata, same;
	bitmask_t fingers, added, thumb;
	bitmask_t pointing, pending, moving;
	int ybar;
	mstime_t mvhold, mvforget;
	int dx[DIM_FINGER], dy[DIM_FINGER];
	mstime_t tpdown, tpup, tprelax;
	int xdown, ydown, xup, yup;
	int wait, maxtap, ntap;
};

void init_memory(struct Memory *mem);
void refresh_memory(struct Memory *m,
		    const struct MTState *prev_state,
		    const struct MTState *state,
		    const struct Capabilities *caps);
void output_memory(const struct Memory *m);

static inline void mem_hold_movement(struct Memory *m, mstime_t t)
{
	if (t > m->mvhold)
		m->mvhold = t;
}

static inline void mem_forget_movement(struct Memory *m, mstime_t t)
{
	if (t > m->mvforget)
		m->mvforget = t;
}

#endif
