#include <algorithm>
#include <deque>
#include <iomanip>
#include <iostream>

#include <fftw3.h>
#include <SDL3/SDL_camera.h>
#include <SDL3/SDL_main.h>

#include "./audio/audio_processor.h"
#include "./graphics/framebuffer.h"
#include "./utility/parameter_parser.h"
#include "application.h"

int main(int argc, char* argv[]) {

	// Default values for starting without command line arguments
	int workGroupDivider = 8;
	
	//Parse command line arguments
	Parameters params;

	int parseSuccess = parseParameters(argc, argv, params, workGroupDivider);

	if(parseSuccess == -1) {
		printHelpMessage(argv, workGroupDivider);
		return 0;
	}

	Application app(params, workGroupDivider);
	app.run();

	return 0;
}