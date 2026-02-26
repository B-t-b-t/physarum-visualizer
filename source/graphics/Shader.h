#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <GL/glew.h>
#include <unordered_map>
//#include "imgui/imgui.h"
#include "../ui/UserInterface.h"

enum class ShaderType {
    VERTEX_SHADER,
    FRAGMENT_SHADER,
    COMPUTE_SHADER
};

class Shader {

public:
	Shader(const std::string& filename, ShaderType shaderType);
    virtual ~Shader();

    GLuint getShaderID() const { return shaderID_; }

private:

    std::string filename_;

    GLuint shaderID_;
    ShaderType shaderType_;

    //Shader(const Shader& other) {}
	//void operator=(const Shader& other) {}
};

#endif // SHADER_H
