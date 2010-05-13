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

#include "memory.h"

void init_memory(struct Memory *mem)
{
	memset(mem, 0, sizeof(struct Memory));
}

void output_memory(const struct Memory *m)
{
	int i;
	xf86Msg(X_INFO, "btdata: %04x\n", m->btdata);
	xf86Msg(X_INFO, "pointing: %04x\n", m->pointing);
	xf86Msg(X_INFO, "moving: %04x\n", m->moving);
	xf86Msg(X_INFO, "ybar: %d\n", m->ybar);
	xf86Msg(X_INFO, "move_time: %lld\n", m->move_time);
}
