#include "particle_data.h"

#include <cmath>
#include <fstream>
#include <GL/glew.h>
#include <iostream>
#include <stdlib.h>


ParticleData::ParticleData() {
}

void ParticleData::createAndSend(int numParticles, int texWidth, int texHeight) {

	numParticles_ = numParticles;
	texWidth_ = texWidth;
	texHeight_ = texHeight;

	createUniformDistribution();

	if(!bufferAlreadyCreated_) {
		glGenBuffers(1, &ssbo_);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_);
	}

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_);
	//usage hint GL_DYNAMIC_COPY, because it is constantly modified and read by GPU, but triggers a harmless warning when buffer is modified by CPU
	glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GLsizeiptr>(shaderData_.size() * sizeof(shader_data_t)), &shaderData_[0], GL_DYNAMIC_COPY);

	if(bufferAlreadyCreated_) {
		shader_data_t* ptr = (shader_data_t*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
		for (int i = 0; i < numParticles_; i++) {
			ptr[i] = shaderData_[(unsigned int) i];
		}
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	}

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	bufferAlreadyCreated_ = true;
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

	for (int i = 0; i < numParticles_; i++) {
		shaderData_.push_back(ptr[i]);
	}

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	for (int i = 0; i < numParticles_; i++) {
		std::cout << "p" << i << ": " << shaderData_[(unsigned int) i].position_x << " , " << shaderData_[(unsigned int) i].position_y << " , " << shaderData_[(unsigned int) i].angle << std::endl;
	}
}

bool ParticleData::writeToFile(const std::filesystem::path& filePath) {
	bool isSuccess = false;

	if(filePath.extension() == ".txt") {
		std::ofstream outFile(filePath.string());

		if (outFile.is_open()) {
			for (int i = 0; i < numParticles_; i++) {
				outFile << "p" << i << ": " 
						<< shaderData_[(unsigned int) i].position_x << " , " 
						<< shaderData_[(unsigned int) i].position_y << " , " 
						<< shaderData_[(unsigned int) i].angle << " , "
						<< shaderData_[(unsigned int) i].speciesID << std::endl;
			}
			outFile.close();
			std::cout << "Data written to " << filePath << std::endl;
			isSuccess = true;
		} else {
			std::cerr << "Unable to open file: " << filePath << std::endl;
		}
	} else if(filePath.extension() == ".bin") {
		// Write raw binary data
		std::ofstream binFile(filePath.string(), std::ios::binary);

		if (binFile.is_open()) {
			binFile.write(reinterpret_cast<const char*>(shaderData_.data()), 
						  static_cast<std::streamsize>(shaderData_.size() * sizeof(shader_data_t)));
			binFile.close();
			std::cout << "Binary data written to " << filePath << std::endl;
			isSuccess = true;
		} else {
			std::cerr << "Unable to open binary file: " << filePath << std::endl;
		}
	} else {
		std::cerr << "Unsupported file extension for file: " << filePath << std::endl;
	}

	return isSuccess;
}

void ParticleData::createParticleCircle() {
	shaderData_.clear();
	
	constexpr float PI = 3.14159265f;

	int smallerTexDim = (texWidth_ < texHeight_) ? texWidth_ : texHeight_;

	const int R_Outer = smallerTexDim * 0.8;
	const int R_Inner = smallerTexDim * 0.2;

	for (int i = 0; i < numParticles_; i++) {
		int r = sqrt((R_Outer - R_Inner) * (rand() % 10000) / 10000.0f + R_Inner);	//sqrt() to maintain a even point distribution (circle area is proportional to the square of the radius)
		float a = 2 * PI * (rand() % 10000) / 10000.0f;
		float x = texWidth_ / 2.0f + r * cos(a);
		float y = texHeight_ / 2.0f + r * sin(a);
		float speciesID = (rand() % 3) + 1;		//+1 to avoid speciesID 0 for Branch Avoidance in GLSL
		shaderData_.push_back({ x, y, a, speciesID });
	}
}

void ParticleData::createUniformDistribution() {
	shaderData_.clear();

	for (int i = 0; i < numParticles_; i++) {
		float x = static_cast<float>(rand() % texWidth_);
		float y = static_cast<float>(rand() % texHeight_);
		float a = 2 * 3.14159265f * (rand() % 10000) / 10000.0f;
		float speciesID = (rand() % 3) + 1;		//+1 to avoid speciesID 0 for Branch Avoidance in GLSL
		shaderData_.push_back({ x, y, a, speciesID });
	}
}