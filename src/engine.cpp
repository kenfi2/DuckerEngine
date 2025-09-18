#include "engine.h"

#include <ui/ui.h>

#include <chrono>
#include <windows.h>

#pragma comment(lib, "winmm.lib")

Engine* g_engine;
Painter* g_painter;
Window* g_window;
UIManager* g_ui;

void Engine::start()
{
    g_engine = this;

    SDL_Log("Starting...");
    g_window = new Window;
    if(!g_window)
        return;

    if(!g_window->init()) {
        SDL_Log("SDL_CreateWindow: ", SDL_GetError());
        return;
    }

    g_painter = new Painter;
    if(!g_painter) {
        SDL_Log("Failed to create painter!");
        return;
    }

    if(!g_painter->create())
        return;

    SDL_Log("Context has been created. Starting poll");

    g_ui = new UIManager;
    g_ui->init();
    g_ui->resize(SizeI(100, 100));

    m_frameTimer.start();

    timeBeginPeriod(1);

    poll();

    timeEndPeriod(1);
}

void Engine::poll()
{
    bool running;
    do {
        g_window->poll();
        running = g_window->isRunning();
        if(running) {
            render();
            frame();
        }
    } while(running);

    g_painter->destroy();
}

void Engine::frame()
{
    static uint32_t frameCount = 0;
    static auto lastUpdateTime = std::chrono::steady_clock::now();

    frameCount++;

    auto currentTime = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastUpdateTime).count();

    if(elapsed >= 1000000) {
        m_lastFps = static_cast<double>(frameCount) * 1000000.0 / elapsed;
        double avgFrameTime = static_cast<double>(elapsed) / frameCount / 1000.0;

        if(auto sdlWindow = g_window->getSDLWindow()) {
            std::stringstream ss;
            ss << "Ducker FPS: " << static_cast<int>(m_lastFps);
            ss << " | Avg: " << std::fixed << std::setprecision(2) << avgFrameTime << "ms";
            SDL_SetWindowTitle(sdlWindow, ss.str().c_str());
        }

        lastUpdateTime = currentTime;
        frameCount = 0;
    }
}

#include <thread>

void Engine::render()
{
    g_painter->beginRender();
    g_ui->render();
    g_painter->flushRender();
    g_painter->endRender();
}

Engine::~Engine()
{
    if(g_window)
        delete g_window;
    if(g_painter)
        delete g_painter;
    if(g_ui)
        delete g_ui;
}
