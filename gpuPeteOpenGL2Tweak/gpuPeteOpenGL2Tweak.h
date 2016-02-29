#define PLUGINLOG(format, ...) printf("TWEAK: " format "\n", __VA_ARGS__)
#define PLUGINLOGF(format, ...) printf("TWEAK: " __FUNCTION__ " " format "\n", __VA_ARGS__)

static const char *inifilename = "\\gpuPeteOpenGL2Tweak.ini";

#ifndef _DEBUG
static const char *libraryName = "PeteOpenGL2 Tweaks";
#else
static const char *libraryName = "PeteOpenGL2 Tweaks DEBUG";
#endif

static const char *dllfilename = "\\gpuPeteOpenGL2.dll";
static const char *pecfilename = "\\gpupec.dll";

#define VERSION_MAJOR 2
#define VERSION_MINOR 4

#define CONFIG_VERSION 240

class Config : NonCopyable
{
public:
    Config()
    {
		std::string inisdir = StringFromFormat("%s%s", ModuleDirectoryA(0).c_str(), "\\inis");
		CreateDirectoryA(inisdir.c_str(), NULL);

        CSimpleIniA ini;
		std::string inipath = StringFromFormat("%s%s", inisdir.c_str(), inifilename);
		ini.LoadFile(inipath.c_str());

		u32 config_version = ini.GetLongValue("Version", "Config");
		if (config_version != CONFIG_VERSION)
        {
            // save file and reload
			ini.Reset();

#define SETTING(_func, _type, _var, _section, _defaultval, _comment) \
    ini.Set##_func(_section, #_var, _defaultval, _comment)
#include "Settings.def"
#undef SETTING

			ini.SetLongValue("Version", "Config", CONFIG_VERSION);
            ini.SaveFile(inipath.c_str());
            ini.LoadFile(inipath.c_str());
        }

#define SETTING(_func, _type, _var, _section, _defaultval, _comment) \
    _var = ini.Get##_func(_section, #_var)
#include "Settings.def"
#undef SETTING
    }

#define SETTING(_func, _type, _var, _section, _defaultval, _comment) \
	private: _type _var; \
	public: const _type& Get##_var() const { return _var; };
#include "Settings.def"
#undef SETTING
};

class GPUPatches;
class GTEAccHack;
class TextureScaler;

class Context : NonCopyable
{
public:
    static Context instance;

    Context();
    ~Context();

	std::shared_ptr<Config> GetConfig() const
	{ 
		return config; 
	}; 

    s32 OnGPUinit();
    s32 OnGPUclose();
    void OnGPUaddVertex(s16 sx, s16 sy, s64 fx, s64 fy, s64 fz);
	void OnGPUclearVertex(s16 sx, s16 sy, u16 z);
	u32 OnGPUreadData();
	void OnGPUreadDataMem(u32* pMem, s32 iSize);
	void OnGPUsetframelimit(u32 option);
	s32 OnGPUgetVertex(s16 sx, s16 sy, u16 z, float* fx, float* fy);

private:
	std::shared_ptr<Config> config;
    std::unique_ptr<GPUPatches> gpuPatches;
	std::unique_ptr<GTEAccHack> gteAccHack;
	std::unique_ptr<TextureScaler> textureScaler;

	static s32(CALLBACK* oGPUopen)(HWND hwndGPU);
	static s32 CALLBACK Hook_GPUopen(HWND hwndGPU);
};

template<typename N>
void CreateHookF(LPVOID pTarget, LPVOID pDetour, N* ppOriginal, const char* pTargetName)
{
	if (*ppOriginal) return;
	MH_STATUS status = MH_CreateHook(pTarget, pDetour, reinterpret_cast<void**>(ppOriginal));
	PLUGINLOG("CreateHook %s status %s", pTargetName, MH_StatusToString(status));
}

inline void EnableHookF(LPVOID pTarget, const char* pTargetName)
{
	MH_STATUS status = MH_EnableHook(pTarget);
	PLUGINLOG("EnableHook %s status %s", pTargetName, MH_StatusToString(status));
}

#define CreateHook(pTarget, pDetour, ppOrgiginal) CreateHookF(pTarget, pDetour, ppOrgiginal, #pTarget)
#define EnableHook(pTarget) EnableHookF(pTarget, #pTarget)

static Context& context = Context::instance;
