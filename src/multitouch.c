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

#include "gestures.h"

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

static int device_init(DeviceIntPtr dev, LocalDevicePtr local)
{
	struct MTouch *mt = local->private;
	unsigned char btmap[DIM_BUTTON + 1]={0,1,2,3};

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

	InitPointerDeviceStruct((DevicePtr)dev,
				btmap, DIM_BUTTON,
				GetMotionHistory,
				pointer_control,
				GetMotionHistorySize(),
				2);

	xf86InitValuatorAxisStruct(dev, 0,
				   mt->caps.abs_position_x.minimum,
				   mt->caps.abs_position_x.maximum,
				   1, 0, 1);
	xf86InitValuatorDefaults(dev, 0);
	xf86InitValuatorAxisStruct(dev, 1,
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

static void handle_gestures(LocalDevicePtr local,
			    const struct Gestures *gs)
{
	int i;
	if (GETBIT(gs->type, GS_MOVE)) {
		xf86PostMotionEvent(local->dev, 0, 0, 2, gs->dx, gs->dy);
		//xf86Msg(X_INFO, "motion: %d %d\n", gs->dx, gs->dy);
	}
	for (i = 0; i < gs->nbt; i++) {
		xf86PostButtonEvent(local->dev, FALSE,
				    gs->btix[i], gs->btval[i],
				    0, 0);
		xf86Msg(X_INFO, "button: %d %d\n", gs->btix[i], gs->btval[i]);
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
		handle_gestures(local, &gs);
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
