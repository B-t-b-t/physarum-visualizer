#ifndef WINDOW_H
#define WINDOW_H

#define SDL_MAIN_HANDLED

#include <string>

#include <GL/glew.h>
#include "imgui_impl_sdl3.h"
#include <SDL3/SDL.h>


#define APIENTRY GLEWAPIENTRY	//tell OpenGL debug callback function to use GLEW's calling convention macro

void setOpenGLDebugCallback();

void APIENTRY openglCallbackFunction(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam);

class Window
{
public:
	Window(int width, int height, const std::string& title, bool customResolution);

	void setFullscreen(bool fullscreen);

	void Clear(float r, float g, float b, float a);
	void Update();
	bool IsClosed();
	SDL_Window* getWindow() { return window_; }
	virtual ~Window();

	int getWindowWidth() { return windowWidth_; }
	int getWindowHeight() { return windowHeight_; }
	float getFractionalScalingFactor() { return fractionalScalingFactor_; }

	void setWindowWidth(int width) { windowWidth_ = width; }
	void setWindowHeight(int height) { windowHeight_ = height; }

	bool getExitLock() { return exitLock_; }
	bool setIsClosed(bool closed) { if(!exitLock_) {isClosed_ = closed; return true;} else {return false;} }

	void updateViewport();

	void printOpenGLExtensions();
	SDL_Window* getWindow() const { return window_; }
	SDL_GLContext getGLContext() const { return glContext_; }

private:

	SDL_Window* window_;
	SDL_GLContext glContext_;

	SDL_DisplayID* displays_;
	int numberOfDisplays_;

	int windowWidth_;
	int windowHeight_;
	float fractionalScalingFactor_;
	bool isClosed_;
	bool exitLock_ = false;
};

#endif // WINDOW_H
