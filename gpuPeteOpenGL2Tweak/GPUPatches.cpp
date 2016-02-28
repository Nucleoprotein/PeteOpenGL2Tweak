#include "stdafx.h"

#include "gpuPeteOpenGL2Tweak.h"
#include "GPUPlugin.h"
#include "SafeWrite.h"

#include "GPUPatches.h"

static GPUPatches* s_GPUPatches;

GPUPatches::GPUPatches()
{
	PLUGINLOG("Pete OpenGL2 Tweak Enabled");

	s_GPUPatches = this;

	CheckTextureMemory = (CheckTextureMemory_fn)GPUPlugin::Get().GetPluginMem(0x000448B0);
	locVRamSize = (int*)GPUPlugin::Get().GetPluginMem(0x00050224);

	nopX = (u16*)GPUPlugin::Get().GetPluginMem(0x0000942E);
	nopY = (u16*)GPUPlugin::Get().GetPluginMem(0x00009449);

	mulX = (u8*)GPUPlugin::Get().GetPluginMem(0x00009431);
	mulY = (u8*)GPUPlugin::Get().GetPluginMem(0x0000944C);

	locX = (u32*)GPUPlugin::Get().GetPluginMem(0x0004FA70);
	locY = (u32*)GPUPlugin::Get().GetPluginMem(0x0004FA74);

	iWindowned = (s32*)GPUPlugin::Get().GetPluginMem(0x00052120);
	iWinSize = (s32*)GPUPlugin::Get().GetPluginMem(0x00052124);

	iResX = (s32*)GPUPlugin::Get().GetPluginMem(0x000522A0);
	iResY = (s32*)GPUPlugin::Get().GetPluginMem(0x000522A4);
}

GPUPatches::~GPUPatches()
{
}

int GPUPatches::GetVideoMemoryAMD()
{
	if (!WGLEW_AMD_gpu_association) return 0;

	size_t total_mem_mb = 0;
	if (wglGetGPUIDsAMD && wglGetGPUInfoAMD)
	{
		UINT n = wglGetGPUIDsAMD(0, 0);
		std::vector<UINT>ids(n);

		wglGetGPUIDsAMD(n, &ids[0]);
		wglGetGPUInfoAMD(ids[0], WGL_GPU_RAM_AMD, GL_UNSIGNED_INT, sizeof(size_t), &total_mem_mb);
	}

	PLUGINLOG("AMD GPU VRAM: %uMB", total_mem_mb);
	return (int)total_mem_mb;
};

int GPUPatches::GetVideoMemoryNV()
{
	if (!GLEW_NVX_gpu_memory_info) return 0;

	GLint total_mem_kb = 0;
	glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &total_mem_kb);

	int iVRamSize = total_mem_kb / 1024;

	PLUGINLOG("NV GPU VRAM: %dMB", iVRamSize);
	return iVRamSize;
}

void GPUPatches::FixMemoryDetection()
{
	if (CheckTextureMemory && locVRamSize)
	{
		// GetVideoMemoryAMD will return 0 on non AMD platforms
		int iVRamSize = GetVideoMemoryAMD();
		if (!iVRamSize) iVRamSize = GetVideoMemoryNV();

		if (iVRamSize > 512) iVRamSize = 512; //clamp memory to 512MB, more does nothing
		if (iVRamSize)
		{
			SafeWrite(locVRamSize, iVRamSize);
			//now recheck usable textures
			PLUGINLOG("CheckTextureMemory: %dMB", iVRamSize);
			CheckTextureMemory();
		}
	}
}

void GPUPatches::EnableVsync(s32 interval)
{
	if (!WGLEW_EXT_swap_control) return;

	if (wglSwapIntervalEXT)
	{
		if (wglSwapIntervalEXT(interval))
			PLUGINLOG("SwapInterval set to %d", interval);
		else
			PLUGINLOG("Fail to set SwapInterval to %d", interval);
	}
}

void GPUPatches::ResHack(u32 _x, u32 _y)
{
	if (!nopX || !nopY || !mulX || !mulY)
		return;

	u8 x = (u8)_x;
	u8 y = (u8)_y;

	if (x == 0) x = 1;
	if (y == 0) y = 1;

	PLUGINLOG("ResHack (%u %u)", x, y);

	SafeWrite<u16>(nopX, 0x9090);
	SafeWrite<u16>(nopY, 0x9090);

	SafeWrite<u8>(mulX, x);
	SafeWrite<u8>(mulY, y);
}

void GPUPatches::FixFullscreenAspect()
{
	if (*iWindowned)
		return;

	int w = GetSystemMetrics(SM_CXSCREEN);
	int h = GetSystemMetrics(SM_CYSCREEN);

	*(this->iResX) = (s32)(w * 0.75f);
	*(this->iResY) = h;

	*locX = (w - *(this->iResX)) / 2;
	*locY = 0;

	*iWinSize = MAKELONG(iResX, iResY);

	PLUGINLOG("FixFullscreenAspect");
}

LRESULT CALLBACK GPUPatches::TweakWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_KEYDOWN:
	case WM_KEYUP:
		if (context.GetConfig()->GetHardcoreMode() &&
			(wParam == VK_F1 ||
				wParam == VK_F2 ||
				wParam == VK_F3))
		{
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
	}

	return CallWindowProc(s_GPUPatches->oldWndProc, hWnd, uMsg, wParam, lParam);
}

void GPUPatches::ApplyWindowProc(HWND hWnd)
{
	if (!oldWndProc)
		oldWndProc = (WNDPROC)SetWindowLong(hWnd, GWL_WNDPROC, (LONG)TweakWindowProc);
}
