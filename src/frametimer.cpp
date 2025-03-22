#include "frametimer.h"

FrameTimer::FrameTimer()
{
    start();
}

void FrameTimer::start()
{
    m_start = std::chrono::high_resolution_clock::now();
}

uint64_t FrameTimer::elapsed() const
{
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(end - m_start).count();
}

void FrameTimer::print(const std::string& name)
{
    std::cout << name << " " << elapsed() << " microseconds." << std::endl;
}
