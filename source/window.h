#ifndef WINDOW_H
#define WINDOW_H

#include <GL/glew.h>
#include <string>

#include "SDL3/SDL_events.h"    // for SDL_EventType, SDL_EventAction, SDL_P...
#include "SDL3/SDL_video.h"

#include "application_state.h"  // for ApplicationState
#include "utility/observer.h"

#define APIENTRY GLEWAPIENTRY	//tell OpenGL debug callback function to use GLEW's calling convention macro

void setOpenGLDebugCallback();

void APIENTRY openglCallbackFunction(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam);

class Window : public Observer {
public:
	Window();
	Window(const std::string& title, ApplicationState* appState, bool customResolution);

	void setFullscreen();

	void swapBuffers();
	void processWindowEvents(SDL_Event* event);
	bool isClosing() { return isClosing_ || appState_->exitProgram; };	//poll appState to immediately react in main loop to exit request from UI
	SDL_Window* getWindow() { return window_; }
	virtual ~Window();

	int getWindowWidth() { return windowWidth_; }
	int getWindowHeight() { return windowHeight_; }
	float getFractionalScalingFactor() { return fractionalScalingFactor_; }

	void setWindowWidth(int width) { windowWidth_ = width; }
	void setWindowHeight(int height) { windowHeight_ = height; }

	void updateViewport();

	void printOpenGLExtensions();
	SDL_Window* getWindow() const { return window_; }
	SDL_GLContext getGLContext() const { return glContext_; }

	void onNotify(const UserEvent event) override;

private:

	SDL_Window* window_;
	SDL_GLContext glContext_;

	SDL_DisplayID* displays_;
	int numberOfDisplays_;

	ApplicationState* appState_;

	int windowWidth_;
	int windowHeight_;
	float fractionalScalingFactor_;
	bool isClosing_;
};

#endif // WINDOW_H
