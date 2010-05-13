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

#include "common.h"
#include <stdio.h>
#include <time.h>

static void print_bitfield(unsigned m)
{
	int i;

	printf("%d\n", m);
	foreach_bit(i, m)
		printf("%d %d\n", i, 1 << i);
}

int main(int argc, char *argv[])
{
	print_bitfield(5);
	print_bitfield(126);
	print_bitfield(0);
	return 0;
}
