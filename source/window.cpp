#include "window.h"

#include <iostream>


Window::Window(int width, int height, const std::string& title, bool customResolution) {
	windowWidth_ = width;
	windowHeight_ = height;

	bool init_SDL_Success = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_CAMERA | SDL_INIT_AUDIO);
	if (!init_SDL_Success) {
		std::cerr << "Error: SDL_Init failed with error message: " << SDL_GetError() << std::endl;
		exit(1);
	}

	//set to maximum display resolution if user provides none
	if(!customResolution) {
		// Display Stuff
		displays_ = SDL_GetDisplays(&numberOfDisplays_);

		const SDL_DisplayMode* displayMode = SDL_GetCurrentDisplayMode(*displays_);
		fractionalScalingFactor_ = displayMode->pixel_density;

		//set window to maximized non-fullscreen size
		SDL_Rect usableBounds;
		bool displayBoundSuccess = SDL_GetDisplayUsableBounds(*displays_, &usableBounds);

		if(displayBoundSuccess) {
			windowWidth_ = usableBounds.w;
			windowHeight_ = usableBounds.h;	
		} else {
			std::cerr << "Error: Couldn't set display resolution!" << std::endl;
		}
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

	window_ = SDL_CreateWindow(title.c_str(), windowWidth_, windowHeight_, SDL_WINDOW_OPENGL);
	glContext_ = SDL_GL_CreateContext(window_);

	//reducing OpenGL Version until it works
	while(glContext_ == NULL && minorVersion_ >= 2) {
		std::cerr << "Could not get newest OpenGL Version " << majorVersion_ << "." << minorVersion_ << "!" << std::endl;
		
		SDL_DestroyWindow(window_);
		
		minorVersion_--;
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minorVersion_);
		std::cerr << "Trying OpenGL Version: " << majorVersion_ << "." << minorVersion_ << std::endl;

		window_ = SDL_CreateWindow(title.c_str(), windowWidth_, windowHeight_, SDL_WINDOW_OPENGL);
		glContext_ = SDL_GL_CreateContext(window_);
	}

	if(window_ == nullptr) {
		std::cerr << "Error: SDL_CreateWindow failed with error message: " << SDL_GetError() << std::endl;
		exit(1);
	}

	if(glContext_ == nullptr) {
		std::cerr << "Error: SDL_GL_CreateContext failed with error message: " << SDL_GetError() << std::endl;
		exit(1);
	}

	//Measure of last resort using extensions for OpenGL Version 4.2
	if(minorVersion_ == 2) {
		std::cerr << "OpenGL Version 4.2 detected! Checking for necessary GL Extensions." << std::endl;
		if(!SDL_GL_ExtensionSupported("GL_ARB_compute_shader")) {
			std::cerr << "Compute Shader Extension not supported!" << std::endl;
			std::cerr << "Exiting Program!" << std::endl;
			exit(1);
		} else {
			std::cerr << "Compute Shader Extension supported!" << std::endl;
		}
		if(!SDL_GL_ExtensionSupported("GL_ARB_shader_storage_buffer_object")) {
			std::cerr << "Shader Storage Buffer Object Extension not supported!" << std::endl;
			std::cerr << "Exiting Program!" << std::endl;
			exit(1);
		} else {
			std::cerr << "Shader Storage Buffer Object Extension supported!" << std::endl;
		}
	}

	//All hope is lost. Exiting program for OpenGL 4.1 and below
	if(minorVersion_ < 2) {
		std::cerr << "Could not get required minimum OpenGL Version 4.2 for compute shaders!" << std::endl;
		std::cerr << "Exiting Program!" << std::endl;
		exit(1);
	}

	SDL_SetWindowResizable(window_, true);		// Allow window resizing
	SDL_GL_SetSwapInterval(1);					// Enable V-Sync
	SDL_MaximizeWindow(window_);				// necessary, because SDL_GetDisplayUsableBounds() doesn't work with Wayland

	glViewport(0, 0, windowWidth_, windowHeight_);

	GLenum status = glewInit();
	if (status != GLEW_OK) {
		std::cerr << "ERROR: Glew failed to initialize with error message: " << glewGetErrorString(status) << std::endl;
	}

	isClosed_ = false;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LINE_SMOOTH);

	#if _DEBUG
	setOpenGLDebugCallback();
	#endif
}

Window::~Window() {
	SDL_GL_DestroyContext(glContext_);
	SDL_DestroyWindow(window_);
	SDL_Quit();
}

void Window::updateViewport() {

    SDL_GetWindowSizeInPixels(window_, &windowWidth_, &windowHeight_);   // Get the window size in pixels
    glViewport(0, 0, windowWidth_, windowHeight_);						  // Update viewport to match window size
}

void Window::setFullscreen(){
	if (uiState_->fullscreen) {
		SDL_SetWindowFullscreen(window_, SDL_WINDOW_FULLSCREEN);
	} else {
		SDL_SetWindowFullscreen(window_, 0);
	}

	updateViewport();
}

void Window::Clear(float r, float g, float b, float a) {
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT);
}

bool Window::IsClosed() {
	return isClosed_;
}

void Window::Update() {
	SDL_GL_SwapWindow(window_);

	SDL_Event inputEvent;

	while (SDL_PollEvent(&inputEvent)) {
		ImGui_ImplSDL3_ProcessEvent(&inputEvent);

		switch (inputEvent.type) {
		case SDL_EVENT_KEY_DOWN:
			if(inputEvent.key.key == SDLK_ESCAPE) { if(!exitLock_) {isClosed_ = true;}}
			if(inputEvent.key.key == SDLK_F11) { exitLock_ = exitLock_ ? false : true; }
			break;
		case SDL_EVENT_QUIT:
			if(!exitLock_) {isClosed_ = true;}
			break;
		default:
			break;
		}
	}
}

void Window::printOpenGLExtensions() {
	GLint numExt = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &numExt);

	std::cout << "----------------------" << std::endl;
	std::cout << "Supported OpenGL Extensions: " << std::endl;
	for (int i = 0; i < numExt; i++) {
		std::cout << glGetStringi(GL_EXTENSIONS, (unsigned int)i) << std::endl;
	}
	std::cout << "----------------------" << std::endl;
}

void Window::onNotify(const Event event) {
	switch (event) {
		case Event::FULLSCREEN_TOGGLE:
			setFullscreen();		//check if fullscreen mode changed in UI and set it
			break;
		default:
			break;
	}
}

/*---------------------------------------------------------------
	OpenGL DebugCallbackFunction thanks to Morten Nobel-Jorgensen
	https://blog.nobel-joergensen.com/2013/02/17/debugging-opengl-part-2-using-gldebugmessagecallback/
---------------------------------------------------------------*/

void setOpenGLDebugCallback() {
	if (glDebugMessageCallback) {
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
	std::cout << "   message: " << message << std::endl;
	std::cout << "   type: ";

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
	std::cout << "   id: " << id << std::endl;
	std::cout << "   severity: ";

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