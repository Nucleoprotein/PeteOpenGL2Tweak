#pragma once

#include <windows.h>

class Timer
{
public:
	Timer()
	{
		QueryPerformanceFrequency(&m_frequency);
		m_startCount.QuadPart = 0;
		m_endCount.QuadPart = 0;

		m_stopped = 0;
		m_startTimeInMicroSec = 0;
		m_endTimeInMicroSec = 0;
	}

	~Timer()
	{

	}

	void Start()
	{
		m_startCount.QuadPart = 0;
		QueryPerformanceCounter(&m_startCount);
	}

	void Stop()
	{
		m_stopped = true;
		QueryPerformanceCounter(&m_endCount);
	}

	double GetElapsedTimeInMicroSec()
	{
		if (!m_stopped)
			QueryPerformanceCounter(&m_endCount);

		m_startTimeInMicroSec = m_startCount.QuadPart * (1000000.0 / m_frequency.QuadPart);
		m_endTimeInMicroSec = m_endCount.QuadPart * (1000000.0 / m_frequency.QuadPart);

		return m_endTimeInMicroSec - m_startTimeInMicroSec;
	}

	double GetElapsedTimeInMilliSec()
	{
		return GetElapsedTimeInMicroSec() * 0.001;
	}

	double GetElapsedTimeInSec()
	{
		return GetElapsedTimeInMicroSec() * 0.000001;
	}

	double GetElapsedTime()
	{
		return GetElapsedTimeInSec();
	}

private:
	double m_startTimeInMicroSec;
	double m_endTimeInMicroSec;
	bool m_stopped;
	LARGE_INTEGER m_frequency;
	LARGE_INTEGER m_startCount;
	LARGE_INTEGER m_endCount;
};