#pragma once

class GPUPatches
{
private:
	int* locVRamSize;
	typedef u32(__cdecl* CheckTextureMemory_fn)();
	CheckTextureMemory_fn CheckTextureMemory;
	int GetVideoMemoryAMD();
	int GetVideoMemoryNV();

	// ResHack
	u16* nopX;
	u16* nopY;
	u8* mulX;
	u8* mulY;

	// FixAspect
	u32* locX;
	u32* locY;
	s32* iWinSize;
	s32* iWindowned;
	s32* iResX;
	s32* iResY;

	WNDPROC oldWndProc;
	static LRESULT CALLBACK TweakWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


public:
	GPUPatches();
	~GPUPatches();

	void FixMemoryDetection();
	void EnableVsync(s32 interval);
	void ResHack(u32 _x, u32 _y);
	void FixFullscreenAspect();
	void ApplyWindowProc(HWND hWnd);
};