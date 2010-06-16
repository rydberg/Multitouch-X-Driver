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

int close_mtouch(struct MTouch *mt, int fd)
{
	if (use_grab) {
		int rc;
		SYSCALL(rc = ioctl(fd, EVIOCGRAB, (pointer)0));
	}
	mtdev_destroy(&mt->dev);
	return 0;
}

int parse_event(struct MTouch *mt, const struct input_event *ev)
{
	mtdev_put(&mt->dev, &mt->caps, ev);
	if (!modify_hwstate(&mt->hs, &mt->dev, &mt->caps))
		return 0;
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

int mt_is_idle(struct MTouch *mt, int fd)
{
	return mt->mem.wait &&
		evbuf_empty(&mt->dev.outbuf) &&
		evbuf_empty(&mt->dev.inbuf) &&
		poll_iobuf(&mt->buf, fd, mt->mem.wait) == 0;
}
