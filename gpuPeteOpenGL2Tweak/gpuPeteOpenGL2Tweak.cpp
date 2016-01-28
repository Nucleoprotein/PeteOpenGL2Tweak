
#include "stdafx.h"

#include "PADPlugin.h"
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

void Context::LoadPad()
{
	static std::once_flag flag;
	std::call_once(flag, [&]()
	{
		if (m_config.GetPort1().empty() &&
			m_config.GetPort2().empty())
			return;
		if (!m_padplugin.Hook())
			return;

		m_padplugin.Load(m_config.GetPort1(), m_config.GetPort2());
	});

	m_padplugin.OnPADinit();
}

s32 Context::OnGPUinit()
{
	MH_CreateHook(GPUPlugin::Get().GPUopen, Hook_GPUopen, reinterpret_cast<void**>(&oGPUopen));
	MH_EnableHook(GPUPlugin::Get().GPUopen);

	PLUGINLOG("Pete OpenGL2 Tweak Enabled");
	if (m_config.GetGTEAccuracy())
		m_gpupatches.GTEAccuracy();

	u32 scale = clamp<u32>(m_config.GetxBRZScale(), 1, 6);
	m_gpupatches.TextureScale(scale, m_config.GetSliceSize(), m_config.GetForceNearest(), m_config.GetFastFBE(), m_config.GetTextureCacheSize());

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

	UINT flags = SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER;

	if (context.GetConfig().GetWindowOnTop())
		flags &= ~SWP_NOZORDER;

	if (context.GetConfig().GetWindowX() > -1 && context.GetConfig().GetWindowY() > -1)
		flags &= ~SWP_NOMOVE;

	SetWindowPos(hwndGPU, HWND_TOPMOST, context.GetConfig().GetWindowX(), context.GetConfig().GetWindowY(), 0, 0, flags);
	context.GetPatches().ApplyWindowProc(hwndGPU);

	if (context.GetConfig().GetVSyncInterval())
		context.GetPatches().EnableVsync(context.GetConfig().GetVSyncInterval());

	if (context.GetConfig().GetGTEAccuracy())
		resetGteVertices();

	if (context.GetConfig().GetHideCursor())
		while (ShowCursor(FALSE) > -1);

	context.LoadPad();
	context.GetPadPlugin().OnPADopen(hwndGPU);
	return ret;
}

s32  Context::OnGPUshutdown()
{
	m_padplugin.OnPADshutdown();
	return GPUPlugin::Get().GPUshutdown();
}

s32 Context::OnGPUclose()
{
	if (m_config.GetVSyncInterval())
		m_gpupatches.EnableVsync(0);

	m_padplugin.OnPADclose();

	if (m_config.GetHideCursor()) while (ShowCursor(TRUE) < 0);
	return GPUPlugin::Get().GPUclose();
}

s32 Context::OnGPUtest()
{
	if (!m_config.GetPort1().empty() ||
		!m_config.GetPort2().empty())
	{
		m_padplugin.Load(m_config.GetPort1(), m_config.GetPort2());

		m_padplugin.OnPADinit();
		m_padplugin.OnPADconfigure();
	}

	return GPUPlugin::Get().GPUtest();
}

void  Context::OnGPUaddVertex(s16 sx, s16 sy, s64 fx, s64 fy, s64 fz)
{
	static std::once_flag flag;
	std::call_once(flag, [&]()
	{
		PLUGINLOG("GTE Accuracy Hack Enabled");
	});

	//PLUGINLOG("GPUaddVertex: sx = %hu sy = %hu fx = %llu fy = %llu fz = %llu", sx, sy, fx, fy, fz);
	if (m_config.GetGTEAccuracy())
		return GPUaddVertex(sx, sy, fx, fy, fz);
}