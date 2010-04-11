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

#include "capabilities.h"

#define SETABS(c, x, map, key, fd)					\
	(c->has_##x = getbit(map, key) && getabs(&c->abs_##x, key, fd))

#define ADDCAP(s, c, x) strcat(s, c->has_##x ? " " #x : "")

static const int SN_COORD = 250;	/* coordinate signal-to-noise ratio */

static const int bits_per_long = 8 * sizeof(long);

static inline int nlongs(int nbit)
{
	return (nbit + bits_per_long - 1) / bits_per_long;
}

static inline int getbit(const unsigned long *map, int key)
{
	return (map[key / bits_per_long] >> (key % bits_per_long)) & 0x01;
}

static int getabs(struct input_absinfo *abs, int key, int fd)
{
	int rc;
	SYSCALL(rc = ioctl(fd, EVIOCGABS(key), abs));
	return rc >= 0;
}

static int has_integrated_button(const struct Capabilities *cap)
{
	static const int bcm5974_vmask_ibt = 1;
	if (strcmp(cap->devname, "bcm5974"))
		return 0;
	return cap->devid.version & bcm5974_vmask_ibt;
}

int read_capabilities(struct Capabilities *cap, int fd)
{
	unsigned long evbits[nlongs(EV_MAX)];
	unsigned long absbits[nlongs(ABS_MAX)];
	unsigned long keybits[nlongs(KEY_MAX)];
	int rc;

	memset(cap, 0, sizeof(struct Capabilities));

	SYSCALL(rc = ioctl(fd, EVIOCGID, &cap->devid));
	if (rc < 0)
		return rc;
	SYSCALL(rc = ioctl(fd, EVIOCGNAME(sizeof(cap->devname)), cap->devname));
	if (rc < 0)
		return rc;
	SYSCALL(rc = ioctl(fd, EVIOCGBIT(EV_SYN, sizeof(evbits)), evbits));
	if (rc < 0)
		return rc;
	SYSCALL(rc = ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(keybits)), keybits));
	if (rc < 0)
		return rc;
	SYSCALL(rc = ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(absbits)), absbits));
	if (rc < 0)
		return rc;

	cap->has_left = getbit(keybits, BTN_LEFT);
	cap->has_middle = getbit(keybits, BTN_MIDDLE);
	cap->has_right = getbit(keybits, BTN_RIGHT);

	SETABS(cap, touch_major, absbits, ABS_MT_TOUCH_MAJOR, fd);
	SETABS(cap, touch_minor, absbits, ABS_MT_TOUCH_MINOR, fd);
	SETABS(cap, width_major, absbits, ABS_MT_WIDTH_MAJOR, fd);
	SETABS(cap, width_minor, absbits, ABS_MT_WIDTH_MINOR, fd);
	SETABS(cap, orientation, absbits, ABS_MT_ORIENTATION, fd);
	SETABS(cap, position_x, absbits, ABS_MT_POSITION_X, fd);
	SETABS(cap, position_y, absbits, ABS_MT_POSITION_Y, fd);

	cap->has_mtdata = cap->has_position_x && cap->has_position_y;
	cap->has_ibt = has_integrated_button(cap);

	cap->xfuzz = cap->abs_position_x.fuzz;
	cap->yfuzz = cap->abs_position_y.fuzz;
	if (cap->xfuzz <= 0 || cap->yfuzz <= 0) {
		cap->xfuzz = get_cap_xsize(cap) / SN_COORD;
		cap->yfuzz = get_cap_ysize(cap) / SN_COORD;
	}

	return 0;
}

int get_cap_xsize(const struct Capabilities *cap)
{
	return cap->abs_position_x.maximum - cap->abs_position_x.minimum;
}

int get_cap_ysize(const struct Capabilities *cap)
{
	return cap->abs_position_y.maximum - cap->abs_position_y.minimum;
}

void output_capabilities(const struct Capabilities *cap)
{
	char line[1024];
	memset(line, 0, sizeof(line));
	ADDCAP(line, cap, left);
	ADDCAP(line, cap, middle);
	ADDCAP(line, cap, right);
	ADDCAP(line, cap, mtdata);
	ADDCAP(line, cap, ibt);
	ADDCAP(line, cap, touch_major);
	ADDCAP(line, cap, touch_minor);
	ADDCAP(line, cap, width_major);
	ADDCAP(line, cap, width_minor);
	ADDCAP(line, cap, orientation);
	ADDCAP(line, cap, position_x);
	ADDCAP(line, cap, position_y);
	xf86Msg(X_INFO, "multitouch: devname: %s\n", cap->devname);
	xf86Msg(X_INFO, "multitouch: devid: %x %x %x\n",
		cap->devid.vendor, cap->devid.product, cap->devid.version);
	xf86Msg(X_INFO, "multitouch: caps:%s\n", line);
	if (cap->has_touch_major)
		xf86Msg(X_INFO, "multitouch: touch: %d %d\n",
			cap->abs_touch_major.minimum,
			cap->abs_touch_major.maximum);
	if (cap->has_width_major)
		xf86Msg(X_INFO, "multitouch: width: %d %d\n",
			cap->abs_width_major.minimum,
			cap->abs_width_major.maximum);
	if (cap->has_orientation)
		xf86Msg(X_INFO, "multitouch: orientation: %d %d\n",
			cap->abs_orientation.minimum,
			cap->abs_orientation.maximum);
	if (cap->has_position_x)
		xf86Msg(X_INFO, "multitouch: position_x: %d %d\n",
			cap->abs_position_x.minimum,
			cap->abs_position_x.maximum);
	if (cap->has_position_y)
		xf86Msg(X_INFO, "multitouch: position_y: %d %d\n",
			cap->abs_position_y.minimum,
			cap->abs_position_y.maximum);
}
