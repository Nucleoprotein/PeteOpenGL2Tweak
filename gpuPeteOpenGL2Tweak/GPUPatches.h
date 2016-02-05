#pragma once

#define MAX_TEXTURE_X 256
#define MAX_TEXTURE_Y 256

class GPUPatches
{
public:
    typedef BOOL(__cdecl* tOffset)(void);

    GPUPatches();
    ~GPUPatches();

    struct GTEData
    {
		std::array<s16*, 4> lx;
		std::array<s16*, 4> ly;
		std::array<OGLVertex*, 4> vertex;
        s16* PSXDisplay_CumulOffset_x;
        s16* PSXDisplay_CumulOffset_y;
    };

    void GTEAccuracy();
    void FixMemoryDetection();
	void EnableVsync(s32 interval);
    void ResHack(u32 _x, u32 _y);
	void FixFullscreenAspect();
	void ApplyWindowProc(HWND hWnd);
	void EnableTextureScaler(u32 scale, u32 m_batch_size, bool force_nearest, bool fast_fbe, u32 texture_cache_size);

private:
    GTEData m_gtedata;
	u32 m_scale;
	u32 m_batch_size;

	bool m_force_nearest;
	bool m_fast_fbe;
	u32 m_texture_cache_size;

	void fix_offsets(s32 count);

	static BOOL __cdecl offset3(void);
	static tOffset ooffset3;

	static BOOL __cdecl offset4(void);
    static tOffset ooffset4;

	static BOOL __cdecl offsetST(void);
	static tOffset ooffsetST;

	static BOOL __cdecl offsetline(void);
	static tOffset ooffsetline;

	u32* locVRamSize;
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

	float* locFPS;
	u32* locFBE;

	WNDPROC oldWndProc;
	static LRESULT CALLBACK TweakWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	static void(APIENTRY* oglTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
	static void APIENTRY Hook_glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);

	static void(APIENTRY* oglTexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
	static void APIENTRY Hook_glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);

	static void(APIENTRY* oglCopyTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
	static void APIENTRY Hook_glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);

	std::vector<u32> GPUPatches::ScaleTexture(const u32* source, u32 srcWidth, u32 srcHeight);

	std::list<u32> m_TextureCacheTimestamp;
	std::unordered_map<u32, std::vector<u32>> m_TextureCache;
};