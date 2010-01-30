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

/******************************************************/

int configure_mtouch(struct MTouch *mt, int fd)
{
	int rc = read_capabilities(&mt->caps, fd);
	if (rc < 0)
		return rc;
	output_capabilities(&mt->caps);
	return 0;
}

/******************************************************/

int open_mtouch(struct MTouch *mt, int fd)
{
	int rc;
	init_iobuf(&mt->buf);
	init_hwdata(&mt->hw);
	init_state(&mt->os);
	init_state(&mt->ns);
	SYSCALL(rc = ioctl(fd, EVIOCGRAB, (pointer)1));
	return rc;
}

/******************************************************/

int close_mtouch(struct MTouch *mt, int fd)
{
	int rc;
	SYSCALL(rc = ioctl(fd, EVIOCGRAB, (pointer)0));
	return rc;
}

/******************************************************/

bool read_synchronized_event(struct MTouch *mt, int fd)
{
	const struct input_event* ev;
	while(ev = get_iobuf_event(&mt->buf, fd))
		if (read_hwdata(&mt->hw, ev))
		    return 1;
	return 0;
}

/******************************************************/

void parse_event(struct MTouch *mt)
{
	modify_state(&mt->ns, &mt->hw, &mt->caps);
}

/******************************************************/
