#pragma once

class GPUPlugin : NonCopyable
{
public:
	s32(CALLBACK* GPUinit)();
	s32(CALLBACK* GPUshutdown)();

	s32(CALLBACK* GPUopen)(HWND hwndGPU);
	s32(CALLBACK* GPUclose)();

	s32(CALLBACK* GPUconfigure)();
	void(CALLBACK* GPUabout)();
	s32(CALLBACK* GPUtest)();

	u32(CALLBACK* GPUreadStatus)();
	u32(CALLBACK* GPUreadData)();
	void(CALLBACK* GPUreadDataMem)(u32* pMem, s32 iSize);

	void(CALLBACK* GPUwriteStatus)(u32 gdata);
	void(CALLBACK* GPUwriteDataMem)(u32* pMem, s32 iSize);
	void(CALLBACK* GPUwriteData)(u32 gdata);

	s32(CALLBACK* GPUgetMode)();
	void(CALLBACK* GPUsetMode)(u32 gdata);

	void(CALLBACK* GPUupdateLace)();
	s32(CALLBACK* GPUdmaChain)(u32 * baseAddrL, u32 addr);

	void(CALLBACK* GPUmakeSnapshot)();
	void(CALLBACK* GPUkeypressed)(u32);
	void(CALLBACK* GPUdisplayText)(s8 *);
	void(CALLBACK* GPUdisplayFlags)(u32 dwFlags);

	s32(CALLBACK* GPUfreeze)(u32, struct GPUFreeze_t *);
	s32(CALLBACK* GPUgetScreenPic)(u8 *);
	s32(CALLBACK* GPUshowScreenPic)(u8 *);
	void(CALLBACK* GPUclearDynarec)(void(CALLBACK *callback)(void));
	void(CALLBACK* GPUhSync)(s32);
	void(CALLBACK* GPUvBlank)(s32);
	void(CALLBACK* GPUvisualVibration)(u32, u32);
	void(CALLBACK* GPUcursor)(s32, s32, s32);
	void(CALLBACK* GPUaddVertex)(s16, s16, s64, s64, s64);

	void(CALLBACK* GPUsetfix)(u32 dwFixBits);
	void(CALLBACK* GPUsetframelimit)(u32 option);

	GPUPlugin()
	{
		if (context.GetConfig().GetUsePEC() && !context.GetConfig().GetHardcoreMode())
		{
			m_pec = LoadLibraryA(pecfilename);
			if (m_pec)
			{
				PLUGINLOG("PEC Loaded");
				GetPECProcedure("GPUinit", &GPUinit);
				GetPECProcedure("GPUshutdown", &GPUshutdown);

				GetPECProcedure("GPUopen", &GPUopen);
				GetPECProcedure("GPUclose", &GPUclose);

				GetPECProcedure("GPUconfigure", &GPUconfigure);
				GetPECProcedure("GPUabout", &GPUabout);
				GetPECProcedure("GPUtest", &GPUtest);

				GetPECProcedure("GPUreadStatus", &GPUreadStatus);
				GetPECProcedure("GPUreadData", &GPUreadData);
				GetPECProcedure("GPUreadDataMem", &GPUreadDataMem);

				GetPECProcedure("GPUwriteStatus", &GPUwriteStatus);
				GetPECProcedure("GPUwriteData", &GPUwriteData);
				GetPECProcedure("GPUwriteDataMem", &GPUwriteDataMem);

				GetPECProcedure("GPUgetMode", &GPUgetMode);
				GetPECProcedure("GPUsetMode", &GPUsetMode);

				GetPECProcedure("GPUupdateLace", &GPUupdateLace);
				GetPECProcedure("GPUdmaChain", &GPUdmaChain);

				GetPECProcedure("GPUmakeSnapshot", &GPUmakeSnapshot);
				GetPECProcedure("GPUkeypressed", &GPUkeypressed);
				GetPECProcedure("GPUdisplayText", &GPUdisplayText);
				GetPECProcedure("GPUdisplayFlags", &GPUdisplayFlags);

				GetPECProcedure("GPUfreeze", &GPUfreeze);
				GetPECProcedure("GPUgetScreenPic", &GPUgetScreenPic);
				GetPECProcedure("GPUshowScreenPic", &GPUshowScreenPic);
				GetPECProcedure("GPUclearDynarec", &GPUclearDynarec);
				GetPECProcedure("GPUhSync", &GPUhSync);
				GetPECProcedure("GPUvBlank", &GPUvBlank);
				GetPECProcedure("GPUvisualVibration", &GPUvisualVibration);
				GetPECProcedure("GPUcursor", &GPUcursor);
				GetPECProcedure("GPUaddVertex", &GPUaddVertex);

				GetPECProcedure("GPUsetfix", &GPUsetfix);
				GetPECProcedure("GPUsetframelimit", &GPUsetframelimit);
			}
		}

		m_petes = LoadLibraryA(dllfilename);
		if (!m_petes)
		{
			HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
			std::unique_ptr<char[]> error_msg(new char[MAX_PATH]);
			sprintf_s(error_msg.get(), MAX_PATH, "Cannot load \"%s\" error: 0x%x", dllfilename, hr);
			MessageBoxA(NULL, error_msg.get(), "Error", MB_ICONERROR);
			exit(hr);
		}
		else
		{
			PLUGINLOG("Loaded %s", dllfilename);
		}

		GetPetesProcedure("GPUinit", &GPUinit);
		GetPetesProcedure("GPUshutdown", &GPUshutdown);

		GetPetesProcedure("GPUopen", &GPUopen);
		GetPetesProcedure("GPUclose", &GPUclose);

		GetPetesProcedure("GPUconfigure", &GPUconfigure);
		GetPetesProcedure("GPUabout", &GPUabout);
		GetPetesProcedure("GPUtest", &GPUtest);

		GetPetesProcedure("GPUreadStatus", &GPUreadStatus);
		GetPetesProcedure("GPUreadData", &GPUreadData);
		GetPetesProcedure("GPUreadDataMem", &GPUreadDataMem);

		GetPetesProcedure("GPUwriteStatus", &GPUwriteStatus);
		GetPetesProcedure("GPUwriteData", &GPUwriteData);
		GetPetesProcedure("GPUwriteDataMem", &GPUwriteDataMem);

		GetPetesProcedure("GPUgetMode", &GPUgetMode);
		GetPetesProcedure("GPUsetMode", &GPUsetMode);

		GetPetesProcedure("GPUupdateLace", &GPUupdateLace);
		GetPetesProcedure("GPUdmaChain", &GPUdmaChain);

		GetPetesProcedure("GPUmakeSnapshot", &GPUmakeSnapshot);
		GetPetesProcedure("GPUkeypressed", &GPUkeypressed);
		GetPetesProcedure("GPUdisplayText", &GPUdisplayText);
		GetPetesProcedure("GPUdisplayFlags", &GPUdisplayFlags);

		GetPetesProcedure("GPUfreeze", &GPUfreeze);
		GetPetesProcedure("GPUgetScreenPic", &GPUgetScreenPic);
		GetPetesProcedure("GPUshowScreenPic", &GPUshowScreenPic);
		GetPetesProcedure("GPUclearDynarec", &GPUclearDynarec);
		GetPetesProcedure("GPUhSync", &GPUhSync);
		GetPetesProcedure("GPUvBlank", &GPUvBlank);
		GetPetesProcedure("GPUvisualVibration", &GPUvisualVibration);
		GetPetesProcedure("GPUcursor", &GPUcursor);
		GetPetesProcedure("GPUaddVertex", &GPUaddVertex);

		GetPetesProcedure("GPUsetfix", &GPUsetfix);
		GetPetesProcedure("GPUsetframelimit", &GPUsetframelimit);
	}

	~GPUPlugin()
	{
		if (m_petes)
			FreeLibrary(m_petes);
		if (m_pec)
			FreeLibrary(m_pec);
	}

	static GPUPlugin& Get()
	{
		static GPUPlugin instance;
		return instance;
	}

	void* GetPluginMem(u32 offset = 0) const
	{
		return reinterpret_cast<u8*>(m_petes)+offset;
	}

private:
	HMODULE m_petes;
	HMODULE m_pec;

	template<typename T>
	void GetPetesProcedure(const char* funcname, T* ppfunc)
	{
		if (!(*ppfunc))
			*ppfunc = reinterpret_cast<T>(::GetProcAddress(m_petes, funcname));
	}

	template<typename T>
	void GetPECProcedure(const char* funcname, T* ppfunc)
	{
		*ppfunc = reinterpret_cast<T>(::GetProcAddress(m_pec, funcname));
	}
};


