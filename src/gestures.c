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

static const int FINGER_THUMB_MS = 600;
static const int BUTTON_HOLD_MS = 200;

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
	bitmask_t btdata = mt->state.button & BITONES(DIM_BUTTON);
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
	} else if (btdata == 0 && mt->mem.ntap) {
		if (npoint == 1 && mt->mem.maxtap == 1)
			btdata = BITMASK(MT_BUTTON_LEFT);
		gs->btmask = (btdata ^ mt->mem.btdata) & BITONES(DIM_BUTTON);
		gs->btdata = btdata;
		mt->mem.btdata = btdata;
	}
	if (gs->btmask) {
		mt_delay_movement(mt, BUTTON_HOLD_MS);
		SETBIT(gs->type, GS_BUTTON);
	}
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
	float xp[DIM_FINGER], yp[DIM_FINGER];
	float xm[DIM_FINGER], ym[DIM_FINGER];
	float xpos = 0, ypos = 0;
	float move, xmove = 0, ymove = 0;
	float rad, rad2 = 0, scale = 0, rot = 0;


	if (!nmove || nmove != npoint)
		return;

	foreach_bit(i, mt->mem.moving) {
		xp[i] = mt->state.finger[i].position_x - xpos;
		yp[i] = mt->state.finger[i].position_y - ypos;
		xm[i] = mt->mem.dx[i];
		ym[i] = mt->mem.dy[i];
		mt->mem.dx[i] = 0;
		mt->mem.dy[i] = 0;
		xpos += xp[i];
		ypos += yp[i];
		xmove += xm[i];
		ymove += ym[i];
	}
	xpos /= nmove;
	ypos /= nmove;
	xmove /= nmove;
	ymove /= nmove;
	move = sqrt(xmove * xmove + ymove * ymove);

	if (nmove == 1) {
		if (mt->mem.moving & mt->mem.thumb) {
			mt_skip_movement(mt, FINGER_THUMB_MS);
			return;
		}
		gs->dx = xmove;
		gs->dy = ymove;
		if (gs->dx || gs->dy)
			SETBIT(gs->type, GS_MOVE);
		return;
	}

	foreach_bit(i, mt->mem.moving) {
		xp[i] -= xpos;
		yp[i] -= ypos;
		rad2 += xp[i] * xp[i];
		rad2 += yp[i] * yp[i];
		scale += xp[i] * xm[i];
		scale += yp[i] * ym[i];
		rot += xp[i] * ym[i];
		rot -= yp[i] * xm[i];
	}
	rad2 /= nmove;
	scale /= nmove;
	rot /= nmove;
	rad = sqrt(rad2);
	scale /= rad;
	rot /= rad;

	if (abs(rot) > move && abs(rot) > abs(scale)) {
		gs->rot = rot;
		if (gs->rot) {
			if (nmove == 2)
				SETBIT(gs->type, GS_ROTATE);
		}
	} else if (abs(scale) > move) {
		gs->scale = scale;
		if (gs->scale) {
			if (nmove == 2)
				SETBIT(gs->type, GS_SCALE);
		}
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

/**
 * extract_delayed_gestures
 *
 * Extract delayed gestures, such as tapping
 *
 * Reset memory after use.
 *
 */
void extract_delayed_gestures(struct Gestures *gs, struct MTouch* mt)
{
	mt->mem.wait = 0;

	if (mt->mem.tpdown < mt->mem.tpup) {
		switch (mt->mem.maxtap) {
		case 1:
			gs->tapmask = BITMASK(MT_BUTTON_LEFT);
			break;
		case 2:
			gs->tapmask = BITMASK(MT_BUTTON_RIGHT);
			break;
		case 3:
			gs->tapmask = BITMASK(MT_BUTTON_MIDDLE);
			break;
		}
	}

	if (gs->tapmask)
		SETBIT(gs->type, GS_TAP);

	gs->ntap = mt->mem.ntap;
}

void output_gesture(const struct Gestures *gs)
{
	int i;
	foreach_bit(i, gs->btmask)
		xf86Msg(X_INFO, "button bit: %d %d\n",
			i, GETBIT(gs->btdata, i));
	if (GETBIT(gs->type, GS_MOVE))
		xf86Msg(X_INFO, "motion: %d %d\n", gs->dx, gs->dy);
	if (GETBIT(gs->type, GS_VSCROLL))
		xf86Msg(X_INFO, "vscroll: %d\n", gs->dy);
	if (GETBIT(gs->type, GS_HSCROLL))
		xf86Msg(X_INFO, "hscroll: %d\n", gs->dx);
	if (GETBIT(gs->type, GS_VSWIPE))
		xf86Msg(X_INFO, "vswipe: %d\n", gs->dy);
	if (GETBIT(gs->type, GS_HSWIPE))
		xf86Msg(X_INFO, "hswipe: %d\n", gs->dx);
	if (GETBIT(gs->type, GS_SCALE))
		xf86Msg(X_INFO, "scale: %d\n", gs->scale);
	if (GETBIT(gs->type, GS_ROTATE))
		xf86Msg(X_INFO, "rotate: %d\n", gs->rot);
	foreach_bit(i, gs->tapmask)
		xf86Msg(X_INFO, "tap: %d %d\n", i, gs->ntap);
}
