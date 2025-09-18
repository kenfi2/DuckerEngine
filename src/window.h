#ifndef WINDOW_H
#define WINDOW_H

#include <utils/include.h>
#include <utils/size.h>

class Window {
public:
	Window() { }
	~Window();

	bool init();

	SDL_Window* const getSDLWindow() const { return m_sdlWindow; }
	
	bool isRunning() const { return m_running; }

	uint32_t getWidth() const { return m_size.w; }
	uint32_t getHeight() const { return m_size.h; }

	void poll();
	void resize(const SizeI& size);

public:
	void onShow();
	void onHide();
	void onExposed();
	void onMoved();
	void onResize(const SizeI& size);
	void onMinimized();
	void onMaximized();
	void onRestore();
	void onFocus(bool focused);
	void onQuit();

private:
	SizeI m_size;
	SizeI m_resolution;
	SDL_Window* m_sdlWindow = nullptr;
	bool m_running = false;
	bool m_visible = false;
};

#endif
