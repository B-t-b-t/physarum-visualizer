#include "ParticleData.h"
#include <fstream>

#define PI 3.14159265f

ParticleData::ParticleData(unsigned int numParticles, unsigned int texWidth, unsigned int texHeight) {
	numParticles_ = numParticles;
	texWidth_ = texWidth;
	texHeight_ = texHeight;
}

void ParticleData::createAndSend() {
	int R_Outer = 3200;
	int R_Inner = 800;

	for (unsigned int i = 0; i < numParticles_; i++) {
		int r = sqrt((R_Outer - R_Inner) * (rand() % 10000) / 10000.0f + R_Inner);	//sqrt() to maintain a even point distribution (circle area is proportional to the square of the radius)
		float a = 2 * PI * (rand() % 10000) / 10000.0f;
		float x = texWidth_ / 2.0f + r * cos(a);
		float y = texHeight_ / 2.0f + r * sin(a);
		float speciesID = (rand() % 3) + 1;		//+1 to avoid speciesID 0 for Branch Avoidance in GLSL
		shaderData_.push_back({ x, y, a, speciesID });
	}

	ssbo_ = 0;
	glGenBuffers(1, &ssbo_);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_);
	//usage hint GL_DYNAMIC_COPY, because it is modified and read by GPU, but triggers a harmless warning when buffer is modified by CPU
	glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GLsizeiptr>(shaderData_.size() * sizeof(shader_data_t)), &shaderData_[0], GL_DYNAMIC_COPY);

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void ParticleData::recreateAndSend(unsigned int numParticles, unsigned int texWidth, unsigned int texHeight) {
	numParticles_ = numParticles;
	texWidth_ = texWidth;
	texHeight_ = texHeight;
	shaderData_.clear();
	
	int R_Outer = 3200;
	int R_Inner = 800;

	for (unsigned int i = 0; i < numParticles_; i++) {
		int r = sqrt((R_Outer - R_Inner) * (rand() % 10000) / 10000.0f + R_Inner);	//sqrt() to maintain a even point distribution (circle area is proportional to the square of the radius)
		float a = 2 * PI * (rand() % 10000) / 10000.0f;
		float x = texWidth_ / 2.0f + r * cos(a);
		float y = texHeight_ / 2.0f + r * sin(a);
		float speciesID = (rand() % 3) + 1;		//+1 to avoid speciesID 0 for Branch Avoidance in GLSL
		shaderData_.push_back({ x, y, a, speciesID });
	}

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_);
	//usage hint GL_DYNAMIC_COPY, because it is modified and read by GPU, but triggers a harmless warning when buffer is modified by CPU
	glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GLsizeiptr>(shaderData_.size() * sizeof(shader_data_t)), &shaderData_[0], GL_DYNAMIC_COPY);

	shader_data_t* ptr = (shader_data_t*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
	for (unsigned int i = 0; i < numParticles_; i++) {
		ptr[i] = shaderData_[i];
	}

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void ParticleData::printSSBO() {

	if (ssbo_ == 0) {
		std::cout << "SSBO not created" << std::endl;
		return;
	}

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_);

	shader_data_t* ptr;
	ptr = (shader_data_t*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	shaderData_.clear();

	for (unsigned int i = 0; i < numParticles_; i++) {
		shaderData_.push_back(ptr[i]);
	}

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	for (unsigned int i = 0; i < numParticles_; i++) {
		std::cout << "p" << i << ": " << shaderData_[i].position_x << " , " << shaderData_[i].position_y << " , " << shaderData_[i].angle << std::endl;
	}
}

void ParticleData::writeToFile(const std::string& filename) {
	std::ofstream outFile(filename + ".txt");
	if (outFile.is_open()) {
		for (unsigned int i = 0; i < numParticles_; i++) {
			outFile << "p" << i << ": " 
					<< shaderData_[i].position_x << " , " 
					<< shaderData_[i].position_y << " , " 
					<< shaderData_[i].angle << " , "
					<< shaderData_[i].speciesID << std::endl;
		}
		outFile.close();
		std::cout << "Data written to particle_data.txt" << std::endl;
	} else {
		std::cout << "Unable to open file" << std::endl;
	}

	// Write raw binary data
	std::ofstream binFile(filename + ".bin", std::ios::binary);
	if (binFile.is_open()) {
		binFile.write(reinterpret_cast<const char*>(shaderData_.data()), 
					 static_cast<std::streamsize>(shaderData_.size() * sizeof(shader_data_t)));
		binFile.close();
		std::cout << "Binary data written to " << filename << ".bin" << std::endl;
	} else {
		std::cout << "Unable to open binary file" << std::endl;
	}
}
