#ifndef PARTICLE_DATA_H
#define PARTICLE_DATA_H

#include <iostream>
#include <math.h>
#include <vector>

#include <GL/glew.h>

class ParticleData {
public:
	ParticleData();

	void createAndSend(int numParticles, int texWidth, int texHeight);
	void printSSBO();
	void writeToFile(const std::string& filename);

private:
	void createParticleCircle();

	int numParticles_ = 0;
	int texWidth_ = 0;
	int texHeight_ = 0;

	bool bufferAlreadyCreated_ = false;

	struct shader_data_t {
		float position_x;
		float position_y;
		float angle;
		float speciesID;
	};

	GLuint ssbo_ = 0;

	std::vector<shader_data_t> shaderData_;
};

#endif // PARTICLE_DATA_H

