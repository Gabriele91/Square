//
//  Square
//
//  Created by Gabriele on 18/10/17.
//  Copyright © 2016 Gabriele. All rights reserved.
//
#pragma once
#include "Square/Config.h"

namespace Square
{
namespace Time
{
	//get ticks
	SQUARE_API unsigned long long get_ticks_time();
	//get time in millisecond
	SQUARE_API double get_ms_time();
	//get time in second
    SQUARE_API double get_time();
	//wait until a time in millisecond (asleep, awake its last ms)
	SQUARE_API void wait_until_ms(double ms);
	//the timer of the system at 1 ms (Windows: else 15.6 ms), while enabled
	SQUARE_API void precise_timer(bool enable);
    //timer object
    class SQUARE_API Timer
    {
        
    public:
        
        Timer();
        
        void reset();
        
        double get_time() const;
        
        double get_time_ms() const;
        
    protected:
        
        double m_start_time{ 0 };
        
    };
    //fps counter
    class SQUARE_API FPSCounter
    {
    public:
        
        void reset();
        
        double count_frame();

		double get();
        
    private:
        
        Timer  m_timer;
        double m_count{ 0 };
        double m_last { 0 };
        
    };
}
}
