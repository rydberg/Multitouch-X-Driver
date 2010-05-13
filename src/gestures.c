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

static const int FINGER_THUMB_MS = 400;
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
	float xm[DIM_FINGER], ym[DIM_FINGER];
	float xmove = 0, ymove = 0;

	if (!nmove || nmove != npoint)
		return;

	foreach_bit(i, mt->mem.moving) {
		xm[i] = mt->mem.dx[i];
		ym[i] = mt->mem.dy[i];
		mt->mem.dx[i] = 0;
		mt->mem.dy[i] = 0;
		xmove += xm[i];
		ymove += ym[i];
	}
	xmove /= nmove;
	ymove /= nmove;

	if (nmove == 1) {
		if (mt->mem.moving & mt->mem.thumb) {
			mt_skip_movement(mt, FINGER_THUMB_MS);
			return;
		}
		gs->dx = xmove;
		gs->dy = ymove;
		if (gs->dx || gs->dy)
			SETBIT(gs->type, GS_MOVE);
	} else {
		if (mt->mem.moving & mt->mem.thumb) {
			mt_skip_movement(mt, FINGER_THUMB_MS);
			return;
		}
		gs->dx = xmove;
		gs->dy = ymove;
		if (abs(gs->dx) > abs(gs->dy)) {
			if (nmove == 2)
				SETBIT(gs->type, GS_HSCROLL);
			if (nmove == 3)
				SETBIT(gs->type, GS_HSWIPE);
		}
		if (abs(gs->dy) > abs(gs->dx)) {
			if (nmove == 2)
				SETBIT(gs->type, GS_VSCROLL);
			if (nmove == 3)
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

