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

#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) >= 7
#include <X11/Xatom.h>
#include <xserver-properties.h>
#endif

#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) >= 12
typedef InputInfoPtr LocalDevicePtr;
#endif

/* these should be user-configurable at some point */
static const float vscroll_fraction = 0.025;
static const float hscroll_fraction = 0.05;
static const float vswipe_fraction = 0.25;
static const float hswipe_fraction = 0.25;
static const float scale_fraction = 0.05;
static const float rot_fraction = 0.05;

/* button mapping simplified */
#define PROPMAP(m, x, y) m[x] = XIGetKnownProperty(y)

static void pointer_control(DeviceIntPtr dev, PtrCtrl *ctrl)
{
	xf86Msg(X_INFO, "pointer_control\n");
}

static int pointer_property(DeviceIntPtr dev,
			    Atom property,
			    XIPropertyValuePtr prop,
			    BOOL checkonly)
{
	xf86Msg(X_INFO, "pointer_property\n");
	return Success;
}

#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) >= 7
static void initAxesLabels(Atom map[2])
{
	memset(map, 0, 2 * sizeof(Atom));
	PROPMAP(map, 0, AXIS_LABEL_PROP_REL_X);
	PROPMAP(map, 1, AXIS_LABEL_PROP_REL_Y);
}

static void initButtonLabels(Atom map[DIM_BUTTON])
{
	memset(map, 0, DIM_BUTTON * sizeof(Atom));
	PROPMAP(map, MT_BUTTON_LEFT, BTN_LABEL_PROP_BTN_LEFT);
	PROPMAP(map, MT_BUTTON_MIDDLE, BTN_LABEL_PROP_BTN_MIDDLE);
	PROPMAP(map, MT_BUTTON_RIGHT, BTN_LABEL_PROP_BTN_RIGHT);
	PROPMAP(map, MT_BUTTON_WHEEL_UP, BTN_LABEL_PROP_BTN_WHEEL_UP);
	PROPMAP(map, MT_BUTTON_WHEEL_DOWN, BTN_LABEL_PROP_BTN_WHEEL_DOWN);
	PROPMAP(map, MT_BUTTON_HWHEEL_LEFT, BTN_LABEL_PROP_BTN_HWHEEL_LEFT);
	PROPMAP(map, MT_BUTTON_HWHEEL_RIGHT, BTN_LABEL_PROP_BTN_HWHEEL_RIGHT);
	/* how to map swipe buttons? */
	PROPMAP(map, MT_BUTTON_SWIPE_UP, BTN_LABEL_PROP_BTN_0);
	PROPMAP(map, MT_BUTTON_SWIPE_DOWN, BTN_LABEL_PROP_BTN_1);
	PROPMAP(map, MT_BUTTON_SWIPE_LEFT, BTN_LABEL_PROP_BTN_2);
	PROPMAP(map, MT_BUTTON_SWIPE_RIGHT, BTN_LABEL_PROP_BTN_3);
	/* how to map scale and rotate? */
	PROPMAP(map, MT_BUTTON_SCALE_DOWN, BTN_LABEL_PROP_BTN_4);
	PROPMAP(map, MT_BUTTON_SCALE_UP, BTN_LABEL_PROP_BTN_5);
	PROPMAP(map, MT_BUTTON_ROTATE_LEFT, BTN_LABEL_PROP_BTN_6);
	PROPMAP(map, MT_BUTTON_ROTATE_RIGHT, BTN_LABEL_PROP_BTN_7);
}
#endif

static int device_init(DeviceIntPtr dev, LocalDevicePtr local)
{
	struct MTouch *mt = local->private;
	unsigned char btmap[DIM_BUTTON + 1] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
	};
#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) >= 7
	Atom axes_labels[2], btn_labels[DIM_BUTTON];
	initAxesLabels(axes_labels);
	initButtonLabels(btn_labels);
#endif

	local->fd = xf86OpenSerial(local->options);
	if (local->fd < 0) {
		xf86Msg(X_ERROR, "multitouch: cannot open device\n");
		return !Success;
	}
	if (configure_mtouch(mt, local->fd)) {
		xf86Msg(X_ERROR, "multitouch: cannot configure device\n");
		return !Success;
	}
	xf86CloseSerial(local->fd);

#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) < 3
	InitPointerDeviceStruct((DevicePtr)dev,
				btmap, DIM_BUTTON,
				GetMotionHistory,
				pointer_control,
				GetMotionHistorySize(),
				2);
#elif GET_ABI_MAJOR(ABI_XINPUT_VERSION) < 7
	InitPointerDeviceStruct((DevicePtr)dev,
				btmap, DIM_BUTTON,
				pointer_control,
				GetMotionHistorySize(),
				2);
#elif GET_ABI_MAJOR(ABI_XINPUT_VERSION) >= 7
	InitPointerDeviceStruct((DevicePtr)dev,
				btmap, DIM_BUTTON, btn_labels,
				pointer_control,
				GetMotionHistorySize(),
				2, axes_labels);
#else
#error "Unsupported ABI_XINPUT_VERSION"
#endif

	xf86InitValuatorAxisStruct(dev, 0,
#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) >= 7
				   axes_labels[0],
#endif
				   mt->caps.abs[MTDEV_POSITION_X].minimum,
				   mt->caps.abs[MTDEV_POSITION_X].maximum,
#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) >= 12
				   1, 0, 1, Absolute);
#else
				   1, 0, 1);
#endif
	xf86InitValuatorDefaults(dev, 0);
	xf86InitValuatorAxisStruct(dev, 1,
#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) >= 7
				   axes_labels[1],
#endif
				   mt->caps.abs[MTDEV_POSITION_Y].minimum,
				   mt->caps.abs[MTDEV_POSITION_Y].maximum,
#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) >= 12
				   1, 0, 1, Absolute);
#else
				   1, 0, 1);
#endif
	xf86InitValuatorDefaults(dev, 1);

	XIRegisterPropertyHandler(dev, pointer_property, NULL, NULL);

	return Success;
}

static int device_on(LocalDevicePtr local)
{
	struct MTouch *mt = local->private;
	local->fd = xf86OpenSerial(local->options);
	if (local->fd < 0) {
		xf86Msg(X_ERROR, "multitouch: cannot open device\n");
		return !Success;
	}
	if (open_mtouch(mt, local->fd)) {
		xf86Msg(X_ERROR, "multitouch: cannot grab device\n");
		return !Success;
	}
	xf86AddEnabledDevice(local);
	return Success;
}

static int device_off(LocalDevicePtr local)
{
	struct MTouch *mt = local->private;
	xf86RemoveEnabledDevice(local);
	if (close_mtouch(mt, local->fd))
		xf86Msg(X_WARNING, "multitouch: cannot ungrab device\n");
	xf86CloseSerial(local->fd);
	return Success;
}

static int device_close(LocalDevicePtr local)
{
	return Success;
}

static void tickle_button(LocalDevicePtr local, int id)
{
	xf86PostButtonEvent(local->dev, FALSE, id, 1, 0, 0);
	xf86PostButtonEvent(local->dev, FALSE, id, 0, 0, 0);
}

static void button_scroll(LocalDevicePtr local,
			  int btdec, int btinc,
			  int *scroll, int step,
			  int delta)
{
	*scroll += delta;
	while (*scroll > step) {
		tickle_button(local, btinc);
		*scroll -= step;
	}
	while (*scroll < -step) {
		tickle_button(local, btdec);
		*scroll += step;
	}
}

static void handle_gestures(LocalDevicePtr local,
			    const struct Gestures *gs,
			    const struct Capabilities *caps)
{
	static int vscroll, hscroll, vswipe, hswipe, scale, rot;
	int i;
	if (!gs->same_fingers) {
		vscroll = 0;
		hscroll = 0;
		vswipe = 0;
		hswipe = 0;
	}
	foreach_bit(i, gs->btmask)
		xf86PostButtonEvent(local->dev, FALSE,
				    i + 1, GETBIT(gs->btdata, i), 0, 0);
	if (GETBIT(gs->type, GS_MOVE))
		xf86PostMotionEvent(local->dev, 0, 0, 2, gs->dx, gs->dy);

	if (GETBIT(gs->type, GS_VSCROLL)) {
		int step = 1 + vscroll_fraction * get_cap_ysize(caps);
		button_scroll(local, 4, 5, &vscroll, step, gs->dy);
	}
	if (GETBIT(gs->type, GS_HSCROLL)) {
		int step = 1 + hscroll_fraction * get_cap_xsize(caps);
		button_scroll(local, 6, 7, &hscroll, step, gs->dx);
	}
	if (GETBIT(gs->type, GS_VSWIPE)) {
		int step = 1 + vswipe_fraction * get_cap_ysize(caps);
		button_scroll(local, 8, 9, &vswipe, step, gs->dy);
	}
	if (GETBIT(gs->type, GS_HSWIPE)) {
		int step = 1 + hswipe_fraction * get_cap_xsize(caps);
		button_scroll(local, 10, 11, &hswipe, step, gs->dx);
	}
	if (GETBIT(gs->type, GS_SCALE)) {
		int step = 1 + scale_fraction * get_cap_xsize(caps);
		button_scroll(local, 12, 13, &scale, step, gs->scale);
	}
	if (GETBIT(gs->type, GS_ROTATE)) {
		int step = 1 + rot_fraction * get_cap_xsize(caps);
		button_scroll(local, 14, 15, &rot, step, gs->rot);
	}
	if (GETBIT(gs->type, GS_TAP) && gs->ntap == 1) {
		foreach_bit(i, gs->tapmask)
			tickle_button(local, i + 1);
	}
	if (GETBIT(gs->type, GS_VSWIPE4)) {
		int step = 1 + vswipe_fraction * get_cap_ysize(caps);
		button_scroll(local, 16, 17, &vswipe, step, gs->dy);
	}
	if (GETBIT(gs->type, GS_HSWIPE4)) {
		int step = 1 + hswipe_fraction * get_cap_xsize(caps);
		button_scroll(local, 18, 19, &hswipe, step, gs->dx);
	}
}

/* called for each full received packet from the touchpad */
static void read_input(LocalDevicePtr local)
{
	struct Gestures gs;
	struct MTouch *mt = local->private;
	while (read_packet(mt, local->fd) > 0) {
		extract_gestures(&gs, mt);
		handle_gestures(local, &gs, &mt->caps);
	}
	if (has_delayed_gestures(mt, local->fd)) {
		extract_delayed_gestures(&gs, mt);
		handle_gestures(local, &gs, &mt->caps);
	}
}

static Bool device_control(DeviceIntPtr dev, int mode)
{
	LocalDevicePtr local = dev->public.devicePrivate;
	switch (mode) {
	case DEVICE_INIT:
		xf86Msg(X_INFO, "device control: init\n");
		return device_init(dev, local);
	case DEVICE_ON:
		xf86Msg(X_INFO, "device control: on\n");
		return device_on(local);
	case DEVICE_OFF:
		xf86Msg(X_INFO, "device control: off\n");
		return device_off(local);
	case DEVICE_CLOSE:
		xf86Msg(X_INFO, "device control: close\n");
		return device_close(local);
	default:
		xf86Msg(X_INFO, "device control: default\n");
		return BadValue;
	}
}


#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) >= 12
static int preinit(InputDriverPtr drv, InputInfoPtr pInfo, int flags)
{
	struct MTouch *mt;

	mt = calloc(1, sizeof(*mt));
	if (!mt)
		return BadAlloc;

	pInfo->private = mt;
	pInfo->type_name = XI_TOUCHPAD;
	pInfo->device_control = device_control;
	pInfo->read_input = read_input;
	pInfo->switch_mode = 0;
	//xf86CollectInputOptions(local, NULL);
	//xf86ProcessCommonOptions(local, local->options);

	return Success;
}
#else
static InputInfoPtr preinit(InputDriverPtr drv, IDevPtr dev, int flags)
{
	struct MTouch *mt;
	InputInfoPtr local = xf86AllocateInput(drv, 0);
	if (!local)
		goto error;
	mt = calloc(1, sizeof(struct MTouch));
	if (!mt)
		goto error;

	local->name = dev->identifier;
	local->type_name = XI_TOUCHPAD;
	local->device_control = device_control;
	local->read_input = read_input;
	local->private = mt;
	local->flags = XI86_POINTER_CAPABLE | XI86_SEND_DRAG_EVENTS;
	local->conf_idev = dev;

	xf86CollectInputOptions(local, NULL, NULL);
	/* xf86OptionListReport(local->options); */
	xf86ProcessCommonOptions(local, local->options);

	local->flags |= XI86_CONFIGURED;
 error:
	return local;
}
#endif

static void uninit(InputDriverPtr drv, InputInfoPtr local, int flags)
{
	free(local->private);
	local->private = 0;
	xf86DeleteInput(local, 0);
}

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
