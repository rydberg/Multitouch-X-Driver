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

/* click area definition */
#define CLICK_AREA(c) ((c->has_ibt ? 0.20 : 0.00) * get_cap_ysize(c))

/* fraction of max movement threshold */
#define DELTA_CUT(x) (0.5 * (x))

/* fraction of tap movement threshold */
#define TAP_XMOVE(c) (0.05 * get_cap_xsize(c))
#define TAP_YMOVE(c) (0.05 * get_cap_ysize(c))

/* timer for cursor stability on finger touch/release */
static const int FINGER_ATTACK_MS = 40;
static const int FINGER_DECAY_MS = 120;
static const int FINGER_CORNER_MS = 300;

/* tapping timings */
static const int TAP_SETTLE_MS = 400;
static const int TAP_TOUCH_MS = 120;
static const int TAP_GAP_MS = 200;

static inline int dxval(const struct FingerState *a,
			const struct FingerState *b)
{
	return a->position_x - b->position_x;
}
static inline int dyval(const struct FingerState *a,
			const struct FingerState *b)
{
	return a->position_y - b->position_y;
}

static inline void relax_tapping(struct Memory *m, const struct MTState *state)
{
	m->tprelax = state->evtime + TAP_SETTLE_MS;
	m->wait = 0;
	m->ntap = 0;
}

static inline int unrelated_taps(const struct Memory *m,
				 const struct Capabilities *caps)
{
	return abs(m->xdown - m->xup) > TAP_XMOVE(caps) ||
		abs(m->ydown - m->yup) > TAP_YMOVE(caps);
}

/**
 * init_memory
 */
void init_memory(struct Memory *mem)
{
	memset(mem, 0, sizeof(struct Memory));
}

/**
 * update_configuration
 *
 * Update the same, fingers, added, and thumb memory variables.
 *
 * Precondition: none
 *
 * Postcondition: same, fingers, added, thumb are set
 *
 */
static void update_configuration(struct Memory *m,
				 const struct MTState *prev_state,
				 const struct MTState *state)
{
	const struct FingerState *f = state->finger;
	bitmask_t fingers = BITONES(state->nfinger);
	int i;
	m->added = 0;
	foreach_bit(i, fingers)
		if (!find_finger(prev_state, f[i].tracking_id))
			SETBIT(m->added, i);
	m->same = m->fingers == fingers && m->added == 0;
	m->fingers = fingers;
	if (!m->same)
		m->thumb = 0;
	m->thumb |= state->thumb;
}

/**
 * update_pointers
 *
 * Update the pointing and ybar memory variables.
 *
 * Precondition: fingers, added, thumb are set
 *
 * Postcondition: pointing, ybar are set
 *
 */
static void update_pointers(struct Memory *m,
			    const struct MTState *state,
			    const struct Capabilities *caps)
{
	const struct FingerState *f = state->finger;
	int yclick = caps->abs[BIT_POSITION_Y].maximum - CLICK_AREA(caps);

	int i;

	if (state->nfinger < 2) {
		m->pointing = m->fingers;
		m->ybar = caps->abs[BIT_POSITION_Y].maximum;
		return;
	}

	if (m->same) {
		foreach_bit(i, m->fingers & ~m->pointing) {
			if (f[i].position_y <= m->ybar) {
				m->pointing = m->fingers;
				return;
			}
		}
		return;
	}

	m->pointing = 0;
	m->ybar = yclick;
	foreach_bit(i, m->fingers) {
		if (f[i].position_y > yclick)
			continue;
		if (!m->pointing || f[i].position_y > m->ybar)
			m->ybar = f[i].position_y;
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
 * Precondition: fingers, added, thumb, pointing are set
 *
 * Postcondition: pending, moving, mvhold, mvforget, dx, dy are set
 *
 */
static void update_movement(struct Memory *m,
			    const struct MTState *prev_state,
			    const struct MTState *state,
			    const struct Capabilities *caps)
{
	const struct FingerState *prev, *f = state->finger;
	int i, xcut, ycut, xmax = 0, ymax = 0;

	m->moving = 0;
	m->pending = 0;

	if (!m->same) {
		mstime_t d2, d2max = center_maxdist2(caps);
		mem_hold_movement(m, state->evtime + FINGER_ATTACK_MS);
		if (!m->added)
			mem_hold_movement(m, state->evtime + FINGER_DECAY_MS);
		foreach_bit(i, m->added) {
			d2 = center_dist2(&f[i], caps);
			d2 *= FINGER_CORNER_MS - FINGER_ATTACK_MS;
			d2 /= d2max;
			m->mvhold += d2;
		}
		memset(m->dx, 0, sizeof(m->dx));
		memset(m->dy, 0, sizeof(m->dy));
		return;
	}

	if (state->evtime < m->mvforget)
		return;

	foreach_bit(i, m->pointing) {
		int dx, dy;
		prev = find_finger(prev_state, f[i].tracking_id);
		dx = dxval(&f[i], prev);
		dy = dyval(&f[i], prev);
		m->dx[i] += dx;
		m->dy[i] += dy;
		xmax = maxval(xmax, abs(m->dx[i]));
		ymax = maxval(ymax, abs(m->dy[i]));
	}
	xcut = DELTA_CUT(xmax);
	ycut = DELTA_CUT(ymax);
	foreach_bit(i, m->pointing)
		if (abs(m->dx[i]) > xcut || abs(m->dy[i]) > ycut)
			SETBIT(m->pending, i);

	if (state->evtime < m->mvhold)
		return;

	m->moving = m->pending;
}

/**
 * update_tapping
 *
 * Update tpdown, tpup, tprelax, wait, maxtap, ntap.
 *
 * Precondition: pointing and thumb are set
 *
 * Postcondition: tpdown, tpup, tprelax, wait, maxtap, ntap set
 *
 */
static void update_tapping(struct Memory *m,
			   const struct MTState *prev_state,
			   const struct MTState *state,
			   const struct Capabilities *caps)
{
	int x, y, i, npoint = bitcount(m->pointing);

	/* use a position independent on the number of fingers */
	if (state->nfinger) {
		x = state->finger[0].position_x;
		y = state->finger[0].position_y;
		for (i = 1; i < state->nfinger; i++) {
			x = minval(x, state->finger[i].position_x);
			y = minval(y, state->finger[i].position_y);
		}
	}

	if (m->thumb || state->evtime < m->mvforget) {
		relax_tapping(m, state);
	} else if (state->nfinger && !prev_state->nfinger) {
		m->tpdown = state->evtime;
		m->xdown = x;
		m->ydown = y;
		m->wait = 0;
		m->maxtap = npoint;
		if (m->tpdown > m->tpup + TAP_GAP_MS)
			m->ntap = 0;
		else if (unrelated_taps(m, caps))
			relax_tapping(m, state);
		m->xup = x;
		m->yup = y;
	} else if (!state->nfinger && prev_state->nfinger) {
		m->tpup = state->evtime;
		if (m->tpup > m->tprelax &&
		    m->tpup <= m->tpdown + TAP_TOUCH_MS) {
			m->wait = TAP_GAP_MS;
			m->ntap++;
			if (unrelated_taps(m, caps))
				relax_tapping(m, state);
		}
	} else if (state->nfinger) {
		m->xup = minval(m->xup, x);
		m->yup = minval(m->yup, y);
		m->maxtap = maxval(m->maxtap, npoint);
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
	update_tapping(m, prev_state, state, caps);
}

void output_memory(const struct Memory *m)
{
	int i;
	xf86Msg(X_INFO, "btdata: %04x\n", m->btdata);
	xf86Msg(X_INFO, "fingers: %04x\n", m->fingers);
	xf86Msg(X_INFO, "added: %04x\n", m->added);
	xf86Msg(X_INFO, "pointing: %04x\n", m->pointing);
	xf86Msg(X_INFO, "pending: %04x\n", m->pending);
	xf86Msg(X_INFO, "moving: %04x\n", m->moving);
	xf86Msg(X_INFO, "ybar: %d\n", m->ybar);
}
