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

/******************************************************/

void extract_gestures(struct Gestures *gs, struct MTouch* mt)
{
	const struct FingerState *b = mt->ns.finger;
	const struct FingerState *e = b + mt->ns.nfinger;
	const struct FingerState *p, *fs;
	int nof = count_fingers(&mt->os);
	int nsf = count_fingers(&mt->ns);
	int dn = 0, i;
	memset(gs, 0, sizeof(struct Gestures));
	if (nof == nsf) {
		for (p = b; p != e; p++) {
			fs = find_finger(&mt->os, p->id);
			if (fs) {
				gs->dx += p->hw.position_x - fs->hw.position_x;
				gs->dy += p->hw.position_y - fs->hw.position_y;
				dn++;
			}
		}
	}
	if (gs->dx || gs->dy) {
		gs->dx /= dn;
		gs->dy /= dn;
		if (nsf == 1)
			SETBIT(gs->type, GS_MOVE);
		if (nsf == 2)
			SETBIT(gs->type, GS_VSCROLL);
		if (nsf == 3)
			SETBIT(gs->type, GS_HSCROLL);
	}
	if (mt->ns.button == BITMASK(MT_BUTTON_LEFT)) {
		if (nsf == 2)
			mt->ns.button = BITMASK(MT_BUTTON_RIGHT);
		if (nsf == 3)
			mt->ns.button = BITMASK(MT_BUTTON_MIDDLE);
	}
   gs->btmask = (mt->ns.button ^ mt->os.button) & BITONES(DIM_BUTTON);
   gs->btdata = mt->ns.button & BITONES(DIM_BUTTON);
	mt->os = mt->ns;
}

