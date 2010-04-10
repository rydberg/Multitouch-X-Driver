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

#include "gestures.h"

static void extract_movement(struct Gestures *gs, struct MTouch* mt)
{
	const struct FingerState *prev[DIM_FINGER];
	const struct FingerState *f = mt->state.finger;
	int same_fingers, i;

	if (mt->state.nfinger == 0)
		return;

	same_fingers = mt->state.nfinger == mt->prev_state.nfinger;
	for (i = 0; i < mt->state.nfinger; i++) {
		prev[i] = find_finger(&mt->prev_state, mt->state.finger[i].id);
		same_fingers = same_fingers && prev[i];
	}

	if (!same_fingers)
		return;

	for (i = 0; i < mt->state.nfinger; i++) {
		gs->dx += f[i].hw.position_x - prev[i]->hw.position_x;
		gs->dy += f[i].hw.position_y - prev[i]->hw.position_y;
	}

	gs->dx /= mt->state.nfinger;
	gs->dy /= mt->state.nfinger;
}

static void extract_buttons(struct Gestures *gs, struct MTouch* mt)
{
	unsigned btdata = mt->state.button & BITONES(DIM_BUTTON);
	if (mt->state.button == BITMASK(MT_BUTTON_LEFT)) {
		if (mt->state.nfinger == 2)
			btdata = BITMASK(MT_BUTTON_RIGHT);
		if (mt->state.nfinger == 3)
			btdata = BITMASK(MT_BUTTON_MIDDLE);
	}
	gs->btmask = (btdata ^ mt->mem.btdata) & BITONES(DIM_BUTTON);
	gs->btdata = btdata;
	mt->mem.btdata = btdata;
}

/******************************************************/

static void extract_type(struct Gestures *gs, struct MTouch* mt)
{
	if (gs->dx || gs->dy) {
		if (mt->state.nfinger == 1)
			SETBIT(gs->type, GS_MOVE);
		if (mt->state.nfinger == 2)
			SETBIT(gs->type, GS_VSCROLL);
		if (mt->state.nfinger == 3)
			SETBIT(gs->type, GS_HSCROLL);
	}
}

/******************************************************/

void extract_gestures(struct Gestures *gs, struct MTouch* mt)
{
	memset(gs, 0, sizeof(struct Gestures));
	extract_movement(gs, mt);
	extract_buttons(gs, mt);
	extract_type(gs, mt);
	mt->prev_state = mt->state;
}

