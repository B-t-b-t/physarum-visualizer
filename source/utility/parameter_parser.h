#ifndef PARAMETER_PARSER_H
#define PARAMETER_PARSER_H

#include <string>

struct Parameters {
    int width = 800;
    int height = 800;
    int numParticles = 10000;
    float slimeRatio = 0.15f;
    std::string audioDevice = "";
    bool customResolution = false;
    bool customParticleCount = false;
};

int parseParameters(int argc, char* argv[], Parameters &params, int workGroupDivider);
void printHelpMessage(char* argv[], int workGroupDivider);

#endif // PARAMETER_PARSER_H