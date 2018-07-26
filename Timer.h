#pragma once

#include <chrono>

class Timer
{
public:    
    inline void Start()
    {
        start_ = std::chrono::steady_clock::now();
    }

    inline void Stop()
    {
        end_ = std::chrono::steady_clock::now();
    }

    inline double Duration()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(end_ - start_).count();        
    }
    
private:
    std::chrono::time_point<std::chrono::steady_clock> start_, end_;
};


