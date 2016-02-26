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
	locVRamSize = (int*)GPUPlugin::Get().GetPluginMem(0x00050224);

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

	//locFPS = (float*)GPUPlugin::Get().GetPluginMem(0x0004FFD8);

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

void GPUPatches::fix_offsets(s32 count)
{
	//PLUGINLOG("GTE Accuracy offset%d", _case);
	//PLUGINLOG("PSXDisplay_CumulOffset_x %d PSXDisplay_CumulOffset_y %d", *m_gtedata.PSXDisplay_CumulOffset_x, *m_gtedata.PSXDisplay_CumulOffset_y);

	//for (int i = 0; i < count; ++i)
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
GPUPatches::tprimMoveImage GPUPatches::oprimMoveImage;
void __cdecl GPUPatches::primMoveImage(unsigned char * baseAddr)
{
	resetGteVertices();
	PLUGINLOG("primMoveImage");
	return oprimMoveImage(baseAddr);
}

void GPUPatches::GTEAccuracy()
{
	tOffset offset3 = (tOffset)GPUPlugin::Get().GetPluginMem(0x000041B0);
	CreateHook(offset3, GPUPatches::offset3, &ooffset3);
	EnableHook(offset3);

	tOffset offset4 = (tOffset)GPUPlugin::Get().GetPluginMem(0x000043F0);
	CreateHook(offset4, GPUPatches::offset4, &ooffset4);
	EnableHook(offset4);

	//void primMoveImage(unsigned char * baseAddr)
	//primMoveImage = .text:10017A50
	//tprimMoveImage primMoveImage = (tprimMoveImage)GPUPlugin::Get().GetPluginMem(0x00017A50);
	//CreateHook(primMoveImage, GPUPatches::primMoveImage, &oprimMoveImage);
	//EnableHook(primMoveImage);
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

void (APIENTRY* GPUPatches::oglTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
void APIENTRY GPUPatches::Hook_glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels)
{
	//PLUGINLOG("glTexSubImage2D 0x%08X %d %d %d %d %d 0x%08X 0x%08X %p", target, level, xoffset, yoffset, width, height, format, type, pixels);
	if (target == GL_TEXTURE_2D && format == GL_RGBA && width <= MAX_TEXTURE_WIDTH && height <= MAX_TEXTURE_HEIGHT)
	{
		const auto& buffer = s_pGPUPatches->ScaleTexture((u32*)pixels, width, height);
		return oglTexSubImage2D(target, level, xoffset * s_pGPUPatches->m_scale, yoffset * s_pGPUPatches->m_scale, width * s_pGPUPatches->m_scale, height * s_pGPUPatches->m_scale, format, type, buffer.data());
	}
	return oglTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
}

void (APIENTRY* GPUPatches::oglTexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
void APIENTRY GPUPatches::Hook_glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels)
{
	if (target == GL_TEXTURE_2D && format == GL_RGBA && width >= 1024 && height >= 512)
		PLUGINLOG("Detected render surface, size: %d x %d x 32bpp, %d MiB", width, height, (width * height * 4) / (1024 * 1024));

	//PLUGINLOG("glTexImage2D 0x%08X %d %d %d 0x%08X 0x%08X %p", target, level, width, height, format, type, pixels);
	if (target == GL_TEXTURE_2D && format == GL_RGBA && width <= MAX_TEXTURE_WIDTH && height <= MAX_TEXTURE_HEIGHT)
	{
		const auto& buffer = s_pGPUPatches->ScaleTexture((u32*)pixels, width, height);
		return oglTexImage2D(target, level, internalformat, width * s_pGPUPatches->m_scale, height * s_pGPUPatches->m_scale, border, format, type, buffer.data());
	}
	return oglTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
}

std::vector<u32> GPUPatches::DePosterize(const u32* source, int width, int height)
{
	std::vector<u32> buf(width*height);
	std::vector<u32> out(width*height);

	deposterizeH(source, buf.data(), width, 0, height);
	deposterizeV(buf.data(), out.data(), width, height, 0, height);

	deposterizeH(out.data(), buf.data(), width, 0, height);
	deposterizeV(buf.data(), out.data(), width, height, 0, height);

	return std::move(out);
}

void GPUPatches::ResetGTECache()
{
	static bool once = true;
	if (*context.GetPatches().locFBE)
	{
		if (once)
		{
			PLUGINLOG("ResetGTECache");
			resetGteVertices();
			once = false;
		}
	}
	else
	{
		once = true;
	}
}

std::vector<u32> GPUPatches::ScaleTexture(const u32* source, u32 srcWidth, u32 srcHeight)
{
	static u32 seed = (u32)time(0);
	u32 texture_hash = XXH32(source, srcWidth * srcHeight * sizeof(u32), seed);

	if (m_texture_cache_size > 0 && texture_hash > 0 && !m_TextureCache[texture_hash].empty())
	{
		//PLUGINLOG("Cache HIT: %lu", texture_hash);
		return m_TextureCache[texture_hash];
	}

	//PLUGINLOG("Cache MISS: %lu", texture_hash);

	std::vector<u32> textureBuffer(srcWidth * m_scale * srcHeight * m_scale);

	//calculate slice
	int slice = (srcWidth * srcHeight) / m_batch_size;
	if (slice > (int)(srcHeight / 2)) slice = srcHeight / 2;
	if (slice <= 0) slice = 1;
	slice = std::min<u32>(slice, m_max_slices);

	//PLUGINLOG("slice: %lu, srcWidth: %lu, srcHeight: %lu, pixels: %lu", slice, srcWidth, srcHeight, srcWidth * srcHeight);
	if ((m_fast_fbe && *locFBE && ((srcWidth * srcHeight) < 16 * 16 || (srcWidth * srcHeight) > 128 * 128)) || m_force_nearest)
	{
		concurrency::parallel_for(0, (int)srcHeight, slice, [&](const int& i)
		{
			xbrz::nearestNeighborScale(source, srcWidth, srcHeight, srcWidth * sizeof(u32), textureBuffer.data(),
				srcWidth * m_scale, srcHeight * m_scale, srcWidth * m_scale * sizeof(u32), xbrz::SliceType::NN_SCALE_SLICE_SOURCE, i, i + slice);
		}
		);
		return std::move(textureBuffer);
	}

	if (context.GetConfig().GetDeposterize())
	{
		const auto& buffer = DePosterize(source, srcWidth, srcHeight);
		concurrency::parallel_for(0, (int)srcHeight, slice, [&](const int& i)
		{
			xbrz::scale(m_scale, buffer.data(), textureBuffer.data(), srcWidth, srcHeight, xbrz::ColorFormat::ARGB, m_ScalerCfg, i, i + slice);
		}
		);
	}
	else
	{
		concurrency::parallel_for(0, (int)srcHeight, slice, [&](const int& i)
		{
			xbrz::scale(m_scale, source, textureBuffer.data(), srcWidth, srcHeight, xbrz::ColorFormat::ARGB, m_ScalerCfg, i, i + slice);
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
			//PLUGINLOG("removing hash %lu", m_TextureCacheTimestamp.front());
			m_TextureCache.erase(m_TextureCacheTimestamp.front());
			m_TextureCacheTimestamp.erase(m_TextureCacheTimestamp.begin());
			//PLUGINLOG("cache size: %lu", m_TextureCache.size());
		}
	}
	return std::move(textureBuffer);
}

void GPUPatches::EnableTextureScaler(u32 scale, u32 batch_size, bool force_nearest, bool fast_fbe, u32 texture_cache_size)
{
	if (scale <= 1)
		return;

	static std::once_flag flag;
	std::call_once(flag, [&]()
	{
		m_scale = scale;
		m_batch_size = batch_size;
		m_force_nearest = force_nearest;
		m_fast_fbe = fast_fbe;
		m_texture_cache_size = texture_cache_size;

		//FIXME
		m_max_slices = context.GetConfig().GetMaxSlicesCount();

		CreateHook(glTexImage2D, Hook_glTexImage2D, reinterpret_cast<void**>(&oglTexImage2D));
		EnableHook(glTexImage2D);

		CreateHook(glTexSubImage2D, Hook_glTexSubImage2D, reinterpret_cast<void**>(&oglTexSubImage2D));
		EnableHook(glTexSubImage2D);

		PLUGINLOG("%ux%s Texture Filter, BatchSize: %lu", m_scale, m_force_nearest ? " Nearest Neighbour" : "BRZ", batch_size);
	});
}
