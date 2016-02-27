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
		//PLUGINLOG(__FUNCTION__ " %lu", sizeof(gteCoordsValidation));
		//gteCoordsValidation.fill(std::bitset<0x800 * 2>());
		memset(gteCoordsValidation.data(), 0, sizeof(gteCoordsValidation));
		dirty = false;
	}
}

bool GTEAccuracy::get(s16 sx, s16 sy, GTEVertex* vertex)
{
	if (!gteCoordsValidation[sy + 0x800][sx + 0x800])
		return false;

	if (sx >= -0x800 && sx <= 0x7FF &&
		sy >= -0x800 && sy <= 0x7FF)
	{
		if ((std::fabs(gteCoords[sy + 0x800][sx + 0x800].x - sx) < 1.0f) &&
			(std::fabs(gteCoords[sy + 0x800][sx + 0x800].y - sy) < 1.0f))
		{
			vertex->x = gteCoords[sy + 0x800][sx + 0x800].x;
			vertex->y = gteCoords[sy + 0x800][sx + 0x800].y;
			return true;
		}
	}
	return false;
}

void GTEAccuracy::set(s16 sx, s16 sy, s64 fx, s64 fy, s64 fz)
{
	if (sx >= -0x800 && sx <= 0x7FF &&
		sy >= -0x800 && sy <= 0x7FF)
	{
		gteCoords[sy + 0x800][sx + 0x800].x = fx / 65536.0f;
		gteCoords[sy + 0x800][sx + 0x800].y = fy / 65536.0f;

		gteCoordsValidation[sy + 0x800][sx + 0x800] = true;
		dirty = true;
	}
}


