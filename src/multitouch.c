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

#include "mtouch.h"
#include "mipointer.h"

////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////

static int device_init(DeviceIntPtr dev, LocalDevicePtr local)
{
	struct MTouch *mt = local->private;
	unsigned char btmap[DIM_BUTTON +  1];
	int i;

	local->fd = xf86OpenSerial(local->options);
	if (local->fd < 0) {
		xf86Msg(X_ERROR, "multitouch: cannot configure device\n");
		return local->fd;
	}
	if (configure_mtouch(mt, local->fd))
		return -1;
	xf86CloseSerial(local->fd);

	for (i = 0; i < DIM_BUTTON + 1; i++)
		btmap[i] = i;

	dev->public.on = FALSE;
    
	InitPointerDeviceStruct((DevicePtr)dev,
				btmap,
				DIM_BUTTON,
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

	//InitDeviceProperties(local);
	//XIRegisterPropertyHandler(dev, pointer_property, NULL, NULL);

	return 0;
}

////////////////////////////////////////////////////////////////////////////

static int device_on(LocalDevicePtr local)
{
	struct MTouch *mt = local->private;
	local->fd = xf86OpenSerial(local->options);
	if (local->fd < 0) {
		xf86Msg(X_ERROR, "multitouch: cannot open device\n");
		return local->fd;
	}
	if (open_mtouch(mt, local->fd))
		return -1;
	xf86AddEnabledDevice(local);
	return 0;
}

////////////////////////////////////////////////////////////////////////////

static void device_off(LocalDevicePtr local)
{
	struct MTouch *mt = local->private;
	if(local->fd < 0)
		return;
	xf86RemoveEnabledDevice(local);
	close_mtouch(mt, local->fd);
	xf86CloseSerial(local->fd);
}

////////////////////////////////////////////////////////////////////////////

static void device_close(LocalDevicePtr local)
{
}

////////////////////////////////////////////////////////////////////////////

static void handle_state(LocalDevicePtr local,
			 const struct State *os,
			 const struct State *ns)
{
	const struct FingerState *fs, *p, *e = ns->finger + ns->nfinger;
	int dx = 0, dy = 0, i;
	for (p = ns->finger; p != e; p++) {
		if (fs = find_finger(os, p->id)) {
			dx += p->hw.position_x - fs->hw.position_x;
			dy += p->hw.position_y - fs->hw.position_y;
		}
	}
	if (dx || dy) {
		output_state(ns);
		xf86Msg(X_INFO, "motion: %d %d\n", dx, dy);
		xf86PostMotionEvent(local->dev, 0, 0, 2, dx, dy);
	}
	for (i = 0; i < DIM_BUTTON; i++)
		if (ns->button[i] != os->button[i])
			xf86PostButtonEvent(local->dev, FALSE,
					    i + 1, ns->button[i],
					    0, 0);
}

////////////////////////////////////////////////////////////////////////////

/* called for each full received packet from the touchpad */
static void read_input(LocalDevicePtr local)
{
	struct MTouch *mt = local->private;
	if (local->fd >= 0) {
		while (read_synchronized_event(mt, local->fd)) {
			modify_state(&mt->ns, &mt->hw, &mt->caps);
			handle_state(local, &mt->os, &mt->ns);
			mt->os = mt->ns;
		}
	}
}

////////////////////////////////////////////////////////////////////////////

static int control_proc(LocalDevicePtr local, xDeviceCtl *control)
{
	xf86Msg(X_INFO, "control_proc\n");
	return Success;
}

////////////////////////////////////////////////////////////////////////////

static void close_proc(LocalDevicePtr local)
{
	xf86Msg(X_INFO, "close_proc\n");
}

////////////////////////////////////////////////////////////////////////////

static int switch_mode(ClientPtr client, DeviceIntPtr dev, int mode)
{
	xf86Msg(X_INFO, "switch mode\n");
	return Success;
}

////////////////////////////////////////////////////////////////////////////

static Bool conversion_proc(LocalDevicePtr local, int first, int num,
			    int v0, int v1, int v2, int v3, int v4, int v5,
			    int *x, int *y)
{
    if (first != 0 || num != 2)
	return FALSE;

    *x = v0;
    *y = v1;

    return TRUE;
}

////////////////////////////////////////////////////////////////////////////

static Bool device_control(DeviceIntPtr dev, int mode)
{
	LocalDevicePtr local = dev->public.devicePrivate;
	switch (mode) {
	case DEVICE_INIT:
		xf86Msg(X_INFO, "device control: init\n");
		if (device_init(dev, local))
			return !Success;
		return Success;
	case DEVICE_ON:
		xf86Msg(X_INFO, "device control: on\n");
		if (device_on(local))
			return !Success;
		dev->public.on = TRUE;
		return Success;
	case DEVICE_OFF:
		xf86Msg(X_INFO, "device control: off\n");
		dev->public.on = FALSE;
		device_off(local);
		return Success;
	case DEVICE_CLOSE:
		xf86Msg(X_INFO, "device control: close\n");
		device_close(local);
		return Success;
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
	local->control_proc = control_proc;
	local->close_proc = close_proc;
	local->switch_mode = switch_mode;
	local->conversion_proc = conversion_proc;
	local->reverse_conversion_proc = NULL;
	local->dev = NULL;
	local->private = mt;
	local->private_flags = 0;
	local->flags = XI86_POINTER_CAPABLE | XI86_SEND_DRAG_EVENTS;
	local->conf_idev = dev;
	local->always_core_feedback = 0;
	
	xf86CollectInputOptions(local, NULL, NULL);
	xf86OptionListReport(local->options);

	local->history_size = xf86SetIntOption(local->options,
					       "HistorySize", 0);

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
