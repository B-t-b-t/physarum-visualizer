#include "shader.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include <SDL3/SDL.h>

static void CheckShaderError(GLuint shader, GLuint flag, const std::string& errorMessage);
static std::string LoadShader(const std::string& filename);
static GLuint CreateShader(const std::string& text, GLenum shaderType, std::string& filename);

Shader::Shader() {
}

Shader::Shader(const std::string& filename, ShaderType shaderType) {
	shaderType_ = shaderType;
	filename_ = filename;

	int glMajorVersion, glMinorVersion;
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &glMajorVersion);
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &glMinorVersion);

	std::string shaderString = "#version " + std::to_string(glMajorVersion) + std::to_string(glMinorVersion) + "0\n";

	if(glMajorVersion == 4 && glMinorVersion == 2 && shaderType_ == ShaderType::COMPUTE_SHADER) {
		shaderString.append("#extension GL_ARB_compute_shader : enable\n");
		shaderString.append("#extension GL_ARB_shader_storage_buffer_object : enable\n");
	}

	shaderString.append(LoadShader(filename));

	switch (shaderType_)
	{
	case ShaderType::VERTEX_SHADER:
		shaderID_ = CreateShader(shaderString, GL_VERTEX_SHADER, filename_);
		break;
	case ShaderType::FRAGMENT_SHADER:
		shaderID_ = CreateShader(shaderString, GL_FRAGMENT_SHADER, filename_);
		break;
	case ShaderType::COMPUTE_SHADER:
		shaderID_ = CreateShader(shaderString, GL_COMPUTE_SHADER, filename_);
		break;
	default:
		break;
	}

}

Shader::Shader(Shader&& rhs) {
	this->filename_ = rhs.filename_;
	this->shaderID_ = rhs.shaderID_;
	this->shaderType_ = rhs.shaderType_;
	rhs.shaderID_ = 0;
}

Shader& Shader::operator=(Shader&& rhs) {
	this->filename_ = rhs.filename_;
	this->shaderID_ = rhs.shaderID_;
	this->shaderType_ = rhs.shaderType_;
	rhs.shaderID_ = 0;
	return *this;
}

Shader::~Shader() {
	glDeleteShader(shaderID_);
}

static GLuint CreateShader(const std::string& text, GLenum shaderType, std::string& filename) {
	GLuint shader = glCreateShader(shaderType);

	if (shader == 0) {
		std::cerr << "Error: Shader creation failed!" << std::endl;
    }

	const GLchar* shaderSourceStrings[1];
	GLint shaderSourceStringLengths[1];
	shaderSourceStrings[0] = text.c_str();
	shaderSourceStringLengths[0] = text.length();

	glShaderSource(shader, 1, shaderSourceStrings, shaderSourceStringLengths);
	glCompileShader(shader);

	CheckShaderError(shader, GL_COMPILE_STATUS, "Error in " + filename + ": Shader compilation failed: ");

	return shader;
}

static std::string LoadShader(const std::string& filename) {
	std::ifstream file;
	file.open((filename).c_str());

	std::string output;
	std::string line;

	if (file.is_open()) {
		while (file.good()) {
			getline(file, line);
			output.append(line + "\n");
		}
	}
	else {
		std::cerr << "Unable to load shader:" << filename << std::endl;
	}

	return output;
}

static void CheckShaderError(GLuint shader, GLuint flag, const std::string& errorMessage) {
	GLint success = 0;
	GLchar error[1024] = { 0 };

	glGetShaderiv(shader, flag, &success);

	if (success == GL_FALSE) {
		glGetShaderInfoLog(shader, sizeof(error), NULL, error);
		std::cerr << errorMessage << ": '" << error << "'" << std::endl;
	}
}