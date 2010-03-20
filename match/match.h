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

#ifndef MATCHER_H
#define MATCHER_H

/**
 * Special implementation of the hungarian algorithm.
 * The maximum number of fingers matches a uint32.
 * Bitmasks are used extensively.
 */

#define DIM_FINGER 32
#define DIM2_FINGER (DIM_FINGER * DIM_FINGER)

void match_fingers(int index[DIM_FINGER], int A[DIM2_FINGER],
		   int nrow, int ncol);

#endif
