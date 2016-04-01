#pragma once

/***************************************************************************
*   Copyright (C) 2016 by iCatButler                                          *
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
	//float z;
};

typedef struct
{
	float	x;
	float	y;
	float	z;
	unsigned int	valid;
	unsigned int	count;
} PGXP_vertex;

static const uint32_t addr_mask[8] = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
0x7FFFFFFF, 0x1FFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };

class PGXP
{
private:
	typedef BOOL(__cdecl* offset_fn)(void);
	typedef void(__cdecl* primPoly_fn)(unsigned char *baseAddr);

	std::array<short*, 4> lx;
	std::array<short*, 4> ly;

	std::array<PGXP_vertex, 4> fxy;

	std::array<OGLVertex*, 4> vertex;
	s16* PSXDisplay_CumulOffset_x;
	s16* PSXDisplay_CumulOffset_y;

	PGXP_vertex* PGXP_Mem;
	u32* lUsedAddr;
	int* gpuDataP;
	u32 currentAddr = 0;

	void fix_offsets(s32 count);

	static offset_fn ooffset3;
	static BOOL __cdecl offset3(void);

	static offset_fn ooffset4;
	static BOOL __cdecl offset4(void);

	static primPoly_fn oprimPolyF3;
	static void __cdecl primPolyF3(unsigned char *baseAddr);

	static primPoly_fn oprimPolyFT3;
	static void __cdecl primPolyFT3(unsigned char *baseAddr);

	static primPoly_fn oprimPolyF4;
	static void __cdecl primPolyF4(unsigned char *baseAddr);

	static primPoly_fn oprimPolyFT4;
	static void __cdecl primPolyFT4(unsigned char *baseAddr);

	static primPoly_fn oprimPolyG3;
	static void __cdecl primPolyG3(unsigned char *baseAddr);

	static primPoly_fn oprimPolyGT3;
	static void __cdecl primPolyGT3(unsigned char *baseAddr);

	static primPoly_fn oprimPolyG4;
	static void __cdecl primPolyG4(unsigned char *baseAddr);

	static primPoly_fn oprimPolyGT4;
	static void __cdecl primPolyGT4(unsigned char *baseAddr);

	void GetVertices(u32* baseAddr);

public:
	PGXP();
	~PGXP();

	void SetMemoryPtr(unsigned int addr, unsigned char* pVRAM);
	void SetAddress();

};

