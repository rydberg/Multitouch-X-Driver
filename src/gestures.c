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
	const struct FingerState *b = mt->state.finger;
	const struct FingerState *e = b + mt->state.nfinger;
	const struct FingerState *p, *fs;
	int dn = 0, i;
	if (mt->state.nfinger == mt->prev_state.nfinger) {
		for (p = b; p != e; p++) {
			fs = find_finger(&mt->prev_state, p->id);
			if (fs) {
				gs->dx += p->hw.position_x - fs->hw.position_x;
				gs->dy += p->hw.position_y - fs->hw.position_y;
				dn++;
			}
		}
	}
	if (dn) {
		gs->dx /= dn;
		gs->dy /= dn;
	}
}

static void extract_buttons(struct Gestures *gs, struct MTouch* mt)
{
	if (mt->state.button == BITMASK(MT_BUTTON_LEFT)) {
		if (mt->state.nfinger == 2)
			mt->state.button = BITMASK(MT_BUTTON_RIGHT);
		if (mt->state.nfinger == 3)
			mt->state.button = BITMASK(MT_BUTTON_MIDDLE);
	}
	gs->btmask = (mt->state.button ^ mt->prev_state.button) & BITONES(DIM_BUTTON);
	gs->btdata = mt->state.button & BITONES(DIM_BUTTON);
}

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

