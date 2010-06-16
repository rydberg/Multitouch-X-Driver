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

#ifndef COMMON_H
#define COMMON_H

#include "xorg-server.h"
#include <xf86.h>
#include <xf86_OSproc.h>
#include <xf86Xinput.h>
#include <linux/input.h>
#include <errno.h>

/* includes available in 2.6.30-rc5 */

#ifndef BTN_TOOL_QUADTAP
#define BTN_TOOL_QUADTAP	0x14f	/* Four fingers on trackpad */
#define ABS_MT_TOUCH_MAJOR	0x30	/* Major axis of touching ellipse */
#define ABS_MT_TOUCH_MINOR	0x31	/* Minor axis (omit if circular) */
#define ABS_MT_WIDTH_MAJOR	0x32	/* Major axis of approaching ellipse */
#define ABS_MT_WIDTH_MINOR	0x33	/* Minor axis (omit if circular) */
#define ABS_MT_ORIENTATION	0x34	/* Ellipse orientation */
#define ABS_MT_POSITION_X	0x35	/* Center X ellipse position */
#define ABS_MT_POSITION_Y	0x36	/* Center Y ellipse position */
#define ABS_MT_TOOL_TYPE	0x37	/* Type of touching device */
#define ABS_MT_BLOB_ID		0x38	/* Group a set of packets as a blob */
#define ABS_MT_TRACKING_ID	0x39	/* Unique ID of initiated contact */
#define SYN_MT_REPORT		2
#define MT_TOOL_FINGER		0
#define MT_TOOL_PEN		1
#endif

/* includes available in 2.6.33 */
#ifndef ABS_MT_PRESSURE
#define ABS_MT_PRESSURE		0x3a	/* Pressure on contact area */
#endif

/* includes available in 2.6.36 */
#ifndef ABS_MT_SLOT
#define ABS_MT_SLOT		0x2f	/* MT slot being modified */
#define MT_ABS_SIZE		11	/* Size of MT_SLOT_ABS_EVENTS */
#define MT_SLOT_ABS_EVENTS {	\
	ABS_MT_TOUCH_MAJOR,	\
	ABS_MT_TOUCH_MINOR,	\
	ABS_MT_WIDTH_MAJOR,	\
	ABS_MT_WIDTH_MINOR,	\
	ABS_MT_ORIENTATION,	\
	ABS_MT_POSITION_X,	\
	ABS_MT_POSITION_Y,	\
	ABS_MT_TOOL_TYPE,	\
	ABS_MT_BLOB_ID,		\
	ABS_MT_TRACKING_ID,	\
	ABS_MT_PRESSURE,	\
}
#endif

#define DIM_FINGER 32
#define DIM2_FINGER (DIM_FINGER * DIM_FINGER)

/* event buffer size (must be a power of two) */
#define DIM_EVENTS 64

/* year-proof millisecond event time */
typedef __u64 mstime_t;

/* all bit masks have this type */
typedef unsigned int bitmask_t;

#define BITMASK(x) (1U << (x))
#define BITONES(x) (BITMASK(x) - 1U)
#define GETBIT(m, x) (((m) >> (x)) & 1U)
#define SETBIT(m, x) (m |= BITMASK(x))
#define CLEARBIT(m, x) (m &= ~BITMASK(x))
#define MODBIT(m, x, b) ((b) ? SETBIT(m, x) : CLEARBIT(m, x))

static inline int maxval(int x, int y) { return x > y ? x : y; }
static inline int minval(int x, int y) { return x < y ? x : y; }

static inline int clamp15(int x)
{
	return x < -32767 ? -32767 : x > 32767 ? 32767 : x;
}

/* absolute scale is assumed to fit in 15 bits */
static inline int dist2(int dx, int dy)
{
	dx = clamp15(dx);
	dy = clamp15(dy);
	return dx * dx + dy * dy;
}

/* Count number of bits (Sean Eron Andersson's Bit Hacks) */
static inline int bitcount(unsigned v)
{
	v -= ((v>>1) & 0x55555555);
	v = (v&0x33333333) + ((v>>2) & 0x33333333);
	return (((v + (v>>4)) & 0xF0F0F0F) * 0x1010101) >> 24;
}

/* Return index of first bit [0-31], -1 on zero */
#define firstbit(v) (__builtin_ffs(v) - 1)

/* boost-style foreach bit */
#define foreach_bit(i, m)						\
	for (i = firstbit(m); i >= 0; i = firstbit((m) & (~0U << i + 1)))

/* robust system ioctl calls */
#define SYSCALL(call) while (((call) == -1) && (errno == EINTR))

#endif
