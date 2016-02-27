
#include "stdafx.h"

#include "gpuPeteOpenGL2Tweak.h"
#include "GPUPlugin.h"

Context Context::instance;

Context::Context()
{
	MH_Initialize();
}

Context::~Context()
{
	MH_Uninitialize();
}

s32 Context::OnGPUinit()
{
	CreateHook(GPUPlugin::Get().GPUopen, Hook_GPUopen, reinterpret_cast<void**>(&oGPUopen));
	EnableHook(GPUPlugin::Get().GPUopen);

	PLUGINLOG("Pete OpenGL2 Tweak Enabled");
	if (m_config.GetGTEAccuracy())
		m_gpupatches.EnableGTEAccuracy();

	u32 scale = clamp<u32>(m_config.GetxBRZScale(), 1, 6);
	m_gpupatches.EnableTextureScaler(scale, m_config.GetBatchSize(), m_config.GetForceNearest(), m_config.GetFastFBE(), m_config.GetTextureCacheSize());

	if (m_config.GetMulX() > 0 && m_config.GetMulY() > 0)
		m_gpupatches.ResHack(m_config.GetMulX(), m_config.GetMulY());

	return GPUPlugin::Get().GPUinit();
}

s32(CALLBACK* Context::oGPUopen)(HWND hwndGPU);
s32 CALLBACK Context::Hook_GPUopen(HWND hwndGPU)
{
	s32 ret = oGPUopen(hwndGPU);

	static std::once_flag glewInitFlag;
	std::call_once(glewInitFlag, glewInit);

	if (context.GetConfig().GetFixFullscreenAspect())
		context.GetPatches().FixFullscreenAspect();

	if (context.GetConfig().GetFixMemoryDetection())
		context.GetPatches().FixMemoryDetection();

	HWND insertAfter = HWND_TOP;
	UINT flags = SWP_NOSIZE | SWP_NOMOVE | SWP_NOCOPYBITS | SWP_NOSENDCHANGING;

	if (context.GetConfig().GetWindowOnTop())
		insertAfter = HWND_TOPMOST;

	if (context.GetConfig().GetWindowX() > -1 && context.GetConfig().GetWindowY() > -1)
		flags &= ~SWP_NOMOVE;

	SetWindowPos(hwndGPU, insertAfter, context.GetConfig().GetWindowX(), context.GetConfig().GetWindowY(), 0, 0, flags);
	context.GetPatches().ApplyWindowProc(hwndGPU);

	if (context.GetConfig().GetVSyncInterval())
		context.GetPatches().EnableVsync(context.GetConfig().GetVSyncInterval());

	if (context.GetConfig().GetHideCursor())
		while (ShowCursor(FALSE) > -1);

	return ret;
}

s32 Context::OnGPUshutdown()
{
	return GPUPlugin::Get().GPUshutdown();
}

s32 Context::OnGPUclose()
{
	if (m_config.GetVSyncInterval())
		m_gpupatches.EnableVsync(0);

	if (m_config.GetHideCursor()) while (ShowCursor(TRUE) < 0);
	return GPUPlugin::Get().GPUclose();
}

s32 Context::OnGPUtest()
{
	return GPUPlugin::Get().GPUtest();
}

void Context::OnGPUreadDataMem(u32* pMem, s32 iSize)
{
	if (m_config.GetGTEAccuracy() && m_config.GetGTECacheClear())
		m_gpupatches.ResetGTECache();
	return GPUPlugin::Get().GPUreadDataMem(pMem, iSize);
}

void Context::OnGPUsetframelimit(u32 option)
{
	if (m_config.GetDisableSetFrameLimit())
		return;

	return GPUPlugin::Get().GPUsetframelimit(option);
}

void Context::OnGPUaddVertex(s16 sx, s16 sy, s64 fx, s64 fy, s64 fz)
{
	static std::once_flag flag;
	std::call_once(flag, []()
	{
		PLUGINLOG("GTE Accuracy Hack Enabled");
	});

	if (m_config.GetGTEAccuracy())
	{
		GTEAccuracy& gteacc = m_gpupatches.GetGTEAccuracy();
		//PLUGINLOG("GPUaddVertex: sx = %hu sy = %hu fx = %llu fy = %llu fz = %llu", sx, sy, fx, fy, fz);
		return gteacc.set(sx, sy, fx, fy, fz);
	}


}