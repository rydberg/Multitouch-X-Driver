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

#ifndef GESTURES_H
#define GESTURES_H

#include "mtouch.h"

#define GS_BUTTON 0
#define GS_MOVE 1
#define GS_VSCROLL 2
#define GS_HSCROLL 3
#define GS_VSWIPE 4
#define GS_HSWIPE 5
#define GS_SCALE 6
#define GS_ROTATE 7

struct Gestures {
	unsigned type, btmask, btdata;
	int same_fingers, dx, dy, scale, rot;
};

void extract_gestures(struct Gestures *gs, struct MTouch* mt);

#endif
