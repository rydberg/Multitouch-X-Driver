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

#include "mtdev-iobuf.h"
#include "mtdev.h"
#include <fcntl.h>
#include <xbypass.h>

static void print_event(const struct input_event *ev)
{
	static const mstime_t ms = 1000;
	static int slot;
	mstime_t evtime = ev->time.tv_usec / ms + ev->time.tv_sec * ms;
	if (ev->type == EV_ABS && ev->code == ABS_MT_SLOT)
		slot = ev->value;
	fprintf(stderr, "%012llx: %04d: %04x %04x %d\n",
		evtime, slot, ev->type, ev->code, ev->value);
}

static void loop_device(int fd)
{
	struct Capabilities caps;
	struct IOBuffer iobuf;
	struct MTDev mtdev;
	const struct input_event *ev;
	struct input_event event;
	if (read_capabilities(&caps, fd)) {
		fprintf(stderr, "error: could not read device capabilities\n");
		return;
	}
	output_capabilities(&caps);
	if (mtdev_init(&mtdev, &caps)) {
		fprintf(stderr, "error: could not initialize device\n");
		return;
	}
	init_iobuf(&iobuf);
	while (ev = get_iobuf_event(&iobuf, fd)) {
		mtdev_push(&mtdev, &caps, ev);
		while (!mtdev_empty(&mtdev)) {
			mtdev_pop(&mtdev, &event);
			print_event(&event);
		}
	}
	mtdev_destroy(&mtdev);
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "Usage: test <mtdev>\n");
		return -1;
	}
	int fd = open(argv[1], O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "error: could not open file\n");
		return -1;
	}
	loop_device(fd);
	close(fd);
	return 0;
}
