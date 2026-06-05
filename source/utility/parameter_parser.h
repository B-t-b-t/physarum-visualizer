#ifndef PARAMETER_PARSER_H
#define PARAMETER_PARSER_H

#include <string>

struct Parameters {
    int width = 800;
    int height = 800;
    int numParticles = 10000;
    float slimeRatio = 0.15f;
    int workGroupDivider = 8;
    std::string audioDevice = "";
    bool customResolution = false;
    bool customParticleCount = false;
};

bool parseParameters(int argc, char* argv[], Parameters &params);
void printHelpMessage(char* programName, int workGroupDivider);

#endif // PARAMETER_PARSER_H