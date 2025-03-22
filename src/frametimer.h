#ifndef FRAMETIME_H
#define FRAMETIME_H

#include <utils/include.h>

class FrameTimer {
    using Clock = std::chrono::time_point<std::chrono::high_resolution_clock>;
public:
    FrameTimer();

    void start();
    uint64_t elapsed() const;

    void print(const std::string& name);

private:
    Clock m_start;
};

#endif
