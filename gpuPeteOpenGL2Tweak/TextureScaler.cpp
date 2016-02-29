#include "stdafx.h"
#include "gpuPeteOpenGL2Tweak.h"
#include "GPUPlugin.h"
#include "xBRZ\xbrz.h"
#include "TextureScaler.h"
#include "xxhash.h"
#include "deposterize.h"
#include <ppl.h>

static TextureScaler* s_TextureScaler;

TextureScaler::TextureScaler()
{
	scale = context.GetConfig()->GetxBRZScale();
	batch_size = context.GetConfig()->GetBatchSize();
	force_nearest = context.GetConfig()->GetForceNearest();
	fast_fbe = context.GetConfig()->GetFastFBE();
	texture_cache_size = context.GetConfig()->GetTextureCacheSize();
	max_slices = context.GetConfig()->GetMaxSlicesCount();

	if (scale <= 1)
		return;

	s_TextureScaler = this;

	iFrameBufferEffect = (u32*)GPUPlugin::Get().GetPluginMem(0x000500E0);

	CreateHook(glTexImage2D, Hook_glTexImage2D, reinterpret_cast<void**>(&oglTexImage2D));
	EnableHook(glTexImage2D);

	CreateHook(glTexSubImage2D, Hook_glTexSubImage2D, reinterpret_cast<void**>(&oglTexSubImage2D));
	EnableHook(glTexSubImage2D);

	PLUGINLOG("%ux%s Texture Filter, BatchSize: %lu", scale, force_nearest ? " Nearest Neighbour" : "BRZ", batch_size);
}

TextureScaler::~TextureScaler()
{
}

void (APIENTRY* TextureScaler::oglTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
void APIENTRY TextureScaler::Hook_glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels)
{
	//PLUGINLOG("glTexSubImage2D 0x%08X %d %d %d %d %d 0x%08X 0x%08X %p", target, level, xoffset, yoffset, width, height, format, type, pixels);
	if (target == GL_TEXTURE_2D && format == GL_RGBA && width <= MAX_TEXTURE_WIDTH && height <= MAX_TEXTURE_HEIGHT)
	{
		const auto& buffer = s_TextureScaler->ScaleTexture((u32*)pixels, width, height);
		return oglTexSubImage2D(target, level, xoffset * s_TextureScaler->scale, yoffset * s_TextureScaler->scale, width * s_TextureScaler->scale, height * s_TextureScaler->scale, format, type, buffer.data());
	}
	return oglTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
}

void (APIENTRY* TextureScaler::oglTexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
void APIENTRY TextureScaler::Hook_glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels)
{
	if (target == GL_TEXTURE_2D && format == GL_RGBA && width >= 1024 && height >= 512)
		PLUGINLOG("Detected render surface, size: %d x %d x 32bpp, %d MiB", width, height, (width * height * 4) / (1024 * 1024));

	//PLUGINLOG("glTexImage2D 0x%08X %d %d %d 0x%08X 0x%08X %p", target, level, width, height, format, type, pixels);
	if (target == GL_TEXTURE_2D && format == GL_RGBA && width <= MAX_TEXTURE_WIDTH && height <= MAX_TEXTURE_HEIGHT)
	{
		const auto& buffer = s_TextureScaler->ScaleTexture((u32*)pixels, width, height);
		return oglTexImage2D(target, level, internalformat, width * s_TextureScaler->scale, height * s_TextureScaler->scale, border, format, type, buffer.data());
	}
	return oglTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
}

std::vector<u32> TextureScaler::DePosterize(const u32* source, int width, int height)
{
	std::vector<u32> buf(width*height);
	std::vector<u32> out(width*height);

	deposterizeH(source, buf.data(), width, 0, height);
	deposterizeV(buf.data(), out.data(), width, height, 0, height);

	deposterizeH(out.data(), buf.data(), width, 0, height);
	deposterizeV(buf.data(), out.data(), width, height, 0, height);

	return std::move(out);
}

std::vector<u32> TextureScaler::ScaleTexture(const u32* source, u32 srcWidth, u32 srcHeight)
{
	static u32 seed = (u32)time(0);
	u32 texture_hash = XXH32(source, srcWidth * srcHeight * sizeof(u32), seed);

	if (texture_cache_size > 0 && texture_hash > 0 && !m_TextureCache[texture_hash].empty())
	{
		//PLUGINLOG("Cache HIT: %lu", texture_hash);
		return m_TextureCache[texture_hash];
	}

	//PLUGINLOG("Cache MISS: %lu", texture_hash);

	std::vector<u32> textureBuffer(srcWidth * scale * srcHeight * scale);

	//calculate slice
	int slice = (srcWidth * srcHeight) / batch_size;
	if (slice > (int)(srcHeight / 2)) slice = srcHeight / 2;
	slice = std::min<u32>(slice, max_slices);
	if (slice <= 0) slice = 1;

	//PLUGINLOG("slice: %lu, srcWidth: %lu, srcHeight: %lu, pixels: %lu", slice, srcWidth, srcHeight, srcWidth * srcHeight);
	if ((fast_fbe && *iFrameBufferEffect && ((srcWidth * srcHeight) < 32 * 32 || (srcWidth * srcHeight) > 128 * 128)) || force_nearest)
	{
		concurrency::parallel_for(0, (int)srcHeight, slice, [&](const int& i)
		{
			xbrz::nearestNeighborScale(source, srcWidth, srcHeight, srcWidth * sizeof(u32), textureBuffer.data(),
				srcWidth * scale, srcHeight * scale, srcWidth * scale * sizeof(u32), xbrz::SliceType::NN_SCALE_SLICE_SOURCE, i, i + slice);
		}
		);
		return std::move(textureBuffer);
	}

	if (context.GetConfig()->GetDeposterize())
	{
		const auto& buffer = DePosterize(source, srcWidth, srcHeight);
		concurrency::parallel_for(0, (int)srcHeight, slice, [&](const int& i)
		{
			xbrz::scale(scale, buffer.data(), textureBuffer.data(), srcWidth, srcHeight, xbrz::ColorFormat::ARGB, m_ScalerCfg, i, i + slice);
		}
		);
	}
	else
	{
		concurrency::parallel_for(0, (int)srcHeight, slice, [&](const int& i)
		{
			xbrz::scale(scale, source, textureBuffer.data(), srcWidth, srcHeight, xbrz::ColorFormat::ARGB, m_ScalerCfg, i, i + slice);
		}
		);
	}

	if (texture_cache_size > 0 && texture_hash > 0)
	{
		m_TextureCacheTimestamp.push_back(texture_hash);
		m_TextureCache[texture_hash] = textureBuffer;

		//PLUGINLOG("cache size: %lu", m_TextureCache.size());
		//PLUGINLOG("cache time size: %lu", m_TextureCacheTimestamp.size());
		if (m_TextureCache.size() >= texture_cache_size)
		{
			//PLUGINLOG("removing hash %lu", m_TextureCacheTimestamp.front());
			m_TextureCache.erase(m_TextureCacheTimestamp.front());
			m_TextureCacheTimestamp.erase(m_TextureCacheTimestamp.begin());
			//PLUGINLOG("cache size: %lu", m_TextureCache.size());
		}
	}
	return std::move(textureBuffer);
}
