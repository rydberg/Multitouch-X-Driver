/***************************************************************************
 *
 * Multitouch X driver
 * Copyright (C) 2008 Henrik Rydberg <rydberg@euromail.se>
 *
 * Licensed under the Academic Free License version 2.1
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

#include "xorg-server.h"
#include <xf86.h>
#include <xf86_OSproc.h>
#include <xf86Xinput.h>
//#include <exevents.h>
#include <linux/input.h>
#include <errno.h>

////////////////////////////////////////////////////////////////////////////
//#define BTN_MT_REPORT_PACKET	0x210	/* report multitouch packet data */
//#define BTN_MT_REPORT_FINGER	0x211	/* report multitouch finger data */
#define BTN_TOOL_PRESS		0x148	/* The trackpad is a physical button */
#define BTN_MT_REPORT_PACKET	0x14b	/* multitouch device */
#define BTN_MT_REPORT_FINGER	0x14c	/* multitouch device */
#define BTN_TOOL_QUADTAP	0x14f	/* Four fingers on trackpad */

#define ABS_MT_TOUCH		0x30
#define ABS_MT_TOUCH_MAJOR	0x30
#define ABS_MT_TOUCH_MINOR	0x31
#define ABS_MT_WIDTH		0x32
#define ABS_MT_WIDTH_MAJOR	0x32
#define ABS_MT_WIDTH_MINOR	0x33
#define ABS_MT_ORIENTATION	0x34
#define ABS_MT_POSITION_X	0x35
#define ABS_MT_POSITION_Y	0x36

typedef int bool;

////////////////////////////////////////////////////////////////////////////

#define SYSCALL(call) while (((call) == -1) && (errno == EINTR))

static const int bits_per_long = 8 * sizeof(long);

static inline int nlongs(int nbit)
{
	return (nbit + bits_per_long - 1) / bits_per_long;
}

static inline bool getbit(const unsigned long* map, int key)
{
	return (map[key / bits_per_long] >> (key % bits_per_long)) & 0x01;
}

static bool getabs(struct input_absinfo *abs, int key, int fd)
{
	int rc;
	SYSCALL(rc = ioctl(fd, EVIOCGABS(key), abs));
	return rc >= 0;
}

#define SETABS(c, x, map, key, fd)					\
	c->has_##x = getbit(map, key) && getabs(&c->abs_##x, key, fd)

#define ADDCAP(s, c, x) strcat(s, c->has_##x ? " " #x : "")

////////////////////////////////////////////////////////////////////////////

struct Capabilities {
	bool has_left, has_middle;
	bool has_right, has_mtdata;
	bool has_touch_major, has_touch_minor;
	bool has_width_major, has_width_minor;
	bool has_orientation, has_dummy;
	bool has_position_x, has_position_y;
	struct input_absinfo abs_touch_major;
	struct input_absinfo abs_touch_minor;
	struct input_absinfo abs_width_major;
	struct input_absinfo abs_width_minor;
	struct input_absinfo abs_orientation;
	struct input_absinfo abs_position_x;
	struct input_absinfo abs_position_y;
};

////////////////////////////////////////////////////////////////////////////

static int read_capabilities(struct Capabilities *cap, int fd)
{
	unsigned long evbits[nlongs(EV_MAX)];
	unsigned long absbits[nlongs(ABS_MAX)];
	unsigned long keybits[nlongs(KEY_MAX)];
	int rc;

	memset(cap, 0, sizeof(struct Capabilities));
	
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
	cap->has_mtdata = getbit(keybits, BTN_MT_REPORT_PACKET);

	SETABS(cap, touch_major, absbits, ABS_MT_TOUCH_MAJOR, fd);
	SETABS(cap, touch_minor, absbits, ABS_MT_TOUCH_MINOR, fd);
	SETABS(cap, width_major, absbits, ABS_MT_WIDTH_MAJOR, fd);
	SETABS(cap, width_minor, absbits, ABS_MT_WIDTH_MINOR, fd);
	SETABS(cap, orientation, absbits, ABS_MT_ORIENTATION, fd);
	SETABS(cap, position_x, absbits, ABS_MT_POSITION_X, fd);
	SETABS(cap, position_y, absbits, ABS_MT_POSITION_Y, fd);
}

static int output_capabilities(const struct Capabilities *cap)
{
	char line[1024];
	memset(line, 0, sizeof(line));
	ADDCAP(line, cap, left);
	ADDCAP(line, cap, middle);
	ADDCAP(line, cap, right);
	ADDCAP(line, cap, mtdata);
	ADDCAP(line, cap, touch_major);
	ADDCAP(line, cap, touch_minor);
	ADDCAP(line, cap, width_major);
	ADDCAP(line, cap, width_minor);
	ADDCAP(line, cap, orientation);
	ADDCAP(line, cap, position_x);
	ADDCAP(line, cap, position_y);
	xf86Msg(X_INFO, "multitouch: caps:%s\n", line);
}

static InputInfoPtr preinit(InputDriverPtr drv, IDevPtr dev, int flags)
{
	InputInfoPtr local = xf86AllocateInput(drv, 0);
	if (!local)
		goto error;

	local->name = dev->identifier;
	local->type_name = XI_TOUCHPAD;
	local->device_control = 0;//DeviceControl;
	local->read_input = 0;//ReadInput;
	local->control_proc = 0;//ControlProc;
	local->close_proc = 0;//CloseProc;
	local->switch_mode = 0;//SwitchMode;
	local->conversion_proc = 0;//ConvertProc;
	local->reverse_conversion_proc = NULL;
	local->dev = NULL;
	local->private = 0;//priv;
	local->private_flags = 0;
	local->flags = XI86_POINTER_CAPABLE | XI86_SEND_DRAG_EVENTS;
	local->conf_idev = dev;
	//local->motion_history_proc = xf86GetMotionEvents;
	//local->history_size = 0;
	local->always_core_feedback = 0;

	xf86CollectInputOptions(local, NULL, NULL);
	xf86OptionListReport(local->options);

	local->fd = xf86OpenSerial(local->options);
	if (local->fd < 0) {
		xf86Msg(X_ERROR, "multitouch: cannot open device\n");
		goto error;
	}
	struct Capabilities cap;
	read_capabilities(&cap, local->fd);
	output_capabilities(&cap);
	xf86CloseSerial(local->fd);
	local->fd = -1;
	local->flags |= XI86_CONFIGURED;
 error:
	return local;
}

static void uninit(InputDriverPtr drv, InputInfoPtr local, int flags)
{
	xf86DeleteInput(local, 0);
}

////////////////////////////////////////////////////////////////////////////

static InputDriverRec MULTITOUCH = {
    1,
    "multitouch",
    NULL,
    preinit,
    uninit,
    NULL,
    0
};

static XF86ModuleVersionInfo VERSION = {
    "multitouch",
    MODULEVENDORSTRING,
    MODINFOSTRING1,
    MODINFOSTRING2,
    XORG_VERSION_CURRENT,
    0, 1, 0,
    ABI_CLASS_XINPUT,
    ABI_XINPUT_VERSION,
    MOD_CLASS_XINPUT,
    {0, 0, 0, 0}
};

static pointer setup(pointer module, pointer options, int *errmaj, int *errmin)
{
    xf86AddInputDriver(&MULTITOUCH, module, 0);
    return module;
}

XF86ModuleData multitouchModuleData = {&VERSION, &setup, NULL };

////////////////////////////////////////////////////////////////////////////
