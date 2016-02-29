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

struct OGLVertex
{
	float x;
	float y;
	float z;
};

struct GTEVertex
{
	float x;
	float y;
	float z;
};

class GTEAccHack
{
private:
	typedef BOOL(__cdecl* offset_fn)(void);

	std::array<s16*, 4> lx;
	std::array<s16*, 4> ly;
	std::array<OGLVertex*, 4> vertex;
	s16* PSXDisplay_CumulOffset_x;
	s16* PSXDisplay_CumulOffset_y;
	s32* iDataReadMode;

	std::array<std::array<GTEVertex, 0x1000>, 0x1000> gteCoords;
	std::array<std::bitset<0x1000>, 0x1000> isCoordValid;
	std::array<std::bitset<0x1000>, 0x1000> isCoordDrawn;
	bool is_dirty = true;

	void GetGTEVertex(s16 sx, s16 sy, OGLVertex* vertex);
	void ClearCache();

	void fix_offsets(s32 count);

	static BOOL __cdecl offset3(void);
	static offset_fn ooffset3;

	static BOOL __cdecl offset4(void);
	static offset_fn ooffset4;

public:
	GTEAccHack();
	~GTEAccHack();

	void AddGTEVertex(s16 sx, s16 sy, s64 llx, s64 lly, s64 llz);
	void ClearGTEVertex(s16 sx, s16 sy, u16 z);
	void ResetGTECache(bool single);
	s32 GetGTEVertex(s16 sx, s16 sy, u16 z, float* x, float* y);
};

