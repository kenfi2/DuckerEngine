#include "window.h"

Window::~Window()
{
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

bool Window::init()
{
    SDL_InitSubSystem(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_RED_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_GREEN_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_BLUE_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_ALPHA_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_STEREO, 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);

    w = 800;
    h = 600;

    m_sdlWindow = SDL_CreateWindow("Main window", w, h, 0);
    if(!m_sdlWindow)
        return false;

    m_running = true;
    return true;
}

void Window::poll()
{
    static SDL_Event e;
    while(SDL_PollEvent(&e)) {
        switch(e.type) {
            if(e.window.windowID != SDL_GetWindowID(m_sdlWindow))
                break;

            case SDL_EVENT_WINDOW_SHOWN:
                onShow();
                break;
            case SDL_EVENT_WINDOW_HIDDEN:
                onHide();
                break;
            case SDL_EVENT_WINDOW_EXPOSED:
                onExposed();
                break;
            case SDL_EVENT_WINDOW_MOVED:
                onMoved();
                break;
            case SDL_EVENT_WINDOW_RESIZED:
            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                onResize();
                break;
            case SDL_EVENT_WINDOW_MINIMIZED:
                onMinimized();
                break;
            case SDL_EVENT_WINDOW_MAXIMIZED:
                onMaximized();
                break;
            case SDL_EVENT_WINDOW_RESTORED:
                onRestore();
                break;
            case SDL_EVENT_WINDOW_ENTER_FULLSCREEN:
                break;
            case SDL_EVENT_WINDOW_FOCUS_GAINED: {
                onFocus(true);
                break;
            }
            case SDL_EVENT_WINDOW_LEAVE_FULLSCREEN:
                break;
            case SDL_EVENT_WINDOW_FOCUS_LOST: {
                onFocus(false);
                break;
            }
            break;
            case SDL_EVENT_QUIT: {
                m_running = false;
                onQuit();
                break;
            }
        }
    }
}

void Window::onShow()
{
}

void Window::onHide()
{
}

void Window::onExposed()
{
}

void Window::onMoved()
{
}

void Window::onResize()
{
}

void Window::onMinimized()
{
}

void Window::onMaximized()
{
}

void Window::onRestore()
{
}

void Window::onFocus(bool focused)
{
}

void Window::onQuit()
{
    m_running = false;
}
