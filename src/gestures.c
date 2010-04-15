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

#define DELTA_CUT(x) (0.2 * (x))

/* timer for cursor stability on finger touch/release */
static const int FINGER_ATTACK_MS = 70;
static const int FINGER_DECAY_MS = 120;

static inline int maxval(int x, int y) { return x > y ? x : y; }

inline int dxval(const struct FingerState *a, const struct FingerState *b)
{
	return a->hw.position_x - b->hw.position_x;
}
inline int dyval(const struct FingerState *a, const struct FingerState *b)
{
	return a->hw.position_y - b->hw.position_y;
}

static void extract_finger_configuration(struct Gestures *gs, struct MTouch* mt)
{
	const struct FingerState *f = mt->state.finger;
	int i, same = mt->state.nfinger == mt->prev_state.nfinger;
	for (i = 0; i < mt->state.nfinger; i++)
		same = same && find_finger(&mt->prev_state, f[i].id);
	gs->same_fingers = same;
}

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

	if (gs->same_fingers) {
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
	const struct FingerState *prev, *f = mt->state.finger;
	int i, x = 0, y = 0;
	int dx, dy, xcut, ycut, xmax = 0, ymax = 0;

	mt->mem.moving = 0;
	mt->mem.nmove = 0;

	if (mt->state.nfinger == 0)
		return;

	if (!gs->same_fingers) {
		mt->mem.move_time = mt->state.evtime;
		if (mt->state.nfinger > mt->prev_state.nfinger)
			mt->mem.move_time += FINGER_ATTACK_MS;
		else
			mt->mem.move_time += FINGER_DECAY_MS;
		memset(mt->mem.dx, 0, sizeof(mt->mem.dx));
		memset(mt->mem.dy, 0, sizeof(mt->mem.dy));
	} else {
		for (i = 0; i < mt->state.nfinger; i++) {
			if (!GETBIT(mt->mem.pointing, i))
				continue;
			prev = find_finger(&mt->prev_state, f[i].id);
			dx = dxval(&f[i], prev);
			dy = dyval(&f[i], prev);
			mt->mem.dx[i] += dx;
			mt->mem.dy[i] += dy;
			xmax = maxval(xmax, abs(mt->mem.dx[i]));
			ymax = maxval(ymax, abs(mt->mem.dy[i]));
		}
		xcut = DELTA_CUT(xmax);
		ycut = DELTA_CUT(ymax);
		for (i = 0; i < mt->state.nfinger; i++) {
			if (abs(mt->mem.dx[i]) > xcut ||
			    abs(mt->mem.dy[i]) > ycut) {
				SETBIT(mt->mem.moving, i);
				mt->mem.nmove++;
			}
		}

		/* accumulate all movement during delay */
		if (mt->mem.nmove && mt->state.evtime >= mt->mem.move_time) {
			for (i = 0; i < mt->state.nfinger; i++) {
				if (GETBIT(mt->mem.moving, i)) {
					gs->dx += mt->mem.dx[i];
					gs->dy += mt->mem.dy[i];
				}
			}
			gs->dx /= mt->mem.nmove;
			gs->dy /= mt->mem.nmove;
			memset(mt->mem.dx, 0, sizeof(mt->mem.dx));
			memset(mt->mem.dy, 0, sizeof(mt->mem.dy));
		}
	}
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
		if (mt->mem.npoint == 1)
			SETBIT(gs->type, GS_MOVE);
		if (mt->mem.npoint == 2) {
			if (gs->dx)
				SETBIT(gs->type, GS_HSCROLL);
			if (gs->dy)
				SETBIT(gs->type, GS_VSCROLL);
		}
		if (mt->mem.npoint == 3) {
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
	extract_finger_configuration(gs, mt);
	extract_pointers(gs, mt);
	extract_movement(gs, mt);
	extract_buttons(gs, mt);
	extract_type(gs, mt);
	mt->prev_state = mt->state;
#if 0
	output_memory(&mt->mem);
#endif
}

