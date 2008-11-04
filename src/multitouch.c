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

#include "capabilities.h"

////////////////////////////////////////////////////////////////////////////

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
