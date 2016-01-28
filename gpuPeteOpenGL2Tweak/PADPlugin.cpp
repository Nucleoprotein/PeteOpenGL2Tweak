#include "stdafx.h"

#include "PADPlugin.h"
#include "gpuPeteOpenGL2Tweak.h"
#include "GPUPlugin.h"
#include "SafeWrite.h"

std::array<PADPlugin::PADPluginData, 2> PADPlugin::pads;

PADPlugin::tStartPollSub PADPlugin::oStartPollSub = nullptr;
PADPlugin::tPadPoll PADPlugin::oPadPoll = nullptr;

void* PADPlugin::PADOutAddr = nullptr;
void* PADPlugin::PAD1Addr = nullptr;
void* PADPlugin::PAD2Addr = nullptr;

const std::vector<PADPlugin::ePSXeHackData> PADPlugin::epsxedata =
{
	//TimeSteamp, StartPollAddr, PADPollAddr, PADOutAddr, PAD1Addr, PAD2Addr
    { 0x54C777F2, 0x0002A070, 0x00028BD0, 0x000B4E34, 0x00549042, 0x00549083 }, //1925
    { 0x51F2F1B3, 0x00023AE0, 0x00022640, 0x00083DC0, 0x00515F82, 0x00515FC3 }, //190
	{ 0x509D0CDF, 0x000230D0, 0x00021C30, 0x0007F7B0, 0x00511482, 0x005114C3 }, //180
    { 0x483816FA, 0x0003B430, 0x0003A2C0, 0x000BE2EC, 0x00107BE2, 0x00107C23 }, //170
};

IMAGE_NT_HEADERS* PADPlugin::SimpeGetImageNtHeader(void* _pMem)
{
    if (_pMem)
    {
        const u16 pe = 0x4550; //"PE"
        u8* pMem = (u8*)_pMem;

        // search first megabyte for "PE"
        for (int i = 0; i < 1024; ++i)
        {
            if (*(u16*)pMem == pe)
                return (IMAGE_NT_HEADERS*)pMem;
            pMem++;
        }
    }
    return nullptr;
}

const PADPlugin::ePSXeHackData* PADPlugin::DetectEmuVersion()
{
    IMAGE_NT_HEADERS *pNtHdr = SimpeGetImageNtHeader(context.GetEmulatorMemory());
    if (pNtHdr)
    {
        for (auto& data : epsxedata)
        {
            // Using TimeDateStamp works for UPXed and unpacked executables !
            if (data.check == pNtHdr->FileHeader.TimeDateStamp)
            {
                PLUGINLOG("ePSXePadSupport: Detected supported ePSXe");
                return &(data);
            }
        }
    }
    return nullptr;
}

bool PADPlugin::Hook()
{
    static const ePSXeHackData* data = DetectEmuVersion();
    if (data)
    {
		tStartPollSub _StartPollSub = (tStartPollSub)context.GetEmulatorMemory(data->PADStartPollAddr);
		tPadPoll _PadPoll = (tPadPoll)context.GetEmulatorMemory(data->PADPollAddr);

		PADOutAddr = context.GetEmulatorMemory(data->PADOutAddr);
		PAD1Addr = context.GetEmulatorMemory(data->PAD1Addr);
		PAD2Addr = context.GetEmulatorMemory(data->PAD2Addr);

		//PLUGINLOG("%p %p %p", PADOutAddr, PAD1Addr, PAD2Addr);

		if (_StartPollSub && _PadPoll && PADOutAddr && PAD1Addr && PAD2Addr && !oStartPollSub && !oPadPoll)
		{
			MH_CreateHook(_StartPollSub, HookStartPollSub, reinterpret_cast<void**>(&oStartPollSub));
			MH_EnableHook(_StartPollSub);

			MH_CreateHook(_PadPoll, HookPadPoll, reinterpret_cast<void**>(&oPadPoll));
			MH_EnableHook(_PadPoll);
			return true;
		}
		else if (oStartPollSub && oPadPoll)
			return true;
    }
    return false;
}

void PADPlugin::Load(const std::string& port1, const std::string& port2)
{
	std::string padpluginanames[2] = {port1, port2};

	for (int i = 0; i < 2; ++i)
	{
		if (pads[i].dll != 0)
			continue;

		if (padpluginanames[i].empty())
			continue;

		HMODULE hModule = LoadLibrary(padpluginanames[i].c_str());
		if(!hModule)
			hModule = LoadLibrary(("plugins\\" + padpluginanames[i]).c_str());
		if (hModule)
		{
			PLUGINLOG("ePSXePadSupport: Initalizing Port%u %s", i + 1, padpluginanames[i].c_str());
			GetProcedure(hModule, "PADinit", &pads[i].PADinit);
			GetProcedure(hModule, "PADshutdown", &pads[i].PADshutdown);

			GetProcedure(hModule, "PADopen", &pads[i].PADopen);
			GetProcedure(hModule, "PADclose", &pads[i].PADclose);

			GetProcedure(hModule, "PADconfigure", &pads[i].PADconfigure);
			GetProcedure(hModule, "PADabout", &pads[i].PADabout);
			GetProcedure(hModule, "PADtest", &pads[i].PADtest);

			GetProcedure(hModule, "PADquery", &pads[i].PADquery);
			GetProcedure(hModule, "PADstartPoll", &pads[i].PADstartPoll);
			GetProcedure(hModule, "PADpoll", &pads[i].PADpoll);

			GetProcedure(hModule, "PADfreeze", &pads[i].PADfreeze);
			GetProcedure(hModule, "PADkeypressed", &pads[i].PADkeypressed);

			pads[i].dll = hModule;
		}
	}
}

void PADPlugin::OnPADinit()
{
    for (auto& pad : pads)
    {
        s32 i = 1;
        if (pad.PADinit)
            pad.PADinit(i);
        ++i;
    }
}

void PADPlugin::OnPADopen(HWND hWnd)
{
    OnPADclose();

    for (auto& pad : pads)
    {
        if (pad.PADopen)
            pad.PADopen(hWnd);
    }
}

void PADPlugin::OnPADclose()
{
    for (auto& pad : pads)
    {
        if (pad.PADclose)
            pad.PADclose();
    }
}

void PADPlugin::OnPADshutdown()
{
    for (auto& pad : pads)
    {
        if (pad.PADshutdown)
            pad.PADshutdown();
    }
}

void PADPlugin::OnPADconfigure()
{
    for (auto& pad : pads)
    {
        if (pad.PADconfigure)
            pad.PADconfigure();
    }
}

char __cdecl PADPlugin::HookStartPollSub(u8 port)
{
	u8 padno = port - 1;
    if (pads[padno].PADstartPoll)
		pads[padno].PADstartPoll(port);

	return PADPlugin::oStartPollSub(port);
}

u8 __cdecl PADPlugin::HookPadPoll(u8 cmd, u8 padno, void* unknown)
{
    if (pads[padno].PADpoll)
    {
		u8 data = pads[padno].PADpoll(cmd);

        //PLUGINLOG("PADpoll %u %u %u", a1, padno, data);

		if (padno == 0 && *((u8*)PAD1Addr) == 1) {
            *((u8*)PADOutAddr) = 1 + 2 + (data & 0x0f) * 2;

        }
		if (padno == 1 && *((u8*)PAD2Addr) == 1) {
			*((u8*)PADOutAddr) = 1 + 2 + (data & 0x0f) * 2;
        }
		// Wipeout 3 fix
		if (cmd == 0x43 && data == 0x23) {
			data = 0x79;
		}
        return data;
    }
    else
		return oPadPoll(cmd, padno, unknown);
}
