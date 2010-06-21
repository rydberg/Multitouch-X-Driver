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

#include "mtouch.h"

static const int use_grab = 1;

int configure_mtouch(struct MTouch *mt, int fd)
{
	int rc = read_capabilities(&mt->caps, fd);
	if (rc < 0)
		return rc;
	output_capabilities(&mt->caps);
	return 0;
}

int open_mtouch(struct MTouch *mt, int fd)
{
	mtdev_init(&mt->dev, &mt->caps);
	init_iobuf(&mt->buf);
	init_hwstate(&mt->hs, &mt->caps);
	init_mtstate(&mt->prev_state);
	init_mtstate(&mt->state);
	init_memory(&mt->mem);
	if (use_grab) {
		int rc;
		SYSCALL(rc = ioctl(fd, EVIOCGRAB, (pointer)1));
		return rc;
	}
	return 0;
}


int get_mtouch(struct MTouch *mt, int fd, struct input_event* ev, int ev_max)
{
	const struct input_event *kev;
	int count = 0;
	while (count < ev_max) {
		while (mtdev_empty(&mt->dev)) {
			kev = get_iobuf_event(&mt->buf, fd);
			if (!kev)
				return count;
			mtdev_put(&mt->dev, &mt->caps, kev);
		}
		mtdev_get(&mt->dev, &ev[count++]);
	}
	return count;
}

int close_mtouch(struct MTouch *mt, int fd)
{
	if (use_grab) {
		int rc;
		SYSCALL(rc = ioctl(fd, EVIOCGRAB, (pointer)0));
	}
	mtdev_destroy(&mt->dev);
	return 0;
}

int read_packet(struct MTouch *mt, int fd)
{
	struct input_event ev;
	int ret;
	while ((ret = get_mtouch(mt, fd, &ev, 1)) > 0)
		if (hwstate_read(&mt->hs, &mt->caps, &ev))
			break;
	if (ret <= 0)
		return ret;
	extract_mtstate(&mt->state, &mt->hs, &mt->caps);
#if 0
	output_mtstate(&mt->state);
#endif
	refresh_memory(&mt->mem, &mt->prev_state, &mt->state, &mt->caps);
#if 0
	output_memory(&mt->mem);
#endif
	return 1;
}

int has_delayed_gestures(struct MTouch *mt, int fd)
{
	return mt->mem.wait &&
		mtdev_empty(&mt->dev) &&
		poll_iobuf(&mt->buf, fd, mt->mem.wait) == 0;
}
