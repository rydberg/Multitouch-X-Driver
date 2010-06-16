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

#ifndef MTDEV_H
#define MTDEV_H

#include "mtdev-caps.h"
#include "mtdev-evbuf.h"

/**
 * struct MTDev - represents an input MT device
 * @inbuf: input event buffer
 * @outbuf: output event buffer
 * @priv: structure of private data
 */
struct MTDev {
	struct EventBuffer inbuf;
	struct EventBuffer outbuf;
	struct MTDevState *priv;
};

int mtdev_init(struct MTDev *mtdev, const struct Capabilities *caps);

static inline int mtdev_empty(struct MTDev *mtdev)
{
	return evbuf_empty(&mtdev->outbuf);
}

void mtdev_put(struct MTDev *dev, const struct Capabilities *caps,
	       const struct input_event *ev);

static inline void mtdev_get(struct MTDev *mtdev, struct input_event* ev)
{
	evbuf_get(&mtdev->outbuf, ev);
}

void mtdev_destroy(struct MTDev *mtdev);

#endif
