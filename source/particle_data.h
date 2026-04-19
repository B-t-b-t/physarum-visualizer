#ifndef PARTICLE_DATA_H
#define PARTICLE_DATA_H

#include <iostream>
#include <math.h>
#include <vector>

#include <GL/glew.h>

class ParticleData {
public:
	ParticleData(int numParticles, int texWidth, int texHeight);

	void createAndSend();
	void recreateAndSend(int numParticles, int texWidth, int texHeight);
	void printSSBO();
	void writeToFile(const std::string& filename);

private:
	int numParticles_;
	int texWidth_;
	int texHeight_;

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

