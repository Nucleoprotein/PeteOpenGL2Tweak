#pragma once

class PADPlugin : NonCopyable
{
public:

    struct PADPluginData
    {
        HMODULE dll;
        s32(CALLBACK* PADinit)(s32 flags);
        s32(CALLBACK* PADshutdown)();
        s32(CALLBACK* PADopen)(HWND hWnd);
        s32(CALLBACK* PADclose)();
        s32(CALLBACK* PADconfigure)();
        void(CALLBACK* PADabout)();
        s32(CALLBACK* PADtest)();
        s32(CALLBACK* PADquery)();
        u8(CALLBACK* PADstartPoll)(s32 port);
        u8(CALLBACK* PADpoll)(u8 data);
        u32(CALLBACK* PADfreeze)(s32 mode, void *data);
        s32(CALLBACK* PADkeypressed)();
    };

    static std::array<PADPluginData, 2> pads;

    struct ePSXeHackData
    {
        u32 check;
		uintptr_t PADStartPollAddr;
		uintptr_t PADPollAddr;
		uintptr_t PADOutAddr;
		uintptr_t PAD1Addr;
		uintptr_t PAD2Addr;
    };

    bool Hook();
	void PADPlugin::Load(const std::string& port1, const std::string& port2);

    void OnPADinit();
    void OnPADopen(HWND hWnd);
    void OnPADclose();
    void OnPADshutdown();
    void OnPADconfigure();

    ~PADPlugin()
    {
        for (auto& pad : pads)
            FreeLibrary(pad.dll);
    }

private:

    template<typename T>
    void GetProcedure(HMODULE hModule, const char* funcname, T* ppfunc)
    {
        *ppfunc = reinterpret_cast<T>(::GetProcAddress(hModule, funcname));
    }

    IMAGE_NT_HEADERS* SimpeGetImageNtHeader(void* _pMem);

	typedef char(__cdecl* tStartPollSub)(u8 port);
    static tStartPollSub oStartPollSub;
	static char __cdecl HookStartPollSub(u8 port);

	typedef u8(__cdecl* tPadPoll)(u8 cmd, u8 padno, void* unknown);
    static tPadPoll oPadPoll;
	static u8 __cdecl HookPadPoll(u8 cmd, u8 padno, void* unknown);

    const ePSXeHackData* DetectEmuVersion();
	static void* PADOutAddr;
    static void* PAD1Addr;
	static void* PAD2Addr;

    static const std::vector<ePSXeHackData> epsxedata;

};
