#pragma once

class TextureScaler
{
private:
	static const u32 MAX_TEXTURE_WIDTH = 256;
	static const u32 MAX_TEXTURE_HEIGHT = 256;

	u32 scale;
	u32 batch_size;
	bool force_nearest;
	bool fast_fbe;
	u32 texture_cache_size;
	u32 max_slices;

	u32* iFrameBufferEffect;

	static void(APIENTRY* oglTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
	static void APIENTRY Hook_glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);

	static void(APIENTRY* oglTexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
	static void APIENTRY Hook_glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);

	xbrz::ScalerCfg m_ScalerCfg;
	std::vector<u32> DePosterize(const u32* source, int width, int height);
	std::vector<u32> ScaleTexture(const u32* source, u32 srcWidth, u32 srcHeight);

	std::list<u32> m_TextureCacheTimestamp;
	std::unordered_map<u32, std::vector<u32>> m_TextureCache;

public:
	TextureScaler();
	~TextureScaler();
};

