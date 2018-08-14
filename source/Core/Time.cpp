//
//  Square
//
//  Created by Gabriele on 18/10/17.
//  Copyright © 2016 Gabriele. All rights reserved.
//
#include "Square/Core/Time.h"
// os libs
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

namespace Square
{
namespace Time
{
    //////////////////////////////////////////////////////////////////////
	//ticks
    unsigned long long get_ticks_time()
	{
		unsigned long long val;
#ifdef _WIN32
		QueryPerformanceCounter((LARGE_INTEGER *)&val);
#else 
		timeval time_val;
		gettimeofday(&time_val, nullptr);
		val = (unsigned long long)time_val.tv_sec * (1000 * 1000) + (unsigned long long)time_val.tv_usec;
#endif
		return val;
	}
	//time in millisecond
    double get_ms_time()
	{
		static double coe = 1.0 / 1000.0;
#ifdef _WIN32
		static unsigned long long freq = 0;
		if (freq == 0)
		{
			QueryPerformanceFrequency((LARGE_INTEGER *)&freq);
			coe = 1000.0 / freq;
		}
#endif
		return get_ticks_time() * coe;
	}
    //time in second
    double get_time()
    {
        return get_ms_time() / 1000.0;
    }
    //////////////////////////////////////////////////////////////////////
    //  Timer
    Timer::Timer()
    {
        reset();
    }
    
    void Timer::reset()
    {
        m_start_time = Time::get_time();
    }
    
    double Timer::get_time() const
    {
        return Time::get_time() - m_start_time;
    }
    
    double Timer::get_time_ms() const
    {
        return Time::get_ms_time() - m_start_time * 1000.0;
    }
    ///////////////////////////////////////////////////////////
    // FPSCounter
    void FPSCounter::reset()
    {
        m_timer.reset();
        m_count = 0;
        m_last = 0;
    }

	double FPSCounter::get()
	{
		return m_last;
	}

    double FPSCounter::count_frame()
    {
        //sum
        m_count += 1;
        
        //time
        double time = m_timer.get_time();
        
        //return fps
        if (time >= 1.0)
        {
            //compute number of frames
            m_last = m_count / time;
            //reset timer
            m_timer.reset();
            //reset count
            m_count = 0;
        }
        
        //return last
        return m_last;
    }
}
}
