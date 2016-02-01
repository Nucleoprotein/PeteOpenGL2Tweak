#include "stdafx.h"

#include "gpuPeteOpenGL2Tweak.h"
#include "GPUPlugin.h"
#include "SafeWrite.h"
#include "gte_accuracy.h"

#include "GPUPatches.h"

#include "xxhash.h"

#include "xBRZ\xbrz.h"
#include "deposterize.h"
#include <ppl.h>

static GPUPatches* s_pGPUPatches;

GPUPatches::GPUPatches()
{
	s_pGPUPatches = this;

	m_gtedata.lx[0] = (s16*)GPUPlugin::Get().GetPluginMem(0x00051A08);
	m_gtedata.lx[1] = (s16*)GPUPlugin::Get().GetPluginMem(0x00051A0A);
	m_gtedata.lx[2] = (s16*)GPUPlugin::Get().GetPluginMem(0x00051A0C);
	m_gtedata.lx[3] = (s16*)GPUPlugin::Get().GetPluginMem(0x00051A0E);

	m_gtedata.ly[0] = (s16*)GPUPlugin::Get().GetPluginMem(0x00051A10);
	m_gtedata.ly[1] = (s16*)GPUPlugin::Get().GetPluginMem(0x00051A12);
	m_gtedata.ly[2] = (s16*)GPUPlugin::Get().GetPluginMem(0x00051A14);
	m_gtedata.ly[3] = (s16*)GPUPlugin::Get().GetPluginMem(0x00051A16);

	m_gtedata.vertex[0] = (OGLVertex*)GPUPlugin::Get().GetPluginMem(0x00052220);
	m_gtedata.vertex[1] = (OGLVertex*)GPUPlugin::Get().GetPluginMem(0x00052238);
	m_gtedata.vertex[2] = (OGLVertex*)GPUPlugin::Get().GetPluginMem(0x00052250);
	m_gtedata.vertex[3] = (OGLVertex*)GPUPlugin::Get().GetPluginMem(0x00052268);

	m_gtedata.PSXDisplay_CumulOffset_x = (s16*)GPUPlugin::Get().GetPluginMem(0x00051FFC);
	m_gtedata.PSXDisplay_CumulOffset_y = (s16*)GPUPlugin::Get().GetPluginMem(0x00051FFE);

	CheckTextureMemory = (fCheckTextureMemory)GPUPlugin::Get().GetPluginMem(0x000448B0);
	locVRamSize = (u32*)GPUPlugin::Get().GetPluginMem(0x00050224);

	nopX = (u16*)GPUPlugin::Get().GetPluginMem(0x0000942E);
	nopY = (u16*)GPUPlugin::Get().GetPluginMem(0x00009449);

	mulX = (u8*)GPUPlugin::Get().GetPluginMem(0x00009431);
	mulY = (u8*)GPUPlugin::Get().GetPluginMem(0x0000944C);

	locW = (u32*)GPUPlugin::Get().GetPluginMem(0x000522A0);
	locH = (u32*)GPUPlugin::Get().GetPluginMem(0x000522A4);

	locX = (u32*)GPUPlugin::Get().GetPluginMem(0x0004FA70);
	locY = (u32*)GPUPlugin::Get().GetPluginMem(0x0004FA74);

	locWinSize = (u32*)GPUPlugin::Get().GetPluginMem(0x00052124);
	locWindowned = (BOOL*)GPUPlugin::Get().GetPluginMem(0x00052120);

	//fps flt_1004FFD8
	locFPS = (float*)GPUPlugin::Get().GetPluginMem(0x0004FFD8);

	//fbe playing = dword_100500E0
	locFBE = (u32*)GPUPlugin::Get().GetPluginMem(0x000500E0);
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

		if (iVRamSize > 1024) iVRamSize = 1024; //clamp memory to 512MB, more does nothing
		if (iVRamSize)
		{
			SafeWrite<u32>(locVRamSize, iVRamSize);
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

void GPUPatches::fix_offsets(s32 count)
{
	//PLUGINLOG("GTE Accuracy offset%d", _case);
	//PLUGINLOG("PSXDisplay_CumulOffset_x %d PSXDisplay_CumulOffset_y %d", *m_gtedata.PSXDisplay_CumulOffset_x, *m_gtedata.PSXDisplay_CumulOffset_y);

	concurrency::parallel_for(0, count, 1, [&](const int& i)
	{
		if (getGteVertex(*m_gtedata.lx[i], *m_gtedata.ly[i], m_gtedata.vertex[i]))
		{
			m_gtedata.vertex[i]->x += *m_gtedata.PSXDisplay_CumulOffset_x;
			m_gtedata.vertex[i]->y += *m_gtedata.PSXDisplay_CumulOffset_y;
		}
	}
	);
}

GPUPatches::tOffset GPUPatches::ooffset3;
BOOL __cdecl GPUPatches::offset3(void)
{
	BOOL ret = ooffset3();
	s_pGPUPatches->fix_offsets(3);
	return ret;
}

GPUPatches::tOffset GPUPatches::ooffset4;
BOOL __cdecl GPUPatches::offset4(void)
{
	BOOL ret = ooffset4();
	s_pGPUPatches->fix_offsets(4);
	return ret;
}

GPUPatches::tOffset GPUPatches::ooffsetST;
BOOL __cdecl GPUPatches::offsetST(void)
{
	s_pGPUPatches->fix_offsets(4);
	BOOL ret = ooffsetST();
	return ret;
}

GPUPatches::tOffset GPUPatches::ooffsetline;
BOOL __cdecl GPUPatches::offsetline(void)
{
	BOOL ret = ooffsetline();
	s_pGPUPatches->fix_offsets(4);
	return ret;
}

void GPUPatches::GTEAccuracy()
{
	if (!ooffset3 && !ooffset4)
	{
		//offsetline = signed int __cdecl sub_10003DC0()
		//offset3 = signed int __cdecl sub_100041B0()
		//offset4 = signed int __cdecl sub_100043F0()
		//offsetST = signed int __cdecl sub_10004780()
		//??????? = int __cdecl sub_100048B0()
		//??????? = int __cdecl sub_10004990()

		tOffset _offset3 = (tOffset)GPUPlugin::Get().GetPluginMem(0x000041B0);
		tOffset _offset4 = (tOffset)GPUPlugin::Get().GetPluginMem(0x000043F0);

		//tOffset _offsetST = (tOffset)GPUPlugin::Get().GetPluginMem(0x00004780);
		//MH_CreateHook(_offsetST, offsetST, reinterpret_cast<void**>(&ooffsetST));
		//MH_EnableHook(_offsetST);

		//tOffset _offsetline = (tOffset)GPUPlugin::Get().GetPluginMem(0x00003DC0);
		//MH_CreateHook(_offsetline, offsetline, reinterpret_cast<void**>(&ooffsetline));
		//MH_EnableHook(_offsetline);


		MH_CreateHook(_offset3, offset3, reinterpret_cast<void**>(&ooffset3));
		MH_EnableHook(_offset3);

		MH_CreateHook(_offset4, offset4, reinterpret_cast<void**>(&ooffset4));
		MH_EnableHook(_offset4);
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
	if (locWindowned && *locWindowned)
		return;

	int w = (int)(GetSystemMetrics(SM_CXSCREEN) * 0.75);
	int h = GetSystemMetrics(SM_CYSCREEN);

	int x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
	int y = 0;

	PLUGINLOG("FixFullscreenAspect %d %d %d %d", x, y, w, h);

	SafeWrite<u32>(locW, w);
	SafeWrite<u32>(locH, h);

	SafeWrite<u32>(locX, x);
	SafeWrite<u32>(locY, y);

	//SafeWrite<u32>(locWinSize, MAKELONG(w, h));
}

LRESULT CALLBACK GPUPatches::TweakWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_KEYDOWN:
	case WM_KEYUP:
		if (context.GetConfig().GetHardcoreMode() &&
			(wParam == VK_F1 ||
				wParam == VK_F2 ||
				wParam == VK_F3))
		{
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}

	}

	return CallWindowProc(s_pGPUPatches->oldWndProc, hWnd, uMsg, wParam, lParam);
}

void GPUPatches::ApplyWindowProc(HWND hWnd)
{
	if (!oldWndProc)
		oldWndProc = (WNDPROC)SetWindowLong(hWnd, GWL_WNDPROC, (LONG)TweakWindowProc);
}

void (APIENTRY* GPUPatches::oglCopyTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
void APIENTRY GPUPatches::Hook_glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
	//PLUGINLOG("glCopyTexSubImage2D 0x%08X %d %d %d %d %d %d %d", target, level, xoffset, yoffset, x, y, width, height);
	if (target != GL_TEXTURE_2D)
		return oglCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);

	if (width >= MAX_TEXTURE_X || height >= MAX_TEXTURE_Y)
	{
		PLUGINLOG("glCopyTexSubImage2D !!!Texture too big !!!");
		return oglCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
	}

	return oglCopyTexSubImage2D(target, level, xoffset * s_pGPUPatches->m_scale, yoffset * s_pGPUPatches->m_scale, x, y, width * s_pGPUPatches->m_scale, height * s_pGPUPatches->m_scale);
}

void (APIENTRY* GPUPatches::oglTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
void APIENTRY GPUPatches::Hook_glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels)
{
	//PLUGINLOG("glTexSubImage2D 0x%08X %d %d %d %d %d 0x%08X 0x%08X %p", target, level, xoffset, yoffset, width, height, format, type, pixels);
	if ((target != GL_TEXTURE_2D || format != GL_RGBA || width >= MAX_TEXTURE_X || height >= MAX_TEXTURE_Y))
	{
		return oglTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
	}

	if (format == GL_RGBA)
	{
		std::vector<u32> textureBuffer = s_pGPUPatches->ScaleTexture((u32*)pixels, width, height);
		return oglTexSubImage2D(target, level, xoffset * s_pGPUPatches->m_scale, yoffset * s_pGPUPatches->m_scale, width * s_pGPUPatches->m_scale, height * s_pGPUPatches->m_scale, format, type, textureBuffer.data());
	}
	return oglTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
}

void (APIENTRY* GPUPatches::oglTexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
void APIENTRY GPUPatches::Hook_glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels)
{
	if (target == GL_TEXTURE_2D && format == GL_RGBA && width >= MAX_TEXTURE_X && height >= MAX_TEXTURE_Y)
		PLUGINLOG("ResHack: Detected render surface, size: %d x %d x 32bpp, %d MiB", width, height, (width * height * 4) / (1024 * 1024));

	//PLUGINLOG("glTexImage2D 0x%08X %d %d %d 0x%08X 0x%08X %p", target, level, width, height, format, type, pixels);
	if (target != GL_TEXTURE_2D || format != GL_RGBA || width >= MAX_TEXTURE_X || height >= MAX_TEXTURE_Y)
	{
		return oglTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
	}

	if (format == GL_RGBA)
	{
		std::vector<u32> textureBuffer = s_pGPUPatches->ScaleTexture((u32*)pixels, width, height);
		return oglTexImage2D(target, level, internalformat, width * s_pGPUPatches->m_scale, height * s_pGPUPatches->m_scale, border, format, type, textureBuffer.data());
	}
	return oglTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
}

std::vector<u32> GPUPatches::ScaleTexture(const u32* source, u32 srcWidth, u32 srcHeight)
{
	u32 texture_hash = XXH32(source, srcWidth * srcHeight * sizeof(u32), 0);
	//PLUGINLOG("texture_hash: %llu", texture_hash);

	if (m_texture_cache_size > 0 && texture_hash > 0 && !m_TextureCache[texture_hash].empty())
	{
		return m_TextureCache[texture_hash];
	}

	std::vector<u32> textureBuffer(srcWidth * m_scale * srcHeight * m_scale);

	if ((m_fast_fbe && *locFBE) || m_force_nearest)
	{
		concurrency::parallel_for(0, (int)srcHeight, (int)m_slice, [&](const int& i)
		{
			xbrz::nearestNeighborScale(source, srcWidth, srcHeight, srcWidth * sizeof(u32), textureBuffer.data(),
				srcWidth * m_scale, srcHeight * m_scale, srcWidth * m_scale * sizeof(u32), xbrz::SliceType::NN_SCALE_SLICE_SOURCE, i, i + m_slice);
		}
		);
	}
	else
	{
		if (context.GetConfig().GetDeposterize())
		{
			std::vector<u32> deposterizeBuffer(srcWidth * srcHeight);
			DePosterize(source, deposterizeBuffer.data(), srcWidth, srcHeight);
			source = deposterizeBuffer.data();

			concurrency::parallel_for(0, (int)srcHeight, (int)m_slice, [&](const int& i)
			{
				xbrz::scale(m_scale, source, textureBuffer.data(), srcWidth, srcHeight, xbrz::ColorFormat::ARGB, xbrz::ScalerCfg(), i, i + m_slice);
			}
			);
		}
		else
		{
			concurrency::parallel_for(0, (int)srcHeight, (int)m_slice, [&](const int& i)
			{
				xbrz::scale(m_scale, source, textureBuffer.data(), srcWidth, srcHeight, xbrz::ColorFormat::ARGB, xbrz::ScalerCfg(), i, i + m_slice);
			}
			);
		}

		if (m_texture_cache_size > 0 && texture_hash > 0)
		{
			m_TextureCacheTimestamp.push_back(texture_hash);
			m_TextureCache[texture_hash] = textureBuffer;

			//PLUGINLOG("cache size: %lu", m_TextureCache.size());
			//PLUGINLOG("cache time size: %lu", m_TextureCacheTimestamp.size());
			if (m_TextureCache.size() >= m_texture_cache_size)
			{
				//PLUGINLOG("removing hash %llu", m_TextureCacheTimestamp.front());
				m_TextureCache.erase(m_TextureCacheTimestamp.front());
				m_TextureCacheTimestamp.erase(m_TextureCacheTimestamp.begin());
				//PLUGINLOG("cache size: %lu", m_TextureCache.size());
			}
		}
	}
	return std::move(textureBuffer);
}

void GPUPatches::EnableTextureScaler(u32 scale, u32 slice, bool force_nearest, bool fast_fbe, u32 texture_cache_size)
{
	if (scale <= 1)
		return;

	static std::once_flag flag;
	std::call_once(flag, [&]()
	{
		m_scale = scale;
		m_slice = slice;
		m_force_nearest = force_nearest;
		m_fast_fbe = fast_fbe;
		m_texture_cache_size = texture_cache_size;

		PLUGINLOG("%ux%s Texture Filter, slice %u", scale, m_force_nearest ? " Nearest Neighbour" : "BRZ", slice);

		MH_CreateHook(glTexImage2D, Hook_glTexImage2D, reinterpret_cast<void**>(&oglTexImage2D));
		MH_EnableHook(glTexImage2D);

		MH_CreateHook(glTexSubImage2D, Hook_glTexSubImage2D, reinterpret_cast<void**>(&oglTexSubImage2D));
		MH_EnableHook(glTexSubImage2D);

		//MH_CreateHook(glCopyTexSubImage2D, Hook_glCopyTexSubImage2D, reinterpret_cast<void**>(&oglCopyTexSubImage2D));
		//MH_EnableHook(glCopyTexSubImage2D);
	});
}
