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

/* timer for cursor stability on finger touch/release */
static const int FINGER_ATTACK_MS = 70;
static const int FINGER_DECAY_MS = 120;

static void extract_pointers(struct Gestures *gs, struct MTouch* mt)
{
	const struct FingerState *f = mt->state.finger;
	int i;

	if (mt->state.nfinger < 2) {
		mt->mem.pointing = mt->state.nfinger;
		mt->mem.npoint = mt->state.nfinger;
		mt->mem.ybar = mt->caps.abs_position_y.maximum;
		return;
	}

	if (mt->state.nfinger == mt->prev_state.nfinger) {
		for (i = 0; i < mt->state.nfinger; i++) {
			if (GETBIT(mt->mem.pointing, i))
				continue;
			if (f[i].hw.position_y <= mt->mem.ybar) {
				mt->mem.pointing = BITONES(mt->state.nfinger);
				mt->mem.npoint = mt->state.nfinger;
				return;
			}
		}
		return;
	}

	mt->mem.pointing = 0;
	mt->mem.npoint = 0;
	mt->mem.ybar = mt->caps.yclick;
	for (i = 0; i < mt->state.nfinger; i++) {
		if (f[i].hw.position_y > mt->caps.yclick)
			continue;
		if (!mt->mem.npoint || f[i].hw.position_y > mt->mem.ybar)
			mt->mem.ybar = f[i].hw.position_y;
		SETBIT(mt->mem.pointing, i);
		mt->mem.npoint++;
	}

}

static void extract_movement(struct Gestures *gs, struct MTouch* mt)
{
	const struct FingerState *prev[DIM_FINGER];
	const struct FingerState *f = mt->state.finger;
	int same_fingers, i, x = 0, y = 0;

	if (mt->state.nfinger == 0)
		return;

	same_fingers = mt->state.nfinger == mt->prev_state.nfinger;
	for (i = 0; i < mt->state.nfinger; i++) {
		prev[i] = find_finger(&mt->prev_state, mt->state.finger[i].id);
		same_fingers = same_fingers && prev[i];
	}

	for (i = 0; i < mt->state.nfinger; i++) {
		x += f[i].hw.position_x;
		y += f[i].hw.position_y;
	}
	x /= mt->state.nfinger;
	y /= mt->state.nfinger;

	if (!same_fingers) {
		mt->mem.move_time = mt->state.evtime;
		if (mt->state.nfinger > mt->prev_state.nfinger)
			mt->mem.move_time += FINGER_ATTACK_MS;
		else
			mt->mem.move_time += FINGER_DECAY_MS;
	} else if (mt->state.evtime >= mt->mem.move_time) {
		gs->dx = x - mt->mem.move_x;
		gs->dy = y - mt->mem.move_y;
	} else {
		/* accumulate all movement during delay */
		return;
	}

	mt->mem.move_x = x;
	mt->mem.move_y = y;
}

static void extract_buttons(struct Gestures *gs, struct MTouch* mt)
{
	unsigned btdata = mt->state.button & BITONES(DIM_BUTTON);
	if (mt->state.button == BITMASK(MT_BUTTON_LEFT)) {
		if (mt->mem.npoint == 2)
			btdata = BITMASK(MT_BUTTON_RIGHT);
		if (mt->mem.npoint == 3)
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
		if (mt->state.nfinger == 2) {
			if (gs->dx)
				SETBIT(gs->type, GS_HSCROLL);
			if (gs->dy)
				SETBIT(gs->type, GS_VSCROLL);
		}
		if (mt->state.nfinger == 3) {
			if (gs->dx)
				SETBIT(gs->type, GS_HSWIPE);
			if (gs->dy)
				SETBIT(gs->type, GS_VSWIPE);
		}
	}
}

/******************************************************/

void extract_gestures(struct Gestures *gs, struct MTouch* mt)
{
	memset(gs, 0, sizeof(struct Gestures));
	extract_pointers(gs, mt);
	extract_movement(gs, mt);
	extract_buttons(gs, mt);
	extract_type(gs, mt);
	mt->prev_state = mt->state;
}

