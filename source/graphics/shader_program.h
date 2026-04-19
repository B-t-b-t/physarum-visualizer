#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H

#include <string>
#include <unordered_map>
#include <vector>

#include <GL/glew.h>

#include "../uniforms.h"
#include "../ui/user_interface.h"

struct UniformMapping {
    std::string uniformName;
    GLenum type;
    GLint location;
};

class ShaderProgram {
public:
    ShaderProgram(std::string programName);
    ~ShaderProgram();

    void attachShader(GLuint shaderID);
    void link();
    void use();
    void dispatchCompute(int numWorkGroupsX, int numWorkGroupsY, int numWorkGroupsZ);

    void attachUniformBufferObject(GLuint uniformBufferObjectID, const std::string& blockName, GLuint bindingPoint);

    void getUniformsFromGLSL();
    void printUniforms();
    void printWorkGroupInfo();

    // Uniform setters
    void setUniform1i(const std::string& name, int value);
    void setUniform1f(const std::string& name, float value);
    void setUniform2f(const std::string& name, float v0, float v1);
    void setUniform4f(const std::string& name, const ImVec4& value);
	void setUniformTexture(const std::string& name, GLuint textureUnit, GLuint textureID);

private:
    std::string programName_;
    GLuint programID_;
    std::vector<GLuint> shaderIDs_;

    GLuint uniformBufferObjectID_;      //don't share UBOs between different ShaderPrograms; each ShaderProgram has its own UBO if needed!
    std::unordered_map<std::string, UniformMapping> uniforms_;
    std::unordered_map<std::string, GLuint> uboMap_;
};

#endif // SHADER_PROGRAM_H