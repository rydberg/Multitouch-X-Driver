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

/* fraction of max movement threshold */
#define DELTA_CUT(x) (0.2 * (x))

/* timer for cursor stability on finger touch/release */
static const int FINGER_ATTACK_MS = 70;
static const int FINGER_DECAY_MS = 120;

static inline int dxval(const struct MTFinger *a, const struct MTFinger *b)
{
	return a->hw.position_x - b->hw.position_x;
}
static inline int dyval(const struct MTFinger *a, const struct MTFinger *b)
{
	return a->hw.position_y - b->hw.position_y;
}

void init_memory(struct Memory *mem)
{
	memset(mem, 0, sizeof(struct Memory));
}

/**
 * update_configuration
 *
 * Update the same, fingers, added memory variables.
 *
 * Precondition: none
 *
 * Postcondition: same, fingers, added are set
 *
 */
static void update_configuration(struct Memory *m,
				 const struct MTState *prev_state,
				 const struct MTState *state)
{
	const struct MTFinger *f = state->finger;
	int i;
	m->same = state->nfinger == prev_state->nfinger;
	for (i = 0; i < state->nfinger; i++)
		m->same = m->same && find_finger(prev_state, f[i].id);
}

/**
 * update_pointers
 *
 * Update the pointing and ybar memory variables.
 *
 * Precondition: fingers, added are set
 *
 * Postcondition: pointing, ybar are set
 *
 */
static void update_pointers(struct Memory *m,
			    const struct MTState *state,
			    const struct Capabilities *caps)
{
	const struct MTFinger *f = state->finger;
	int i;

	if (state->nfinger < 2) {
		m->pointing = state->nfinger;
		m->ybar = caps->abs_position_y.maximum;
		return;
	}

	if (m->same) {
		for (i = 0; i < state->nfinger; i++) {
			if (GETBIT(m->pointing, i))
				continue;
			if (f[i].hw.position_y <= m->ybar) {
				m->pointing = BITONES(state->nfinger);
				return;
			}
		}
		return;
	}

	m->pointing = 0;
	m->ybar = caps->yclick;
	for (i = 0; i < state->nfinger; i++) {
		if (f[i].hw.position_y > caps->yclick)
			continue;
		if (!m->pointing || f[i].hw.position_y > m->ybar)
			m->ybar = f[i].hw.position_y;
		SETBIT(m->pointing, i);
	}

}

/**
 * update_movement
 *
 * Update the pending, moving, mvhold, mvforget, dx and dy memory variables.
 *
 * When moving is nonzero, gestures can be extracted from the dx and dy
 * variables. These variables should be cleared after use.
 *
 * Precondition: fingers, added, pointing are set
 *
 * Postcondition: pending, moving, mvhold, mvforget, dx, dy are set
 *
 */
static void update_movement(struct Memory *m,
			    const struct MTState *prev_state,
			    const struct MTState *state,
			    const struct Capabilities *caps)
{
	const struct MTFinger *prev, *f = state->finger;
	int i, x = 0, y = 0;
	int dx, dy, xcut, ycut, xmax = 0, ymax = 0;

	m->pending = 0;
	m->moving = 0;

	if (state->nfinger == 0)
		return;

	if (!m->same) {
		m->move_time = state->evtime;
		if (state->nfinger > prev_state->nfinger)
			m->move_time += FINGER_ATTACK_MS;
		else
			m->move_time += FINGER_DECAY_MS;
		memset(m->dx, 0, sizeof(m->dx));
		memset(m->dy, 0, sizeof(m->dy));
	} else {
		for (i = 0; i < state->nfinger; i++) {
			if (!GETBIT(m->pointing, i))
				continue;
			prev = find_finger(prev_state, f[i].id);
			dx = dxval(&f[i], prev);
			dy = dyval(&f[i], prev);
			m->dx[i] += dx;
			m->dy[i] += dy;
			xmax = maxval(xmax, abs(m->dx[i]));
			ymax = maxval(ymax, abs(m->dy[i]));
		}
		xcut = DELTA_CUT(xmax);
		ycut = DELTA_CUT(ymax);
		for (i = 0; i < state->nfinger; i++) {
			if (!GETBIT(m->pointing, i))
				continue;
			if (abs(m->dx[i]) > xcut ||
			    abs(m->dy[i]) > ycut)
				SETBIT(m->pending, i);
		}

		/* accumulate all movement during delay */
		if (m->pending && state->evtime >= m->move_time)
			m->moving = m->pending;
	}
}

void refresh_memory(struct Memory *m,
		    const struct MTState *prev_state,
		    const struct MTState *state,
		    const struct Capabilities *caps)
{
	update_configuration(m, prev_state, state);
	update_pointers(m, state, caps);
	update_movement(m, prev_state, state, caps);
}

void output_memory(const struct Memory *m)
{
	int i;
	xf86Msg(X_INFO, "btdata: %04x\n", m->btdata);
	xf86Msg(X_INFO, "pointing: %04x\n", m->pointing);
	xf86Msg(X_INFO, "pending: %04x\n", m->pending);
	xf86Msg(X_INFO, "moving: %04x\n", m->moving);
	xf86Msg(X_INFO, "ybar: %d\n", m->ybar);
	xf86Msg(X_INFO, "move_time: %lld\n", m->move_time);
}
