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

#include <common.h>
#include <fcntl.h>
#include <xbypass.h>

#define BIT_DEF(name)				       \
	printf("#define BIT_"#name" %d\n", \
	       cabs2mt[ABS_MT_##name] - 1)

static unsigned int cabs2mt[ABS_CNT];
static unsigned int cmt2abs[MT_ABS_SIZE];

void init_caps()
{
	static const int init_abs_map[MT_ABS_SIZE] = MT_SLOT_ABS_EVENTS;
	int i;
	for (i = 0; i < MT_ABS_SIZE; i++) {
		cabs2mt[init_abs_map[i]] = i + 1;
		cmt2abs[i] = init_abs_map[i];
	}
}

static inline const char *newln(int i, int n)
{
	return i == n - 1 || i % 8 == 7 ? "\n" : "";
}

int main(int argc, char *argv[])
{
	int i;
	init_caps();
	printf("static const unsigned int map_abs2mt[ABS_CNT] = {\n");
	for (i = 0; i < ABS_CNT; i++)
		printf(" 0x%04x,%s", cabs2mt[i], newln(i, ABS_CNT));
	printf("};\n\n");
	printf("static const unsigned int map_mt2abs[MT_ABS_SIZE] = {\n");
	for (i = 0; i < MT_ABS_SIZE; i++)
		printf(" 0x%04x,%s", cmt2abs[i], newln(i, MT_ABS_SIZE));
	printf("};\n\n");
	BIT_DEF(TRACKING_ID);
	BIT_DEF(POSITION_X);
	BIT_DEF(POSITION_Y);
	BIT_DEF(TOUCH_MAJOR);
	BIT_DEF(TOUCH_MINOR);
	BIT_DEF(WIDTH_MAJOR);
	BIT_DEF(WIDTH_MINOR);
	printf("\n");
	return 0;
}
