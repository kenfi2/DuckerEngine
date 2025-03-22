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

    if(!g_painter->init())
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
}

void Engine::frame()
{
    static uint32_t frameCount = 0;
    static std::chrono::microseconds updateInterval(1000000);
    static uint32_t totalLostFrames = 0;

    uint64_t elapsed = m_frameTimer.elapsed();

    frameCount++;
    totalLostFrames += g_painter->getLostFrames();

    if(elapsed >= 1000000) {
        if(auto sdlWindow = g_window->getSDLWindow()) {
            m_lastFps = (frameCount * 1000000) / elapsed / 2; // divided by 2 cause we toggle between upload and draw
            std::stringstream ss;
            ss << "Ducker FPS: " << m_lastFps;
            ss << " Average time: " << (double)(elapsed) / frameCount / 1000.0;
            ss << " Lost frames: " << totalLostFrames;
            SDL_SetWindowTitle(sdlWindow, ss.str().c_str());
        }
        m_frameTimer.start();
        frameCount = 0;
        totalLostFrames = 0;
    }
}

void Engine::render()
{
    g_painter->beginFrame();
    g_ui->render();
    g_painter->endFrame();
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
