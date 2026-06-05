#include "parameter_parser.h"

#include <cstring>
#include <iostream>

bool parseParameters(int argc, char* argv[], Parameters &params) {

	bool parsingSuccess = true;
	constexpr int minWorkGroupDivider = 8;	//minimal possible value for work group divider

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--width") == 0 && i + 1 < argc) {
			int parsedValue = atoi(argv[i + 1]);
			if(parsedValue <= 0) {
				parsingSuccess = false;
				break; 
			}
			params.width = parsedValue;
			params.customResolution = true;
			i++;	//jump over value for this parameter
		}
		else if (strcmp(argv[i], "--height") == 0 && i + 1 < argc) {
			int parsedValue = atoi(argv[i + 1]);
			if(parsedValue <= 0) {
				parsingSuccess = false;
				break;
			}
			params.height = parsedValue;
			params.customResolution = true;
			i++;	//jump over value for this parameter
		}
		else if (strcmp(argv[i], "--particles") == 0 && i + 1 < argc) {
			int parsedValue = atoi(argv[i + 1]);
			if(parsedValue < 0) {
				parsingSuccess = false;
				break;
			}
			params.numParticles = parsedValue;
			params.customParticleCount = true;
			i++;	//jump over value for this parameter
		}
		else if (strcmp(argv[i], "--slimeRatio") == 0 && i + 1 < argc) {
			float parsedValue = static_cast<float>(atof(argv[i + 1]));
			if(parsedValue < 0.0f) {
				parsingSuccess = false;
				break;
			}
			params.slimeRatio = parsedValue;
			i++;	//jump over value for this parameter
		}
		else if (strcmp(argv[i], "--workGroupDivider") == 0 && i + 1 < argc) {
			int parsedValue = atoi(argv[i + 1]);
			if(parsedValue % minWorkGroupDivider || parsedValue <= 0) {
				parsingSuccess = false;
				break;
			}
			params.workGroupDivider = parsedValue;
			i++;	//jump over value for this parameter
		}
		else if (strcmp(argv[i], "--audioDevice") == 0 && i + 1 < argc) {
			std::string deviceName = "";

			deviceName += std::string(argv[i + 1]);
			i++;	//jump over value for this parameter

			params.audioDevice = deviceName;
		}
		else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
			printHelpMessage(argv[0], minWorkGroupDivider);
		} else {
  			parsingSuccess = false;
			break;
		}
	}

	//wait with this check after a possible user defined workGroupDivider was parsed
	if(params.width % params.workGroupDivider != 0 || params.height % params.workGroupDivider != 0) {
		parsingSuccess = false;
	}

    return parsingSuccess;
}

void printHelpMessage(char* programName, int workGroupDivider) {

    std::string helpMessage = "Usage: " + std::string(programName) + " [options]\n"
	+ "Options:\n"
	+ "  --help | -h              Show this help message\n"
	+ "  --width <pixels>         Set texture width (must be multiples of workGroupDivider: " + std::to_string(workGroupDivider) + ")\n"
	+ "  --height <pixels>        Set texture height (must be multiples of workGroupDivider: " + std::to_string(workGroupDivider) + ")\n"
	+ "  --particles <count>      Set number of particles (must be multiples of workGroupDivider: " + std::to_string(workGroupDivider) + ") (overwrites --slimeRatio <float>)\n"
	+ "  --slimeRatio <float>     Set number of particles as a ratio of window area\n"
	+ "  --workGroupDivider <int> Set the workGroupDivider for GPU (must be multiples of 8)\n"
	+ "  --audioDevice <name>     Set the audio device to use (in quotation marks if it contains spaces)\n";

    std::cout << helpMessage << std::endl;
}