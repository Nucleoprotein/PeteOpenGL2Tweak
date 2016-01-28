#pragma once

#define MAX_TEXTURE_X 512
#define MAX_TEXTURE_Y 512

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
	void TextureScale(u32 scale, u32 slice, bool force_nearest, bool fast_fbe, u32 texture_cache_size);

private:
    static GTEData m_gtedata;
	static u32 m_scale;
	static u32 m_slice;

	static bool m_force_nearest;
	static bool m_fast_fbe;
	static u32 m_texture_cache_size;

	static void fix_offsets(s32 count);

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

	static float* locFPS;
	static u32* locFBE;

	static WNDPROC oldWndProc;
	static LRESULT CALLBACK TweakWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	static void(APIENTRY* oglTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
	static void APIENTRY Hook_glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);

	static void(APIENTRY* oglTexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
	static void APIENTRY Hook_glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);

	static void(APIENTRY* oglCopyTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
	static void APIENTRY Hook_glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);

	static void ScaleXBRZ(size_t factor, const void* src, void* trg, int srcWidth, int srcHeight, xbrz::ColorFormat format);

	//static u8* m_TexBuf;
	static std::vector<u32> m_TexBuf;

	typedef std::list<u32> timestamp_to_key_type;
	typedef std::unordered_map<u32, std::vector<u32>> key_to_value_type;

	static timestamp_to_key_type m_TextureCacheTimestamp;
	static key_to_value_type m_TextureCache;
};