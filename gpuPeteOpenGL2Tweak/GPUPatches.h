#pragma once

#define MAX_TEXTURE_WIDTH 256
#define MAX_TEXTURE_HEIGHT 256

class GPUPatches
{
public:
	typedef BOOL(__cdecl* tOffset)(void);
	typedef void(__cdecl* tprimMoveImage)(unsigned char * baseAddr);

	GPUPatches();
	~GPUPatches();

	struct GTEData
	{
		std::array<s16*, 4> lx;
		std::array<s16*, 4> ly;
		std::array<GTEVertex*, 4> vertex;
		s16* PSXDisplay_CumulOffset_x;
		s16* PSXDisplay_CumulOffset_y;
	};

	void EnableGTEAccuracy();
	GTEAccuracy& GetGTEAccuracy() { return m_gteacc; };
	void FixMemoryDetection();
	void EnableVsync(s32 interval);
	void ResHack(u32 _x, u32 _y);
	void FixFullscreenAspect();
	void ApplyWindowProc(HWND hWnd);
	void EnableTextureScaler(u32 scale, u32 m_batch_size, bool force_nearest, bool fast_fbe, u32 texture_cache_size);

	void ResetGTECache();

private:
	GTEData m_gtedata;
	GTEAccuracy m_gteacc;
	u32 m_scale;
	u32 m_batch_size;

	bool m_force_nearest;
	bool m_fast_fbe;
	u32 m_texture_cache_size;
	u32 m_max_slices;

	void fix_offsets(s32 count);

	static BOOL __cdecl offset3(void);
	static tOffset ooffset3;

	static BOOL __cdecl offset4(void);
	static tOffset ooffset4;

	int* locVRamSize;
	typedef u32(__cdecl* fCheckTextureMemory)();
	fCheckTextureMemory CheckTextureMemory;
	int GetVideoMemoryAMD();
	int GetVideoMemoryNV();

	// ResHack
	u16* nopX;
	u16* nopY;
	u8* mulX;
	u8* mulY;

	// FixAspect
	u32* locW;
	u32* locH;
	u32* locX;
	u32* locY;
	u32* locWinSize;
	BOOL* locWindowned;

	//float* locFPS;
	u32* locFBE;

	WNDPROC oldWndProc;
	static LRESULT CALLBACK TweakWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	static void(APIENTRY* oglTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
	static void APIENTRY Hook_glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);

	static void(APIENTRY* oglTexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
	static void APIENTRY Hook_glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);

	xbrz::ScalerCfg m_ScalerCfg;
	std::vector<u32> DePosterize(const u32* source, int width, int height);
	std::vector<u32> ScaleTexture(const u32* source, u32 srcWidth, u32 srcHeight);

	std::list<u32> m_TextureCacheTimestamp;
	std::unordered_map<u32, std::vector<u32>> m_TextureCache;
};