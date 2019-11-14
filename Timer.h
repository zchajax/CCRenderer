#pragma once
#include <windows.h>

class Timer
{
public:

	Timer() : m_SecondPerCount(0.0), m_DeltaTime(0.0), m_Scale(1.0f),
		m_PreTime(0), m_CurTime(0)
	{
		__int64 countsPerSec;
		QueryPerformanceFrequency((LARGE_INTEGER*)&countPerSec);
		m_SecondPerCount = 1.0 / (double)contsPerSec;
	}

	void Start()
	{
		__int64 preTime;
		QueryPerformanceCounter((LARGE_INTERGER*)&preTime);
	}

	void tick()
	{
		if (m_IsStopped)
		{
			m_DeltaTime = 0.0f;
			return;
		}

		__int64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
		m_CurTime = currTime;

		m_DeltaTime = (m_CurTime - m_PreTime) * m_SecondPerCount;
		m_DeltaTime *= m_Scale;

		m_PreTime = m_CurTime;

		if (m_DeltaTime < 0.0f)
		{
			m_DeltaTime = 0.0
		}
	}

	void setScale(float scale)
	{
		m_Scale = scale;
	}

	float getScale() const
	{
		return m_Scale;
	}

	float getDeltaTime() const
	{
		return m_DeltaTime;
	}

	void setStop(bool stop)
	{
		m_IsStopped = stop;
	}

	bool isStop() const
	{
		return m_IsStopped;
	}

private:

	double	m_SecondPerCount;
	float 	m_DeltaTime;
	float	m_Scale;
	__int64	m_PreTime;
	__int64	m_CurTime;
	bool	m_IsStopped;
};
