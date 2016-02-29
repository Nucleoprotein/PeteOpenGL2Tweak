// gpuPeteOGL2Fixes.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include "gpuPeteOpenGL2Tweak.h"
#include "GPUPlugin.h"

//#define EXPORT_LOG(format, ...) PLUGINLOG(format, __VA_ARGS__)
#define EXPORT_LOG(format, ...) 

const char* CALLBACK EXPORT_PSEgetLibName(void)
{
    EXPORT_LOG(__FUNCTION__);
	return libraryName;
}

u32 CALLBACK EXPORT_PSEgetLibType(void)
{
    EXPORT_LOG(__FUNCTION__);
	return  2;
}

u32 CALLBACK EXPORT_PSEgetLibVersion(void)
{
    EXPORT_LOG(__FUNCTION__);
    return 1 << 16 | VERSION_MAJOR << 8 | VERSION_MINOR;
}

s32 CALLBACK EXPORT_GPUinit()
{
    EXPORT_LOG(__FUNCTION__);
    return context.OnGPUinit();
}

s32 CALLBACK EXPORT_GPUshutdown()
{
    EXPORT_LOG(__FUNCTION__);
    return GPUPlugin::Get().GPUshutdown();
}

s32 CALLBACK EXPORT_GPUopen(HWND hwndGPU)
{
    EXPORT_LOG(__FUNCTION__);
	return GPUPlugin::Get().GPUopen(hwndGPU);
}

s32 CALLBACK EXPORT_GPUclose()
{
    EXPORT_LOG(__FUNCTION__);
    return context.OnGPUclose();
}

s32 CALLBACK EXPORT_GPUconfigure()
{
    EXPORT_LOG(__FUNCTION__);
	return GPUPlugin::Get().GPUconfigure();
}

void CALLBACK EXPORT_GPUabout()
{
    EXPORT_LOG(__FUNCTION__);
    return GPUPlugin::Get().GPUabout();
}

s32 CALLBACK EXPORT_GPUtest()
{
    EXPORT_LOG(__FUNCTION__);
    return GPUPlugin::Get().GPUtest();
}

u32 CALLBACK EXPORT_GPUreadStatus()
{
    EXPORT_LOG(__FUNCTION__);
	return GPUPlugin::Get().GPUreadStatus();
}

u32 CALLBACK EXPORT_GPUreadData()
{
    EXPORT_LOG(__FUNCTION__);
	return context.OnGPUreadData();
}

void CALLBACK EXPORT_GPUreadDataMem(u32* pMem, s32 iSize)
{
    EXPORT_LOG(__FUNCTION__);
	return context.OnGPUreadDataMem(pMem, iSize);
}

void CALLBACK EXPORT_GPUwriteStatus(u32 gdata)
{
    EXPORT_LOG(__FUNCTION__);
	return GPUPlugin::Get().GPUwriteStatus(gdata);
}

void CALLBACK EXPORT_GPUwriteData(u32 gdata)
{
    EXPORT_LOG(__FUNCTION__);
	return GPUPlugin::Get().GPUwriteData(gdata);
}

void CALLBACK EXPORT_GPUwriteDataMem(u32* pMem, s32 iSize)
{
    EXPORT_LOG(__FUNCTION__);
	return GPUPlugin::Get().GPUwriteDataMem(pMem, iSize);
}

s32 CALLBACK EXPORT_GPUgetMode()
{
    EXPORT_LOG(__FUNCTION__);
	return GPUPlugin::Get().GPUgetMode();
}

void CALLBACK EXPORT_GPUsetMode(u32 gdata)
{
    EXPORT_LOG(__FUNCTION__);
	return GPUPlugin::Get().GPUsetMode(gdata);
}

void CALLBACK EXPORT_GPUupdateLace()
{
    EXPORT_LOG(__FUNCTION__);
	return GPUPlugin::Get().GPUupdateLace();
}

s32 CALLBACK EXPORT_GPUdmaChain(u32 * baseAddrL, u32 addr)
{
    EXPORT_LOG(__FUNCTION__);
	return GPUPlugin::Get().GPUdmaChain(baseAddrL, addr);
}

void CALLBACK EXPORT_GPUmakeSnapshot()
{
    EXPORT_LOG(__FUNCTION__);
	return GPUPlugin::Get().GPUmakeSnapshot();
}

void CALLBACK EXPORT_GPUkeypressed(u32 key)
{
    EXPORT_LOG(__FUNCTION__);
	return GPUPlugin::Get().GPUkeypressed(key);
}

void CALLBACK EXPORT_GPUdisplayText(s8 * text)
{
    EXPORT_LOG(__FUNCTION__);
	return GPUPlugin::Get().GPUdisplayText(text);
}

void CALLBACK EXPORT_GPUdisplayFlags(u32 dwFlags)
{
    EXPORT_LOG(__FUNCTION__);
	return GPUPlugin::Get().GPUdisplayFlags(dwFlags);
}

s32 CALLBACK EXPORT_GPUfreeze(u32 ulGetFreezeData, struct GPUFreeze_t* pF)
{
    EXPORT_LOG(__FUNCTION__);
	return GPUPlugin::Get().GPUfreeze(ulGetFreezeData, pF);
}

s32 CALLBACK EXPORT_GPUgetScreenPic(u8 * pMem)
{
    EXPORT_LOG(__FUNCTION__);
	return GPUPlugin::Get().GPUgetScreenPic(pMem);
}

s32 CALLBACK EXPORT_GPUshowScreenPic(u8 * pMem)
{
    EXPORT_LOG(__FUNCTION__);
	return GPUPlugin::Get().GPUshowScreenPic(pMem);
}

void CALLBACK EXPORT_GPUclearDynarec(void(CALLBACK *callback)(void))
{
    EXPORT_LOG(__FUNCTION__);
	return GPUPlugin::Get().GPUclearDynarec(callback);
}

void CALLBACK EXPORT_GPUhSync(s32 val)
{
    EXPORT_LOG(__FUNCTION__);
	return GPUPlugin::Get().GPUhSync(val);
}

void CALLBACK EXPORT_GPUvBlank(s32 val)
{
    EXPORT_LOG(__FUNCTION__);
	return GPUPlugin::Get().GPUvBlank(val);
}

void CALLBACK EXPORT_GPUvisualVibration(u32 iSmall, u32 iBig)
{
    EXPORT_LOG(__FUNCTION__);
	return GPUPlugin::Get().GPUvisualVibration(iSmall, iBig);
}

void CALLBACK EXPORT_GPUcursor(s32 iPlayer, s32 x, s32 y)
{
    EXPORT_LOG(__FUNCTION__);
	return GPUPlugin::Get().GPUcursor(iPlayer, x, y);
}

void CALLBACK EXPORT_GPUaddVertex(s16 sx, s16 sy, s64 fx, s64 fy, s64 fz)
{
    EXPORT_LOG(__FUNCTION__);
    return context.OnGPUaddVertex(sx, sy, fx, fy, fz);
}

s32 CALLBACK EXPORT_GPUgetVertex(s16 sx, s16 sy, u16 z, float* fx, float* fy)
{
	EXPORT_LOG(__FUNCTION__);
	return context.OnGPUgetVertex(sx, sy, z, fx, fy);
}

void CALLBACK EXPORT_GPUclearVertex(s16 sx, s16 sy, u16 z)
{
	EXPORT_LOG(__FUNCTION__);
	return context.OnGPUclearVertex(sx, sy, z);
}

void CALLBACK EXPORT_GPUsetfix(u32 dwFixBits)
{
    EXPORT_LOG(__FUNCTION__);
	return GPUPlugin::Get().GPUsetfix(dwFixBits);
}

void CALLBACK EXPORT_GPUsetframelimit(u32 option)
{
    EXPORT_LOG(__FUNCTION__);
	return context.OnGPUsetframelimit(option);
}
