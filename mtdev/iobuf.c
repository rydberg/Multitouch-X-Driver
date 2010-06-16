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

#include <mtdev-iobuf.h>

void init_iobuf(struct IOBuffer *buf)
{
	memset(buf, 0, sizeof(struct IOBuffer));
	buf->at = buf->begin;
	buf->top = buf->at;
	buf->end = buf->begin + DIM_BUFFER;
}

const struct input_event *get_iobuf_event(struct IOBuffer *buf, int fd)
{
	const struct input_event *ev;
	int n = buf->top - buf->at;
	if (n < EVENT_SIZE) {
		/* partial event is available: save it */
		if (buf->at != buf->begin && n > 0)
			memmove(buf->begin, buf->at, n);
		/* start from the beginning */
		buf->at = buf->begin;
		buf->top = buf->at + n;
		/* read more data */
		SYSCALL(n = read(fd, buf->top, buf->end - buf->top));
		if (n <= 0)
			return NULL;
		buf->top += n;
	}
	if (buf->top - buf->at < EVENT_SIZE)
		return NULL;
	ev = (const struct input_event *)buf->at;
	buf->at += EVENT_SIZE;
	return ev;
}
