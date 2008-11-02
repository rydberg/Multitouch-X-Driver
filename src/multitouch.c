/***************************************************************************
 *
 * addon-generic-kbd-backlight.c:
 * Copyright (C) 2008 Henrik Rydberg <rydberg@euromail.se>
 *
 * Based on addon-generic-backlight.c:
 * Copyright (C) 2008 Danny Kukawka <danny.kukawka@web.de>
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

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "glib-abi.h"
#include "dbus-abi.h"
#include "hal-abi.h"

#define HAL_PROPERTY "org.freedesktop.Hal.Device.KeyboardBacklight"

static const char prop_name[] = HAL_PROPERTY;
static const char prop_invalid[] = HAL_PROPERTY ".Invalid";
static const char prop_set[] = "SetBrightness";
static const char prop_get[] = "GetBrightness";

static struct GMainLoop *main_loop;
static struct LibHalContext *halctx;
static struct DBusConnection *conn;

static char sysfs_path[512];
static int levels;

/**
 * Get keyboard backlight level
 *
 * Tries to read from the specified sysfs device, and if successful,
 * updates the internal keyboard backlight state. Warns if the device
 * cannot be opened or be read from. Always returns the last succesful
 * read.
 */
static int get_kbd_backlight()
{
	static int level;
	int fd;
	char buf[64];

	fd = open(sysfs_path, O_RDONLY);
	if (fd < 0) {
		HAL_WARNING(("Could not open '%s'", sysfs_path));
		return level;
	}

	memset(buf, 0, sizeof(buf));
	if (read(fd, buf, sizeof(buf)) < 0)
		HAL_WARNING(("Could not read from '%s'", sysfs_path));
	else
		level = atoi(buf);

	close(fd);
	return level;
}

/**
 * Set keyboard backlight level
 *
 * Warns if the device cannot be opened or be written to.
 * Returns bytes written on success, negative value on failure.
 */
static int set_kbd_backlight(int level)
{
	int fd, ret;
	char buf[64];

	sprintf(buf, "%d", level);

	fd = open(sysfs_path, O_WRONLY);
	if (fd < 0) {
		HAL_WARNING(("Could not open '%s'", sysfs_path));
		return -1;
	}

	ret = write(fd, buf, strlen(buf));
	if (ret < 0)
		HAL_WARNING(("Could not write '%s' to '%s'", buf, sysfs_path));

	close(fd);
	return ret;
}

/* DBus set keyboard backlight level */
static struct DBusMessage *dbus_get_kbd_backlight(struct DBusMessage *msg)
{
	struct DBusError err;
	struct DBusMessage *reply;
	int level;

	dbus_error_init(&err);
	if (!dbus_message_get_args(msg, &err, DBUS_TYPE_INVALID))
		return dbus_message_new_error(msg, prop_invalid,
			"Invalid message");

	level = get_kbd_backlight();

	reply = dbus_message_new_method_return(msg);
	if (reply)
		dbus_message_append_args(reply, DBUS_TYPE_INT32, &level,
			DBUS_TYPE_INVALID);

	return reply;
}

/* DBus set keyboard backlight level */
static struct DBusMessage *dbus_set_kbd_backlight(struct DBusMessage *msg)
{
	struct DBusError err;
	int level;

	dbus_error_init(&err);
	if (!dbus_message_get_args(msg, &err, DBUS_TYPE_INT32, &level,
			DBUS_TYPE_INVALID))
		return dbus_message_new_error(msg, prop_invalid,
			"Brightness level argument missing");

	if (level < 0 || level > levels - 1)
		return dbus_message_new_error(msg, prop_invalid,
			"Brightness level is invalid");

	set_kbd_backlight(level);

	return dbus_message_new_method_return(msg);
}

/* DBus filter function */
static DBusHandlerResult filter_function(struct DBusConnection *connection,
					 struct DBusMessage *message,
					 void *userdata)
{
	struct DBusMessage *reply = NULL;

	if (dbus_message_is_method_call(message, prop_name, prop_get))
		reply = dbus_get_kbd_backlight(message);
	else if (dbus_message_is_method_call(message, prop_name, prop_set))
		reply = dbus_set_kbd_backlight(message);

	if (reply) {
		dbus_connection_send(connection, reply, NULL);
		dbus_message_unref(reply);
	}

	return DBUS_HANDLER_RESULT_HANDLED;
}

int main(int argc, char *argv[])
{
	struct DBusError err;
	struct stat fs;

	char *udi = getenv("UDI");
	char *path = getenv("HAL_PROP_LINUX_SYSFS_PATH");
	char *level_str = getenv("HAL_PROP_KEYBOARD_BACKLIGHT_NUM_LEVELS");

	setup_logger();

	if (udi == NULL) {
		HAL_ERROR(("No device specified"));
		return -2;
	}
	if (path == NULL) {
		HAL_ERROR(("No sysfs path specified"));
		return -2;
	}
	if (level_str == NULL) {
		HAL_ERROR(("No keyboard_backlight.num_levels defined"));
		return -2;
	}

	levels = atoi(level_str);
	snprintf(sysfs_path, sizeof(sysfs_path), "%s/brightness", path);

	HAL_DEBUG(("udi='%s', path='%s', levels='%d'", udi, path, levels));

	if (stat(sysfs_path, &fs)) {
		HAL_ERROR(("The sysfs property path does not exist"));
		return -2;
	}

	dbus_error_init(&err);
	halctx = libhal_ctx_init_direct(&err);

	if (halctx == NULL) {
		HAL_ERROR(("Cannot connect to hald"));
		return -3;
	}

	conn = libhal_ctx_get_dbus_connection(halctx);
	dbus_connection_setup_with_g_main(conn, NULL);

	dbus_connection_add_filter(conn, filter_function, NULL, NULL);

	if (!libhal_device_claim_interface(halctx, udi, prop_name,
		"    <method name=\"GetBrightness\">\n"
		"      <arg name=\"brightness_value\" direction=\"out\" type=\"i\"/>\n"
		"    </method>\n"
		"    <method name=\"SetBrightness\">\n"
		"      <arg name=\"brightness_value\" direction=\"in\" type=\"i\"/>\n"
		"    </method>\n",
		&err)) {
		HAL_ERROR(("Cannot claim interface '" HAL_PROPERTY "'"));
		return -4;
	}

	dbus_error_init(&err);
	if (!libhal_device_addon_is_ready(halctx, udi, &err))
		return -4;

	main_loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(main_loop);

	return 0;
}

