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

#ifndef MTDEV_EVBUF_H
#define MTDEV_EVBUF_H

#include "common.h"

struct EventBuffer {
	int head;
	int tail;
	struct input_event buffer[DIM_EVENTS];
};

static inline void evbuf_init(struct EventBuffer *evbuf)
{
	memset(evbuf, 0, sizeof(*evbuf));
}

static inline int evbuf_empty(const struct EventBuffer *evbuf)
{
	return evbuf->head == evbuf->tail;
}

static inline void evbuf_put(struct EventBuffer *evbuf,
			     const struct input_event *ev)
{
	evbuf->buffer[evbuf->head++] = *ev;
	evbuf->head &= DIM_EVENTS - 1;
}

static inline void evbuf_get(struct EventBuffer *evbuf,
			     struct input_event *ev)
{
	*ev = evbuf->buffer[evbuf->tail++];
	evbuf->tail &= DIM_EVENTS - 1;
}

#endif
