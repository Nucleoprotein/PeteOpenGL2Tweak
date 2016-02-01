
#include "gte_accuracy.h"
#include "GPUPatches.h"
#include "PADPlugin.h"

#define PLUGINLOG(format, ...) printf("TWEAK: " format "\n", __VA_ARGS__)

static const char *ini_filename = ".\\inis\\gpuPeteOpenGL2Tweak.ini";
static const char *libraryName = "PeteOpenGL2 Tweaks";

static const char *dllfilename = ".\\plugins\\gpuPeteOpenGL2.dll";
static const char *dllfilename2 = ".\\plugins\\gpuPeopsOpenGL2.dll";
static const char *pecfilename = ".\\plugins\\gpupec.dll";

#define VERSION_MAJOR 2
#define VERSION_MINOR 3

#define CONFIG_VERSION 230

class Config : NonCopyable
{
public:
    Config()
    {
		CreateDirectoryA(".\\inis", NULL);

        CSimpleIniA ini;
		ini.LoadFile(ini_filename);

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
            ini.SaveFile(ini_filename);
            ini.LoadFile(ini_filename);
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

class Context : NonCopyable
{
public:
    static Context instance;

    Context();
    ~Context();

	Config& GetConfig(){ return m_config; };
	GPUPatches& GetPatches(){ return m_gpupatches; };
	PADPlugin& GetPadPlugin(){ return m_padplugin; };

    s32 OnGPUinit();
    s32 OnGPUshutdown();
    s32 OnGPUclose();
    void OnGPUaddVertex(s16 sx, s16 sy, s64 fx, s64 fy, s64 fz);

    s32 OnGPUtest();

    void LoadPad();

    void* GetEmulatorMemory(u32 offset = 0)
    {
		std::string exename = ModuleNameA(NULL);

		if (StringSearchIgnoreCase(exename, "ePSXeCutor"))
            return nullptr;
		if (!StringSearchIgnoreCase(exename, "ePSXe"))
            return nullptr;

        return (u8*)GetModuleHandle(NULL) + offset;
    }

private:
    Config m_config;
    GPUPatches m_gpupatches;
    PADPlugin m_padplugin;

	static s32(CALLBACK* oGPUopen)(HWND hwndGPU);
	static s32 CALLBACK Hook_GPUopen(HWND hwndGPU);
};

static Context& context = Context::instance;