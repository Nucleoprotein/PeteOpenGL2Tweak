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

void GTEAccuracy::init(bool small_cache)
{
	size_t cache_width = 0x800 * 2;
	size_t cache_height = 0x800 * 2;

	if (small_cache)
	{
		this->small_cache = small_cache;
		cache_width = 640;
		cache_height = 480;
	}

	gteCoords = std::vector<std::vector<GTEVertex>>(cache_height, std::vector<GTEVertex>(cache_width));
	clear();

	PLUGINLOG("GTEAccuracy Cache Size %.2fMB", gteCoords.size() * gteCoords[0].size() * sizeof(GTEVertex) / (1024.0f*1024.0f) );
}

void GTEAccuracy::clear()
{
	if (dirty)
	{
		//PLUGINLOG(__FUNCTION__);
		for (auto && w : gteCoords)
			std::fill(w.begin(), w.end(), GTEVertex());

		dirty = false;
	}
}

bool GTEAccuracy::get(s16 sx, s16 sy, GTEVertex* vertex)
{
	static s32 sx_center = gteCoords[0].size() / 2;
	static s32 sy_center = gteCoords.size() / 2;

	if (sx >= -sx_center && sx <= sx_center - 1 &&
		sy >= -sy_center && sy <= sy_center - 1)
	{
		if ((std::fabs(gteCoords[sy + sy_center][sx + sx_center].x - sx) < 1.0f) &&
			(std::fabs(gteCoords[sy + sy_center][sx + sx_center].y - sy) < 1.0f))
		{
			vertex->x = gteCoords[sy + sy_center][sx + sx_center].x;
			vertex->y = gteCoords[sy + sy_center][sx + sx_center].y;
			return true;
		}
	}
	return false;
}

void GTEAccuracy::set(s16 sx, s16 sy, s64 fx, s64 fy, s64 fz)
{
	static s32 sx_center = gteCoords[0].size() / 2;
	static s32 sy_center = gteCoords.size() / 2;

	if (sx >= -sx_center && sx <= sx_center - 1 &&
		sy >= -sy_center && sy <= sy_center - 1)
	{
		gteCoords[sy + sy_center][sx + sx_center].x = fx / 65536.0f;
		gteCoords[sy + sy_center][sx + sx_center].y = fy / 65536.0f;
		dirty = true;
	}
}


