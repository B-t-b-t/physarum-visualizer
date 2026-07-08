#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H

#include <initializer_list>
#include <string>
#include <unordered_map>
#include <vector>

#include <GL/glew.h>

#include "shader.h"
#include "uniform_buffer_object.h"
#include "../uniforms.h"
#include "../ui/user_interface.h"

struct UniformMapping {
    std::string uniformName;
    GLenum type;
    GLint location;
};

class ShaderProgram {
public:
    ShaderProgram() = default;
    ShaderProgram(std::string programName, std::initializer_list<Shader*> shaders);
    ShaderProgram(ShaderProgram& rhs) = delete;   //no copies, to prevent multiple destructor calls on same GL programID
    ShaderProgram& operator=(ShaderProgram& rhs) = delete;
    ShaderProgram(ShaderProgram&& rhs) noexcept;
    ShaderProgram& operator=(ShaderProgram&& rhs) noexcept;

    ~ShaderProgram();

    void use();
    void dispatchCompute(int numWorkGroupsX, int numWorkGroupsY, int numWorkGroupsZ);
    
    bool isProgramValid() const { return programID_ != 0; }

    void printUniforms();
    void printWorkGroupInfo();

    GLuint getProgramID() const { return programID_; }
    
    // Uniform setters
    void setUniform1i(const std::string& name, int value);
    void setUniform1f(const std::string& name, float value);
    void setUniform2f(const std::string& name, float v0, float v1);
    void setUniform4f(const std::string& name, const ImVec4& value);
	void setUniformTexture(const std::string& name, GLuint textureUnit, GLuint textureID);
    
    private:
    bool attachShaders(std::initializer_list<Shader*> shaders);
    void detachShaders();
    bool link();
    void getUniformsFromGLSL();

    std::string programName_{""};
    GLuint programID_{0};
    std::vector<GLuint> shaderIDs_;

    std::unordered_map<std::string, UniformMapping> uniforms_;
};

#endif // SHADER_PROGRAM_H