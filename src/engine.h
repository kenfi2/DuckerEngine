#ifndef ENGINE_H
#define ENGINE_H

#include "window.h"
#include "frametimer.h"
#include <graphics/painter.h>

class UIManager;
class Engine {
public:
    Engine() = default;
    ~Engine();

    void start();

    uint64_t lastFps() const { return m_lastFps; }

private:
    void poll();
    void frame();
    void render();

    FrameTimer m_frameTimer;
    uint64_t m_lastFps = 0;
};

extern Engine* g_engine;
extern Window* g_window;
extern UIManager* g_ui;

#endif
