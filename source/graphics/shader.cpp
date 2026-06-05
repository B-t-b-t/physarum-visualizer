#include "shader.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include <SDL3/SDL.h>

static GLuint createShader(const std::string& text, GLenum shaderType, const std::string& fileName);
static std::string readTextFromFile(const std::string& fileName);
static bool checkShaderError(GLuint shader, GLuint flag, const std::string& errorMessage);

Shader::Shader(const std::string& fileName, ShaderType shaderType) 
: fileName_(fileName), shaderType_(shaderType)
{
	int glMajorVersion, glMinorVersion;
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &glMajorVersion);
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &glMinorVersion);

	std::string shaderString = "#version " + std::to_string(glMajorVersion) + std::to_string(glMinorVersion) + "0\n";

	if(glMajorVersion == 4 && glMinorVersion == 2 && shaderType_ == ShaderType::COMPUTE_SHADER) {
		shaderString.append("#extension GL_ARB_compute_shader : enable\n");
		shaderString.append("#extension GL_ARB_shader_storage_buffer_object : enable\n");
	}

	shaderString.append(readTextFromFile(fileName_));

	switch (shaderType_)
	{
	case ShaderType::VERTEX_SHADER:
		shaderID_ = createShader(shaderString, GL_VERTEX_SHADER, fileName_);
		break;
	case ShaderType::FRAGMENT_SHADER:
		shaderID_ = createShader(shaderString, GL_FRAGMENT_SHADER, fileName_);
		break;
	case ShaderType::COMPUTE_SHADER:
		shaderID_ = createShader(shaderString, GL_COMPUTE_SHADER, fileName_);
		break;
	default:
		break;
	}

}

Shader::Shader(Shader&& rhs) {
	this->fileName_ = rhs.fileName_;
	this->shaderID_ = rhs.shaderID_;
	this->shaderType_ = rhs.shaderType_;
	rhs.fileName_ = "";
	rhs.shaderID_ = 0;
}

Shader& Shader::operator=(Shader&& rhs) {
	this->fileName_ = rhs.fileName_;
	this->shaderID_ = rhs.shaderID_;
	this->shaderType_ = rhs.shaderType_;
	rhs.fileName_ = "";
	rhs.shaderID_ = 0;
	return *this;
}

Shader::~Shader() {
	glDeleteShader(shaderID_);
}

/**
 * @brief Creates and compiles a Shader object from GLSL code.
 * 
 * @param text The GLSL code for the shader
 * @param shaderType The type of the shader (e.g., GL_VERTEX_SHADER)
 * @param fileName The name of the shader file (just for error messages)
 * @return The OpenGL shader ID (0 if creation failed)
 */
static GLuint createShader(const std::string& text, GLenum shaderType, const std::string& fileName) {
	GLuint shader = glCreateShader(shaderType);

	if (shader == 0) {
		std::cerr << "Error: Shader creation failed!" << std::endl;
		return 0;
    }

	const GLchar* shaderSourceStrings[1];
	GLint shaderSourceStringLengths[1];
	shaderSourceStrings[0] = text.c_str();
	shaderSourceStringLengths[0] = text.length();

	glShaderSource(shader, 1, shaderSourceStrings, shaderSourceStringLengths);
	glCompileShader(shader);

	//check for compilation errors
	if (!checkShaderError(shader, GL_COMPILE_STATUS, "Error in " + fileName + ": Shader compilation failed: ")) {
		glDeleteShader(shader);
		return 0;
	}

	return shader;
}

/**
 * @brief Read the contents of a text file into a string
 * 
 * @param fileName The name of the file to read
 * @return std::string The contents of the file
 */
static std::string readTextFromFile(const std::string& fileName) {
	std::ifstream file;
	file.open((fileName).c_str());

	std::string output;
	std::string line;

	if (file.is_open()) {
		while (file.good()) {
			getline(file, line);
			output.append(line + "\n");
		}
		//file closed automatically by destructor of ifstream
	}
	else {
		std::cerr << "Unable to read from file: " << fileName << std::endl;
	}

	return output;
}

/**
 * @brief Check for shader compilation errors
 * 
 * @param shader The shader to check
 * @param flag The flag to check (e.g., GL_COMPILE_STATUS)
 * @param errorMessage The error message to display if the check fails
 * @return true if no errors, false otherwise
 */
static bool checkShaderError(GLuint shader, GLuint flag, const std::string& errorMessage) {
	GLint success = 0;
	GLchar error[1024] = { 0 };

	glGetShaderiv(shader, flag, &success);

	if (success == GL_FALSE) {
		glGetShaderInfoLog(shader, sizeof(error), NULL, error);
		std::cerr << errorMessage << ": '" << error << "'" << std::endl;
	}

	return (bool)success;	//return true if no error (success >= 1)
}