/***************************************************************************
 *
 * Multitouch X driver
 * Copyright (C) 2008 Henrik Rydberg <rydberg@euromail.se>
 *
 * Gestures
 * Copyright (C) 2008 Henrik Rydberg <rydberg@euromail.se>
 * Copyright (C) 2010 Arturo Castro <mail@arturocastro.net>
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

#include "gestures.h"

/**
 * extract_buttons
 *
 * Set the button gesture.
 *
 * Reset memory after use.
 *
 */
static void extract_buttons(struct Gestures *gs, struct MTouch* mt)
{
	unsigned btdata = mt->state.button & BITONES(DIM_BUTTON);
	int npoint = bitcount(mt->mem.pointing);
	if (mt->state.button == BITMASK(MT_BUTTON_LEFT)) {
		if (npoint == 2)
			btdata = BITMASK(MT_BUTTON_RIGHT);
		if (npoint == 3)
			btdata = BITMASK(MT_BUTTON_MIDDLE);
	}
	if (mt->state.button != mt->prev_state.button) {
		gs->btmask = (btdata ^ mt->mem.btdata) & BITONES(DIM_BUTTON);
		gs->btdata = btdata;
		mt->mem.btdata = btdata;
	}
	if (gs->btmask)
		SETBIT(gs->type, GS_BUTTON);
}

/**
 * extract_movement
 *
 * Set the movement gesture.
 *
 * Reset memory after use.
 *
 */
static void extract_movement(struct Gestures *gs, struct MTouch* mt)
{
	int npoint = bitcount(mt->mem.pointing);
	int nmove = bitcount(mt->mem.moving);
	int i;
	if (nmove && mt->state.evtime >= mt->mem.move_time) {
		for (i = 0; i < mt->state.nfinger; i++) {
			if (GETBIT(mt->mem.moving, i)) {
				gs->dx += mt->mem.dx[i];
				gs->dy += mt->mem.dy[i];
			}
		}
		gs->dx /= nmove;
		gs->dy /= nmove;
		memset(mt->mem.dx, 0, sizeof(mt->mem.dx));
		memset(mt->mem.dy, 0, sizeof(mt->mem.dy));
	}
	if (gs->dx || gs->dy) {
		if (npoint == 1)
			SETBIT(gs->type, GS_MOVE);
		if (npoint == 2) {
			if (gs->dx)
				SETBIT(gs->type, GS_HSCROLL);
			if (gs->dy)
				SETBIT(gs->type, GS_VSCROLL);
		}
		if (npoint == 3) {
			if (gs->dx)
				SETBIT(gs->type, GS_HSWIPE);
			if (gs->dy)
				SETBIT(gs->type, GS_VSWIPE);
		}
	}
}

/**
 * extract_gestures
 *
 * Extract the gestures.
 *
 * Reset memory after use.
 *
 */
void extract_gestures(struct Gestures *gs, struct MTouch* mt)
{
	memset(gs, 0, sizeof(struct Gestures));
	gs->same_fingers = mt->mem.same;
	extract_buttons(gs, mt);
	extract_movement(gs, mt);
	mt->prev_state = mt->state;
}

