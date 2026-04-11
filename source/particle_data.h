#ifndef PARTICLE_DATA_H
#define PARTICLE_DATA_H

#include <iostream>
#include <math.h>
#include <vector>

#include <GL/glew.h>

class ParticleData {
public:
	ParticleData(unsigned int numParticles, unsigned int texWidth, unsigned int texHeight);

	void createAndSend();
	void recreateAndSend(unsigned int numParticles, unsigned int texWidth, unsigned int texHeight);
	void printSSBO();
	void writeToFile(const std::string& filename);

private:
	unsigned int numParticles_;
	unsigned int texWidth_;
	unsigned int texHeight_;

	struct shader_data_t {
		float position_x;
		float position_y;
		float angle;
		float speciesID;
	};

	GLuint ssbo_;

	std::vector<shader_data_t> shaderData_;
};

#endif // PARTICLE_DATA_H

