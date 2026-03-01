#include "display.h"
#include <iostream>
//#include <glm/glm.hpp>


Display::Display(int width, int height, const std::string& title, int workGroupSize, bool customResolution) {
	windowWidth_ = width;
	windowHeight_ = height;

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_CAMERA | SDL_INIT_AUDIO);

	if(!customResolution) {
		// Display Stuff
		int num_displays;
		SDL_DisplayID *displays = SDL_GetDisplays(&num_displays);

		const SDL_DisplayMode* displayMode = SDL_GetCurrentDisplayMode(*displays);

		std::cout << "Display Id: " << displayMode->displayID << " " << displayMode->w << "x" << displayMode->h << " " << displayMode->refresh_rate << "Hz" << std::endl;
		std::cout << "Display Format: " << SDL_GetPixelFormatName(displayMode->format) << std::endl;

		windowWidth_ = (displayMode->w) - (displayMode->w % workGroupSize);		//make sure width is multiple of workgroup size
		windowHeight_ = (displayMode->h) - (displayMode->h % workGroupSize);	//make sure height is multiple of workgroup size
	}

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	#if _DEBUG
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
	#else
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
	#endif

	int majorVersion_ = 4;
	int minorVersion_ = 6;
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, majorVersion_);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minorVersion_);
	std::cout << "Requested OpenGL Version: " << majorVersion_ << "." << minorVersion_ << std::endl;

	m_window = SDL_CreateWindow(title.c_str(), windowWidth_, windowHeight_, SDL_WINDOW_OPENGL);
	m_glContext = SDL_GL_CreateContext(m_window);

	//reducing OpenGL Version until it works
	while(m_glContext == NULL && minorVersion_ >= 2) {
		std::cerr << "Could not get requested OpenGL Version!" << std::endl;
		
		SDL_DestroyWindow(m_window);
		
		minorVersion_--;
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minorVersion_);
		std::cerr << "Trying OpenGL Version: " << majorVersion_ << "." << minorVersion_ << std::endl;

		m_window = SDL_CreateWindow(title.c_str(), windowWidth_, windowHeight_, SDL_WINDOW_OPENGL);
		m_glContext = SDL_GL_CreateContext(m_window);
	} 

	//Measure of last resort using extensions for OpenGL Version 4.2
	if(minorVersion_ == 2) {
		std::cout << "OpenGL Version 4.2 detected! Checking for necessary GL Extensions." << std::endl;
		if(!SDL_GL_ExtensionSupported("GL_ARB_compute_shader")) {
			std::cerr << "Compute Shader Extension not supported!" << std::endl;
			std::cerr << "Exiting Program!" << std::endl;
			exit(1);
		} else {
			std::cout << "Compute Shader Extension supported!" << std::endl;
		}
		if(!SDL_GL_ExtensionSupported("GL_ARB_shader_storage_buffer_object")) {
			std::cerr << "Shader Storage Buffer Object Extension not supported!" << std::endl;
			std::cerr << "Exiting Program!" << std::endl;
			exit(1);
		} else {
			std::cout << "Shader Storage Buffer Object Extension supported!" << std::endl;
		}
	}

	//All hope is lost. Exiting program for OpenGL 4.1 and below
	if(minorVersion_ < 2) {
		std::cerr << "Could not get required minimum OpenGL Version 4.2 for compute shaders!" << std::endl;
		std::cerr << "Exiting Program!" << std::endl;
		exit(1);
	} else {
		std::cout << "Using OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
	}

	SDL_SetWindowResizable(m_window, true);		// Allow window resizing
	SDL_GL_SetSwapInterval(1);					// Enable V-Sync

	glViewport(0, 0, windowWidth_, windowHeight_);

	GLenum status = glewInit();
	if (status != GLEW_OK) {
		std::cerr << "Glew failed to initialize!" << std::endl;
		std::cerr << "Glew Error: " << glewGetErrorString(status) << std::endl;
	}

	m_isClosed = false;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LINE_SMOOTH);

	#if _DEBUG
	setOpenGLDebugCallback();
	#endif
}

Display::~Display() {
	SDL_GL_DestroyContext(m_glContext);
	SDL_DestroyWindow(m_window);
	SDL_Quit();
}

void Display::updateViewport() {

    SDL_GetWindowSizeInPixels(m_window, &windowWidth_, &windowHeight_);   // Get the window size in pixels
    glViewport(0, 0, windowWidth_, windowHeight_);						  // Update viewport to match window size
}

void Display::setFullscreen(bool fullscreen){
	if (fullscreen) {
		SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN);
	} else {
		SDL_SetWindowFullscreen(m_window, 0);
	}

	updateViewport();
}

void Display::Clear(float r, float g, float b, float a) {
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT);
}

bool Display::IsClosed() {
	return m_isClosed;
}

void Display::Update() {
	SDL_GL_SwapWindow(m_window);

	SDL_Event inputEvent;

	while (SDL_PollEvent(&inputEvent)) {
		ImGui_ImplSDL3_ProcessEvent(&inputEvent);

		switch (inputEvent.type) {
		case SDL_EVENT_KEY_DOWN:
			if(inputEvent.key.key == SDLK_ESCAPE) { if(!m_exitLock) {m_isClosed = true;}}
			if(inputEvent.key.key == SDLK_F11) { m_exitLock = m_exitLock ? false : true; }
			break;
		case SDL_EVENT_QUIT:
			if(!m_exitLock) {m_isClosed = true;}
			break;
		default:
			break;
		}
	}
}

void Display::printOpenGLExtensions() {
	GLint numExt = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &numExt);

	std::cout << "----------------------" << std::endl;
	std::cout << "Supported OpenGL Extensions: " << std::endl;
	for (int i = 0; i < numExt; i++) {
		std::cout << glGetStringi(GL_EXTENSIONS, (unsigned int)i) << std::endl;
	}
	std::cout << "----------------------" << std::endl;
}

/*---------------------------------------------------------------
	OpenGL DebugCallbackFunction thanks to Morten Nobel-Jorgensen
	https://blog.nobel-joergensen.com/2013/02/17/debugging-opengl-part-2-using-gldebugmessagecallback/
---------------------------------------------------------------*/

void setOpenGLDebugCallback() {
	if (glDebugMessageCallback) {
		std::cout << "Register OpenGL debug callback " << std::endl;
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(openglCallbackFunction, nullptr);
		GLuint unusedIds = 0;
		glDebugMessageControl(GL_DONT_CARE,
			GL_DONT_CARE,
			GL_DONT_CARE,
			0,
			&unusedIds,
			true);
	}
	else {
		std::cerr << "glDebugMessageCallback not available" << std::endl;
	}
}

void APIENTRY openglCallbackFunction(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam) {

	source = source;		// unused
	length = length;		// unused
	userParam = userParam;	// unused

	if (id == 131185) {
		return;
	}

	std::cout << "---------------------opengl-callback-start------------" << std::endl;
	std::cout << "message: " << message << std::endl;
	std::cout << "type: ";

	switch (type) {
	case GL_DEBUG_TYPE_ERROR:
		std::cout << "ERROR";
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		std::cout << "DEPRECATED_BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		std::cout << "UNDEFINED_BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		std::cout << "PORTABILITY";
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		std::cout << "PERFORMANCE";
		break;
	}

	std::cout << std::endl;
	std::cout << "id: " << id << std::endl;
	std::cout << "severity: ";

	switch (severity) {
	case GL_DEBUG_SEVERITY_LOW:
		std::cout << "LOW";
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		std::cout << "MEDIUM";
		break;
	case GL_DEBUG_SEVERITY_HIGH:
		std::cout << "HIGH";
		break;
	}
	std::cout << std::endl;
	std::cout << "---------------------opengl-callback-end--------------" << std::endl;
}