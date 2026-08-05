#include <iostream>

#include <SDL3/SDL_main.h>

#include "./utility/parameter_parser.h"
#include "application.h"

int main(int argc, char* argv[]) {
	
	//Parse command line arguments
	Parameters params;

	if(!parseParameters(argc, argv, params)) {
		printHelpMessage(argv[0], params.workGroupDivider);
		return -1;
	}

	std::cout << "====================" << std::endl 
			  << "Physarum 0.6.0-alpha" << std::endl
			  << "====================" << std::endl;
			   
	Application app(params);
	app.run();

	return 0;
}