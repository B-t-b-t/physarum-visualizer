#include "shader_program.h"

#include <iostream>

static bool CheckProgramError(GLuint shader, GLuint flag, const std::string& errorMessage);

ShaderProgram::ShaderProgram(std::string programName, std::initializer_list<Shader*> shaders) : programName_(programName) {
    programID_ = glCreateProgram();

	if(!isProgramValid()) {
		std::cerr << "Error: Program creation failed!" << std::endl;
		return;
	}

	if(!attachShaders(shaders)) {
		detachShaders();
		glDeleteProgram(programID_);
		programID_ = 0;		//set programID to 0 to mark this ShaderProgram as invalid
		return;
	}
		
	if(!link()) {
		std::cerr << "Failed to link '" << programName_ << "' Shader Program." << std::endl;
		detachShaders();
		glDeleteProgram(programID_);
		programID_ = 0;		//set programID to 0 to mark this ShaderProgram as invalid
		return;
	}
	
	getUniformsFromGLSL();
	
	detachShaders();	//detach shaders for possible reuse of Shaders in other Programs
}

ShaderProgram::ShaderProgram(ShaderProgram&& rhs) noexcept
	: programName_{std::move(rhs.programName_)},
	  programID_{rhs.programID_},
	  shaderIDs_{std::move(rhs.shaderIDs_)},
	  uniforms_{std::move(rhs.uniforms_)}
{
	rhs.programID_ = 0;
	rhs.shaderIDs_.clear();
}

ShaderProgram& ShaderProgram::operator=(ShaderProgram&& rhs) noexcept {
	if(this == &rhs) {
		return *this;
	}

	if(programID_ != 0) {
		detachShaders();
		glDeleteProgram(programID_);
	}

	programName_ = std::move(rhs.programName_);
	programID_ = rhs.programID_;
	shaderIDs_ = std::move(rhs.shaderIDs_);
	uniforms_ = std::move(rhs.uniforms_);

	rhs.programID_ = 0;
	rhs.shaderIDs_.clear();

	return *this;
}

ShaderProgram::~ShaderProgram() {
	if(programID_ != 0) {
    	detachShaders();
    	glDeleteProgram(programID_);
	}
}

bool ShaderProgram::attachShaders(std::initializer_list<Shader*> shaders) {

	bool success = true;

	for (const Shader* shader : shaders) {
		if (shader->isShaderValid()) {
			glAttachShader(programID_, shader->getShaderID());
			shaderIDs_.push_back(shader->getShaderID());
		}
		else {
			std::cerr << "Error: Shader '" << shader->getFileName() << "' is not valid and can't be attached to program '" << programName_ << "'." << std::endl;
			success = false;
		}
	}

	return success;
}

void ShaderProgram::detachShaders() {
    if (programID_ == 0) {
        shaderIDs_.clear();
        return;
    }

    for(GLuint shaderID : shaderIDs_) {
        if (shaderID != 0) {
            glDetachShader(programID_, shaderID);
        }
    }
    shaderIDs_.clear();
}

bool ShaderProgram::link() {

    glLinkProgram(programID_);
    bool success = CheckProgramError(programID_, GL_LINK_STATUS, "Error in " + programName_ + ": Program linking failed!");

	if(success) {
		glValidateProgram(programID_);
		success = success && CheckProgramError(programID_, GL_VALIDATE_STATUS, "Error in " + programName_ + ": Program is invalid: ");
	}

	return success;
}

void ShaderProgram::getUniformsFromGLSL() {
	GLint numUniforms = 0;
	glGetProgramiv(programID_, GL_ACTIVE_UNIFORMS, &numUniforms);

	for (int i = 0; i < numUniforms; ++i) {
		GLint nameLength = 0;
		GLint size = 0;
		GLenum type = GL_ZERO;
		GLchar name[100];

		glGetActiveUniform(programID_, static_cast<unsigned int>(i), sizeof(name) - 1, &nameLength, &size, &type, name);
		name[nameLength] = 0;
		GLint location = glGetUniformLocation(programID_, name);

		if (uniforms_.find(name) == uniforms_.end()) {
        	uniforms_[name] = {name, type, location};
    	}
	}
}

void ShaderProgram::printUniforms() {
	for (const auto& uniform : uniforms_) {
		std::cout << "Name: " << uniform.second.uniformName << " Type: " << uniform.second.type << " Location: " << uniform.second.location << std::endl;
	}
}

void ShaderProgram::use() {
    glUseProgram(programID_);
}

void ShaderProgram::dispatchCompute(int numWorkGroupsX, int numWorkGroupsY, int numWorkGroupsZ) {
		glUseProgram(programID_);
		glDispatchCompute((unsigned int) numWorkGroupsX, (unsigned int) numWorkGroupsY, (unsigned int) numWorkGroupsZ);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		//glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	
}

void ShaderProgram::setUniform1i(const std::string& name, int value) {
    glUseProgram(programID_);
    glUniform1i(uniforms_[name].location, value);
}

void ShaderProgram::setUniform1f(const std::string& name, float value) {
    glUseProgram(programID_);
    glUniform1f(uniforms_[name].location, value);
}

void ShaderProgram::setUniform2f(const std::string& name, float v0, float v1) {
	glUseProgram(programID_);
	glUniform2f(uniforms_[name].location, v0, v1);
}

void ShaderProgram::setUniform4f(const std::string& name, const ImVec4& value) {
    glUseProgram(programID_);
    glUniform4f(uniforms_[name].location, value.x, value.y, value.z, value.w);
}

void ShaderProgram::setUniformTexture(const std::string& name, GLuint textureUnit, GLuint textureID) {
    glUseProgram(programID_);
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glUniform1i(uniforms_[name].location, static_cast<int>(textureUnit));
}

static bool CheckProgramError(GLuint shader, GLuint flag, const std::string& errorMessage) {
	GLint success = 0;
	GLchar error[1024] = { 0 };

	glGetProgramiv(shader, flag, &success);

	if (success == GL_FALSE) {
		glGetProgramInfoLog(shader, sizeof(error), NULL, error);
		std::cerr << errorMessage << ": '" << error << "'" << std::endl;
	}

    return success;
}

void ShaderProgram::printWorkGroupInfo() {
	int work_grp_cnt[3];

	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_cnt[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_cnt[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_cnt[2]);

	printf("max global (total) work group counts x:%i y:%i z:%i\n",
	work_grp_cnt[0], work_grp_cnt[1], work_grp_cnt[2]);


	int work_grp_size[3];

	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &work_grp_size[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &work_grp_size[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &work_grp_size[2]);

	printf("max local (in one shader) work group sizes x:%i y:%i z:%i\n",
	work_grp_size[0], work_grp_size[1], work_grp_size[2]);

	int max_work_grp_invocations;

	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &max_work_grp_invocations);

	printf("max number of invocations in a work group: %i\n",
	max_work_grp_invocations);

	int max_shared_memory;

	glGetIntegerv(GL_MAX_COMPUTE_SHARED_MEMORY_SIZE, &max_shared_memory);

	printf("max memory size of all shared variables in a compute shader [bytes]: %i\n", max_shared_memory);
}