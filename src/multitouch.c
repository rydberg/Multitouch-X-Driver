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


////////////////////////////////////////////////////////////////////////////

static void pointer_control(DeviceIntPtr dev, PtrCtrl *ctrl)
{
	xf86Msg(X_INFO, "pointer_control\n");
}

////////////////////////////////////////////////////////////////////////////

static int pointer_property(DeviceIntPtr dev,
			    Atom property,
			    XIPropertyValuePtr prop,
			    BOOL checkonly)
{
	xf86Msg(X_INFO, "pointer_property\n");
	return Success;
}

////////////////////////////////////////////////////////////////////////////



#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) >= 7
static void InitAxesLabels(Atom *labels, int nlabels)
{
	memset(labels, 0, nlabels * sizeof(Atom));
	switch(nlabels)
	{
		default:
		case 2:
			labels[1] = XIGetKnownProperty(AXIS_LABEL_PROP_REL_Y);
		case 1:
			labels[0] = XIGetKnownProperty(AXIS_LABEL_PROP_REL_X);
			break;
	}
}

static void InitButtonLabels(Atom *labels, int nlabels)
{
	memset(labels, 0, nlabels * sizeof(Atom));
	switch(nlabels)
	{
		default:
		case 7:
			labels[6] = XIGetKnownProperty(BTN_LABEL_PROP_BTN_HWHEEL_RIGHT);
		case 6:
			labels[5] = XIGetKnownProperty(BTN_LABEL_PROP_BTN_HWHEEL_LEFT);
		case 5:
			labels[4] = XIGetKnownProperty(BTN_LABEL_PROP_BTN_WHEEL_DOWN);
		case 4:
			labels[3] = XIGetKnownProperty(BTN_LABEL_PROP_BTN_WHEEL_UP);
		case 3:
			labels[2] = XIGetKnownProperty(BTN_LABEL_PROP_BTN_RIGHT);
		case 2:
			labels[1] = XIGetKnownProperty(BTN_LABEL_PROP_BTN_MIDDLE);
		case 1:
			labels[0] = XIGetKnownProperty(BTN_LABEL_PROP_BTN_LEFT);
			break;
	}
}
#endif

static int device_init(DeviceIntPtr dev, LocalDevicePtr local)
{
	struct MTouch *mt = local->private;
	unsigned char btmap[DIM_BUTTON + 1]={0,1,2,3};
#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) >= 7
	Atom btn_labels[SYN_MAX_BUTTONS] = { 0 };
	Atom axes_labels[2] = { 0 };

	InitAxesLabels(axes_labels, 2);
	InitButtonLabels(btn_labels, SYN_MAX_BUTTONS);
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
#elif GET_ABI_MAJOR(ABI_XINPUT_VERSION) == 7
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
				   mt->caps.abs_position_x.minimum,
				   mt->caps.abs_position_x.maximum,
				   1, 0, 1);
	xf86InitValuatorDefaults(dev, 0);
	xf86InitValuatorAxisStruct(dev, 1,
#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) >= 7
					axes_labels[1],
#endif
				   mt->caps.abs_position_y.minimum,
				   mt->caps.abs_position_y.maximum,
				   1, 0, 1);
	xf86InitValuatorDefaults(dev, 1);

	XIRegisterPropertyHandler(dev, pointer_property, NULL, NULL);

	return Success;
}

////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////

static int device_off(LocalDevicePtr local)
{
	struct MTouch *mt = local->private;
	xf86RemoveEnabledDevice(local);
	if(close_mtouch(mt, local->fd)) {
		xf86Msg(X_WARNING, "multitouch: cannot ungrab device\n");
	}
	xf86CloseSerial(local->fd);
	return Success;
}

////////////////////////////////////////////////////////////////////////////

static int device_close(LocalDevicePtr local)
{
	return Success;
}

////////////////////////////////////////////////////////////////////////////

static void tickle_button(LocalDevicePtr local, int id)
{
	xf86PostButtonEvent(local->dev, FALSE, id, 1, 0, 0);
	xf86PostButtonEvent(local->dev, FALSE, id, 0, 0, 0);
}

////////////////////////////////////////////////////////////////////////////

static void handle_gestures(LocalDevicePtr local,
			    const struct Gestures *gs,
			    const struct Capabilities *caps)
{
	static int vscroll, hscroll;
	int i;
	for (i = 0; i < gs->nbt; i++) {
		xf86PostButtonEvent(local->dev, FALSE,
				    gs->btix[i], gs->btval[i],
				    0, 0);
		xf86Msg(X_INFO, "button: %d %d\n", gs->btix[i], gs->btval[i]);
	}
	if (GETBIT(gs->type, GS_MOVE)) {
		xf86PostMotionEvent(local->dev, 0, 0, 2,
				    gs->dx, gs->dy);
		xf86Msg(X_INFO, "motion: %d %d\n", gs->dx, gs->dy);
	}
	if (GETBIT(gs->type, GS_VSCROLL)) {
		int vstep = 0.03 * (caps->abs_position_y.maximum -
				    caps->abs_position_y.minimum);
		vscroll += gs->dy;
		while (vscroll > vstep) {
			tickle_button(local, 5);
			vscroll -= vstep;
		}
		while (vscroll < -vstep) {
			tickle_button(local, 4);
			vscroll += vstep;
		}
		xf86Msg(X_INFO, "vscroll: %d\n", gs->dy);
	}
	if (GETBIT(gs->type, GS_HSCROLL)) {
		int hstep = 0.1 * (caps->abs_position_x.maximum -
				   caps->abs_position_x.minimum);
		hscroll += gs->dx;
		while (hscroll > hstep) {
			tickle_button(local, 6);
			hscroll -= hstep;
		}
		while (hscroll < -hstep) {
			tickle_button(local, 7);
			hscroll += hstep;
		}
		xf86Msg(X_INFO, "hscroll: %d\n", gs->dx);
	}
}

////////////////////////////////////////////////////////////////////////////

/* called for each full received packet from the touchpad */
static void read_input(LocalDevicePtr local)
{
	struct Gestures gs;
	struct MTouch *mt = local->private;
	while (read_synchronized_event(mt, local->fd)) {
		parse_event(mt);
		extract_gestures(&gs, mt);
		handle_gestures(local, &gs, &mt->caps);
	}
}

////////////////////////////////////////////////////////////////////////////

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


////////////////////////////////////////////////////////////////////////////

static InputInfoPtr preinit(InputDriverPtr drv, IDevPtr dev, int flags)
{
	struct MTouch *mt;
	InputInfoPtr local = xf86AllocateInput(drv, 0);
	if (!local)
		goto error;
	mt = xcalloc(1, sizeof(struct MTouch));
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
	//xf86OptionListReport(local->options);
	xf86ProcessCommonOptions(local, local->options);

	local->flags |= XI86_CONFIGURED;
 error:
	return local;
}

static void uninit(InputDriverPtr drv, InputInfoPtr local, int flags)
{
	xfree(local->private);
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
