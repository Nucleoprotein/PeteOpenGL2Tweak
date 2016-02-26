/***************************************************************************
 *   Copyright (C) 2015 by tapcio                                          *
 *   Copyright (C) 2011 by Blade_Arma                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.           *
 ***************************************************************************/

 // Orginal code by Blade_Arma, additional fixes and conversion to C++ class by tapcio

#include "stdafx.h"

#include <math.h>

#include "gte_accuracy.h"
#include "gpuPeteOpenGL2Tweak.h"

void GTEAccuracy::clear()
{
	if (dirty)
	{
		//gteCoords.fill(std::array<OGLVertex, COORDS_ARRAY_SIZE>());
		// memset is faster 
		memset(gteCoords.data(), 0, COORDS_ARRAY_SIZE * COORDS_ARRAY_SIZE * sizeof(GTEVertex));
		dirty = false;
	}
}

bool GTEAccuracy::get(s16 sx, s16 sy, GTEVertex* vertex)
{
	if (sx >= -0x800 && sx <= 0x7ff &&
		sy >= -0x800 && sy <= 0x7ff)
	{
		if ((std::fabs(gteCoords[sy + 0x800][sx + 0x800].x - sx) < 1.0f) &&
			(std::fabs(gteCoords[sy + 0x800][sx + 0x800].y - sy) < 1.0f))
		{
			//PLUGINLOG("%fx%f %dx%d", gteCoords[sy + 0x800][sx + 0x800].x, gteCoords[sy + 0x800][sx + 0x800].y, sx, sy);
			vertex->x = gteCoords[sy + 0x800][sx + 0x800].x;
			vertex->y = gteCoords[sy + 0x800][sx + 0x800].y;
			//vertex->z = gteCoords[sy + 0x800][sx + 0x800].z;

			return true;
		}
	}

	return false;
}

void GTEAccuracy::set(s16 sx, s16 sy, s64 fx, s64 fy, s64 fz)
{
	if (sx >= -0x800 && sx <= 0x7ff &&
		sy >= -0x800 && sy <= 0x7ff)
	{
		gteCoords[sy + 0x800][sx + 0x800].x = fx / 65536.0f;
		gteCoords[sy + 0x800][sx + 0x800].y = fy / 65536.0f;

		dirty = true;

		//gteCoords[sy + 0x800][sx + 0x800].z = fz / 65536.0f;

		//PLUGINLOG("%dx%d %lldx%lld %fx%f", sx, sy, fx, fy, gteCoords[sy + 0x800][sx + 0x800].x, gteCoords[sy + 0x800][sx + 0x800].y);
	}
}


