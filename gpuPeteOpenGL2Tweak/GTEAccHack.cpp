#include "stdafx.h"
#include "GTEAccHack.h"

#include "gpuPeteOpenGL2Tweak.h"
#include "GPUPlugin.h"

#include <ppl.h>

static GTEAccHack* s_GTEAccHack;

GTEAccHack::GTEAccHack()
{
	PLUGINLOG("GTE Accuracy Hack Enabled");

	s_GTEAccHack = this;

	lx[0] = (s16*)GPUPlugin::Get().GetPluginMem(0x00051A08);
	lx[1] = (s16*)GPUPlugin::Get().GetPluginMem(0x00051A0A);
	lx[2] = (s16*)GPUPlugin::Get().GetPluginMem(0x00051A0C);
	lx[3] = (s16*)GPUPlugin::Get().GetPluginMem(0x00051A0E);

	ly[0] = (s16*)GPUPlugin::Get().GetPluginMem(0x00051A10);
	ly[1] = (s16*)GPUPlugin::Get().GetPluginMem(0x00051A12);
	ly[2] = (s16*)GPUPlugin::Get().GetPluginMem(0x00051A14);
	ly[3] = (s16*)GPUPlugin::Get().GetPluginMem(0x00051A16);

	vertex[0] = (GTEVertex*)GPUPlugin::Get().GetPluginMem(0x00052220);
	vertex[1] = (GTEVertex*)GPUPlugin::Get().GetPluginMem(0x00052238);
	vertex[2] = (GTEVertex*)GPUPlugin::Get().GetPluginMem(0x00052250);
	vertex[3] = (GTEVertex*)GPUPlugin::Get().GetPluginMem(0x00052268);

	PSXDisplay_CumulOffset_x = (s16*)GPUPlugin::Get().GetPluginMem(0x00051FFC);
	PSXDisplay_CumulOffset_y = (s16*)GPUPlugin::Get().GetPluginMem(0x00051FFE);

	iDataReadMode = (s32*)GPUPlugin::Get().GetPluginMem(0x00051F2C);

	offset_fn offset3 = (offset_fn)GPUPlugin::Get().GetPluginMem(0x000041B0);
	CreateHook(offset3, GTEAccHack::offset3, &ooffset3);
	EnableHook(offset3);

	offset_fn offset4 = (offset_fn)GPUPlugin::Get().GetPluginMem(0x000043F0);
	CreateHook(offset4, GTEAccHack::offset4, &ooffset4);
	EnableHook(offset4);

	ClearCache();
}

GTEAccHack::~GTEAccHack()
{
}

void GTEAccHack::ResetGTECache(bool single)
{
	if (*iDataReadMode == 1 || single)
		ClearCache();
}

void GTEAccHack::ClearCache()
{
	if (is_dirty)
	{
		isCoordValid.fill(0);
	}
}

bool GTEAccHack::GetGTEVertex(s16 sx, s16 sy, GTEVertex* vertex)
{
	if (sx >= -0x800 && sx <= 0x7FF &&
		sy >= -0x800 && sy <= 0x7FF)
	{
		s32 x = sx + 0x800;
		s32 y = sy + 0x800;

		if (!isCoordValid[x][y])
			return false;

		if ((std::fabs(gteCoords[x][y].x - sx) < 1.0f) &&
			(std::fabs(gteCoords[x][y].y - sy) < 1.0f))
		{
			vertex->x = gteCoords[x][y].x;
			vertex->y = gteCoords[x][y].y;
			return true;
		}
	}
	return false;
}

void GTEAccHack::AddGTEVertex(s16 sx, s16 sy, s64 fx, s64 fy, s64 fz)
{
	if (sx >= -0x800 && sx <= 0x7FF &&
		sy >= -0x800 && sy <= 0x7FF)
	{
		s32 x = sx + 0x800;
		s32 y = sy + 0x800;

		gteCoords[x][y].x = fx / 65536.0f;
		gteCoords[x][y].y = fy / 65536.0f;

		isCoordValid[x][y] = is_dirty = true;
	}
}

void GTEAccHack::fix_offsets(s32 count)
{
	//PLUGINLOG("GTE Accuracy offset%d", _case);
	//PLUGINLOG("PSXDisplay_CumulOffset_x %d PSXDisplay_CumulOffset_y %d", *m_gtedata.PSXDisplay_CumulOffset_x, *m_gtedata.PSXDisplay_CumulOffset_y);

	//for (int i = 0; i < count; ++i)

	concurrency::parallel_for(0, count, 1, [&](const int& i)
	{
		if (GetGTEVertex(*lx[i], *ly[i], vertex[i]))
		{
			vertex[i]->x += *PSXDisplay_CumulOffset_x;
			vertex[i]->y += *PSXDisplay_CumulOffset_y;
		}
	}
	);
}

GTEAccHack::offset_fn GTEAccHack::ooffset3;
BOOL __cdecl GTEAccHack::offset3(void)
{
	BOOL ret = ooffset3();
	s_GTEAccHack->fix_offsets(3);
	return ret;
}

GTEAccHack::offset_fn GTEAccHack::ooffset4;
BOOL __cdecl GTEAccHack::offset4(void)
{
	BOOL ret = ooffset4();
	s_GTEAccHack->fix_offsets(4);
	return ret;
}
