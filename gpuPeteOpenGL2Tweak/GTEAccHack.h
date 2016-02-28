#pragma once

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

struct GTEVertex
{
	float x;
	float y;
	//float z; - not used
};

class GTEAccHack
{
private:
	typedef BOOL(__cdecl* offset_fn)(void);

	std::array<s16*, 4> lx;
	std::array<s16*, 4> ly;
	std::array<GTEVertex*, 4> vertex;
	s16* PSXDisplay_CumulOffset_x;
	s16* PSXDisplay_CumulOffset_y;
	s32* iDataReadMode;

	std::array<std::array<GTEVertex, 0x800 * 2>, 0x800 * 2> gteCoords;
	std::array<std::bitset<0x800 * 2>, 0x800 * 2> isCoordValid;
	bool is_dirty = true;

	bool GetGTEVertex(s16 sx, s16 sy, GTEVertex* vertex);
	void ClearCache();

	void fix_offsets(s32 count);

	static BOOL __cdecl offset3(void);
	static offset_fn ooffset3;

	static BOOL __cdecl offset4(void);
	static offset_fn ooffset4;

public:
	GTEAccHack();
	~GTEAccHack();

	void AddGTEVertex(s16 sx, s16 sy, s64 fx, s64 fy, s64 fz);
	void ResetGTECache(bool single);
};

