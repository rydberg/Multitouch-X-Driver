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

#ifndef MTOUCH_H
#define MTOUCH_H

#include "mtdev-iobuf.h"
#include "hwdata.h"
#include "hwstate.h"
#include "mtstate.h"
#include "memory.h"

struct MTouch {
	struct Capabilities caps;
	struct IOBuffer buf;
	struct HWData hw;
	struct HWState hs;
	struct MTState prev_state, state;
	struct Memory mem;
};

int configure_mtouch(struct MTouch *mt, int fd);
int open_mtouch(struct MTouch *mt, int fd);
int close_mtouch(struct MTouch *mt, int fd);

int read_synchronized_event(struct MTouch *mt, int fd);
void parse_event(struct MTouch *mt);


static inline void mt_delay_movement(struct MTouch *mt, int t)
{
	mem_hold_movement(&mt->mem, mt->state.evtime + t);
}

static inline void mt_skip_movement(struct MTouch *mt, int t)
{
	mem_forget_movement(&mt->mem, mt->state.evtime + t);
}

#endif
