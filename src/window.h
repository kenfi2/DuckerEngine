#ifndef WINDOW_H
#define WINDOW_H

#include <utils/include.h>

class Window {
public:
	Window() : m_running(false), w(0), h(0) { }
	~Window();

	bool init();

	SDL_Window* const getSDLWindow() const { return m_sdlWindow; }
	
	bool isRunning() const { return m_running; }

	uint32_t getWidth() const { return w; }
	uint32_t getHeight() const { return h; }

	void poll();

public:
	void onShow();
	void onHide();
	void onExposed();
	void onMoved();
	void onResize();
	void onMinimized();
	void onMaximized();
	void onRestore();
	void onFocus(bool focused);
	void onQuit();

private:
	SDL_Window* m_sdlWindow = nullptr;
	bool m_running;
	uint32_t w;
	uint32_t h;
};

#endif
