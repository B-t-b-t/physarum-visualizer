#include "parameter_parser.h"

#include <cstring>
#include <iostream>

int parseParameters(int argc, char* argv[], Parameters &params, int workGroupDivider) {

	bool parsingError = false;

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--width") == 0 && i + 1 < argc) {
			int parsedValue = atoi(argv[i + 1]);
			if(parsedValue <= 0) {
				parsingError = true;
				break; 
			}
			params.width = parsedValue;
			params.customResolution = true;
			i++;	//jump over value for this parameter
		}
		else if (strcmp(argv[i], "--height") == 0 && i + 1 < argc) {
			int parsedValue = atoi(argv[i + 1]);
			if(parsedValue <= 0) {
				parsingError = true;
				break;
			}
			params.height = parsedValue;
			params.customResolution = true;
			i++;	//jump over value for this parameter
		}
		else if (strcmp(argv[i], "--particles") == 0 && i + 1 < argc) {
			int parsedValue = atoi(argv[i + 1]);
			if(parsedValue < 0) {
				parsingError = true;
				break;
			}
			params.numParticles = parsedValue;
			params.customParticleCount = true;
			i++;	//jump over value for this parameter
		}
		else if (strcmp(argv[i], "--slimeRatio") == 0 && i + 1 < argc) {
			float parsedValue = static_cast<float>(atof(argv[i + 1]));
			if(parsedValue < 0.0f) {
				parsingError = true;
				break;
			}
			params.slimeRatio = parsedValue;
			i++;	//jump over value for this parameter
		}
		else if (strcmp(argv[i], "--audioDevice") == 0 && i + 1 < argc) {
			std::string deviceName = "";

			deviceName += std::string(argv[i + 1]);
			i++;	//jump over value for this parameter

			params.audioDevice = deviceName;
		}
		else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
			printHelpMessage(argv, workGroupDivider);
		} else {
  			parsingError = true;
			break;
		}
	}

    return parsingError ? -1 : 0;
}

void printHelpMessage(char* argv[], int workGroupDivider) {
    std::string helpMessage = "Usage: " + std::string(argv[0]) + " [options]\n"
	+ "Options:\n"
	+ "  --help | -h          Show this help message\n"
	+ "  --width <pixels>     Set texture width (multiples of " + std::to_string(workGroupDivider) + ")\n"
	+ "  --height <pixels>    Set texture height (multiples of " + std::to_string(workGroupDivider) + ")\n"
	+ "  --particles <count>  Set number of particles (multiples of " + std::to_string(workGroupDivider) + ") (overwrites --slimeRatio <float>)\n"
	+ "  --slimeRatio <float> Set number of particles as a ratio of window area\n"
	+ "  --audioDevice <name> Set the audio device to use (in quotation marks if it contains spaces)\n";

    std::cout << helpMessage << std::endl;
}