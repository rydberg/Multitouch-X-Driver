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

#include <gestures.h>
#include <fcntl.h>
#include <xbypass.h>

static void loop_device(int fd)
{
	struct Gestures gs;
	struct MTouch mt;
	const struct input_event *ev;
	struct input_event event;
	if (configure_mtouch(&mt, fd)) {
		fprintf(stderr, "error: could not configure device\n");
		return;
	}
	if (open_mtouch(&mt, fd)) {
		fprintf(stderr, "error: could not open device\n");
		return;
	}
	while (ev = get_iobuf_event(&mt.buf, fd)) {
		if (parse_event(&mt, ev)) {
			extract_gestures(&gs, &mt);
			output_gesture(&gs);
		}
	}
	close_mtouch(&mt, fd);
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
