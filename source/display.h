#ifndef DISPLAY_H
#define DISPLAY_H
#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <GL/glew.h>
#include <string>
#include "imgui_impl_sdl3.h"


#define APIENTRY GLEWAPIENTRY

void setOpenGLDebugCallback();

void APIENTRY openglCallbackFunction(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam);

class Display
{
public:
	Display(int width, int height, const std::string& title, int workGroupSize, bool customResolution);

	void setFullscreen(bool fullscreen);

	void Clear(float r, float g, float b, float a);
	void Update();
	bool IsClosed();
	SDL_Window* getWindow() { return m_window; }
	virtual ~Display();

	int getWindowWidth() { return windowWidth_; }
	int getWindowHeight() { return windowHeight_; }

	void setWindowWidth(int width) { windowWidth_ = width; }
	void setWindowHeight(int height) { windowHeight_ = height; }

	bool getExitLock() { return m_exitLock; }
	bool setIsClosed(bool closed) { if(!m_exitLock) {m_isClosed = closed; return true;} else {return false;} }

	void updateViewport();

	void printOpenGLExtensions();
	SDL_Window* getWindow() const { return m_window; }
	SDL_GLContext getGLContext() const { return m_glContext; }

protected:

private:

	SDL_Window* m_window;
	SDL_GLContext m_glContext;
	int windowWidth_;
	int windowHeight_;
	bool m_isClosed;
	bool m_exitLock = false;
};

#endif // DISPLAY_H
