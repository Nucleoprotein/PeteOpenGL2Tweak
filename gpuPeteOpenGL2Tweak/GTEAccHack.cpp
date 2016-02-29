#include "stdafx.h"
#include "GTEAccHack.h"

#include "gpuPeteOpenGL2Tweak.h"
#include "GPUPlugin.h"

#include <ppl.h>

static GTEAccHack* s_GTEAccHack;

GTEAccHack::GTEAccHack()
{
	s_GTEAccHack = this;

	lx[0] = (s16*)GPUPlugin::Get().GetPluginMem(0x00051A08);
	lx[1] = (s16*)GPUPlugin::Get().GetPluginMem(0x00051A0A);
	lx[2] = (s16*)GPUPlugin::Get().GetPluginMem(0x00051A0C);
	lx[3] = (s16*)GPUPlugin::Get().GetPluginMem(0x00051A0E);

	ly[0] = (s16*)GPUPlugin::Get().GetPluginMem(0x00051A10);
	ly[1] = (s16*)GPUPlugin::Get().GetPluginMem(0x00051A12);
	ly[2] = (s16*)GPUPlugin::Get().GetPluginMem(0x00051A14);
	ly[3] = (s16*)GPUPlugin::Get().GetPluginMem(0x00051A16);

	vertex[0] = (OGLVertex*)GPUPlugin::Get().GetPluginMem(0x00052220);
	vertex[1] = (OGLVertex*)GPUPlugin::Get().GetPluginMem(0x00052238);
	vertex[2] = (OGLVertex*)GPUPlugin::Get().GetPluginMem(0x00052250);
	vertex[3] = (OGLVertex*)GPUPlugin::Get().GetPluginMem(0x00052268);

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

	PLUGINLOG("GTE Accuracy Hack Enabled");
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
		//isCoordValid.fill(0);
	}
}

void GTEAccHack::GetGTEVertex(s16 sx, s16 sy, OGLVertex* vertex)
{
	if (sx < -0x800 || sx >= 0x800 || sy < -0x800 || sy >= 0x800)
		return; // Out of range

	u16 x = sx + 0x800;
	u16 y = sy + 0x800;

	if (!isCoordValid[x][y])
		return;

	vertex->x = gteCoords[x][y].x + *PSXDisplay_CumulOffset_x;
	vertex->y = gteCoords[x][y].y + *PSXDisplay_CumulOffset_y;

	isCoordDrawn[x][y] = true;
}

s32 GTEAccHack::GetGTEVertex(s16 sx, s16 sy, u16 z, float* fx, float* fy)
{
	if (sx < -0x800 || sx >= 0x800 || sy < -0x800 || sy >= 0x800)
		return 0; // Out of range

	u16 x = sx + 0x800;
	u16 y = sy + 0x800;

	if (!isCoordValid[x][y] || gteCoords[x][y].z != z)
		return 0;

	//PLUGINLOG("%d %d %u", sx, sy, z);

	*fx = gteCoords[x][y].x;
	*fy = gteCoords[x][y].y;

	return 1;
}

float lerp(float x, float y, float s)
{
	return x*(1 - s) + y*s;
}

void GTEAccHack::AddGTEVertex(s16 sx, s16 sy, s64 llx, s64 lly, s64 llz)
{
	if (sx < -0x800 || sx >= 0x800 || sy < -0x800 || sy >= 0x800)
		return; // Out of range

	u16 x = sx + 0x800;
	u16 y = sy + 0x800;

	float fx = float(llx) / float(1 << 16);
	float fy = float(lly) / float(1 << 16);

	u16 z = (u16)llz;

	if (std::fabs(sx - fx) >= 1.0f || std::fabs(sy - fy) >= 1.0f)
		return;

	if (gteCoords[x][y].x == fx && gteCoords[x][y].y == fy && gteCoords[x][y].z == z)
		return;

#if 0
	if (isCoordValid[x][y] && gteCoords[x][y].z == z && !isCoordDrawn[x][y])
	{
		float lerpx = lerp(gteCoords[x][y].x, fx, 0.5);
		float lerpy = lerp(gteCoords[x][y].y, fy, 0.5);

		//PLUGINLOG("%f %f %f %f %f %f", gteCoords[x][y].x, gteCoords[x][y].y, fx, fy, lerpx, lerpy);

		gteCoords[x][y].x = lerpx;
		gteCoords[x][y].y = lerpy;
		gteCoords[x][y].z = z;

		isCoordValid[x][y] = is_dirty = true;
		isCoordDrawn[x][y] = false;

		return;
	}
#endif // 0


	gteCoords[x][y].x = fx;
	gteCoords[x][y].y = fy;
	gteCoords[x][y].z = z;

	isCoordValid[x][y] = is_dirty = true;
	isCoordDrawn[x][y] = false;
}

void GTEAccHack::ClearGTEVertex(s16 sx, s16 sy, u16 z)
{
	if (sx < -0x800 || sx >= 0x800 || sy < -0x800 || sy >= 0x800)
		return; // Out of range

	s32 x = sx + 0x800;
	s32 y = sy + 0x800;

	if (gteCoords[x][y].z == z && !isCoordDrawn[x][y])
	{
		//PLUGINLOG("%d %d %u", sx, sy, z);
		isCoordValid[x][y] = false;
	}
}

void GTEAccHack::fix_offsets(s32 count)
{
	concurrency::parallel_for(0, count, 1, [&](const int& i)
	{
		GetGTEVertex(*lx[i], *ly[i], vertex[i]);
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
