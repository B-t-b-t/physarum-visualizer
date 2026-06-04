#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <unordered_map>

#include <GL/glew.h>

#include "../ui/user_interface.h"

enum class ShaderType {
    VERTEX_SHADER,
    FRAGMENT_SHADER,
    COMPUTE_SHADER
};

class Shader {

public:
    Shader();
	Shader(const std::string& filename, ShaderType shaderType);
    Shader(Shader&& rhs);
    Shader& operator=(Shader&& rhs);

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
