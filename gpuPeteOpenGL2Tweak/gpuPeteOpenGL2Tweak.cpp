
#include "stdafx.h"
#include "gpuPeteOpenGL2Tweak.h"
#include "GPUPlugin.h"
#include "GPUPatches.h"
#include "GTEAccHack.h"
#include "TextureScaler.h"

Context Context::instance;

Context::Context()
{
	MH_Initialize();
	config.reset(new Config);
	gpuPatches.reset(new GPUPatches);
}

Context::~Context()
{
	MH_Uninitialize();
}

s32 Context::OnGPUinit()
{
	CreateHook(GPUPlugin::Get().GPUopen, Hook_GPUopen, reinterpret_cast<void**>(&oGPUopen));
	EnableHook(GPUPlugin::Get().GPUopen);

	if (config->GetxBRZScale() > 1)
	{
		if (!textureScaler)
			textureScaler.reset(new TextureScaler);
	}

	if (config->GetMulX() > 0 && config->GetMulY() > 0)
		gpuPatches->ResHack(config->GetMulX(), config->GetMulY());

	return GPUPlugin::Get().GPUinit();
}

s32(CALLBACK* Context::oGPUopen)(HWND hwndGPU);
s32 CALLBACK Context::Hook_GPUopen(HWND hwndGPU)
{
	s32 ret = oGPUopen(hwndGPU);

	static std::once_flag glewInitFlag;
	std::call_once(glewInitFlag, glewInit);

	if (context.GetConfig()->GetFixFullscreenAspect())
		context.gpuPatches->FixFullscreenAspect();

	if (context.GetConfig()->GetFixMemoryDetection())
		context.gpuPatches->FixMemoryDetection();

	HWND insertAfter = HWND_TOP;
	UINT flags = SWP_NOSIZE | SWP_NOMOVE | SWP_NOCOPYBITS | SWP_NOSENDCHANGING;

	if (context.GetConfig()->GetWindowOnTop())
		insertAfter = HWND_TOPMOST;

	if (context.GetConfig()->GetWindowX() > -1 && context.GetConfig()->GetWindowY() > -1)
		flags &= ~SWP_NOMOVE;

	SetWindowPos(hwndGPU, insertAfter, context.GetConfig()->GetWindowX(), context.GetConfig()->GetWindowY(), 0, 0, flags);
	context.gpuPatches->ApplyWindowProc(hwndGPU);

	if (context.GetConfig()->GetVSyncInterval())
		context.gpuPatches->EnableVsync(context.GetConfig()->GetVSyncInterval());

	if (context.GetConfig()->GetHideCursor())
		while (ShowCursor(FALSE) > -1);

	return ret;
}

s32 Context::OnGPUclose()
{
	if (config->GetVSyncInterval())
		gpuPatches->EnableVsync(0);

	if (config->GetHideCursor()) while (ShowCursor(TRUE) < 0);
	return GPUPlugin::Get().GPUclose();
}

u32 Context::OnGPUreadData()
{
	if (gteAccHack && config->GetGTECacheClear())
		gteAccHack->ResetGTECache(true);

	return GPUPlugin::Get().GPUreadData();
}
void Context::OnGPUreadDataMem(u32* pMem, s32 iSize)
{
	if (gteAccHack && config->GetGTECacheClear())
		gteAccHack->ResetGTECache(false);

	return GPUPlugin::Get().GPUreadDataMem(pMem, iSize);
}

void Context::OnGPUsetframelimit(u32 option)
{
	if (config->GetDisableSetFrameLimit())
		return;

	return GPUPlugin::Get().GPUsetframelimit(option);
}

void Context::OnGPUaddVertex(s16 sx, s16 sy, s64 fx, s64 fy, s64 fz)
{
	if (config->GetGTEAccuracy())
	{
		if (!gteAccHack)
			gteAccHack.reset(new GTEAccHack);

		//PLUGINLOG("GPUaddVertex: sx = %hu sy = %hu fx = %llu fy = %llu fz = %llu", sx, sy, fx, fy, fz);
		return gteAccHack->AddGTEVertex(sx, sy, fx, fy, fz);
	}
}

s32 Context::OnGPUgetVertex(s16 sx, s16 sy, u16 z, float * fx, float * fy)
{
	if (gteAccHack)
		return gteAccHack->GetGTEVertex(sx, sy, z, fx, fy);
	return 0;
}

void Context::OnGPUclearVertex(s16 sx, s16 sy, u16 z)
{
	if (gteAccHack)
		return gteAccHack->ClearGTEVertex(sx, sy, z);
}
