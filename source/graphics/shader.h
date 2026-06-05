#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <unordered_map>

#include <GL/glew.h>

#include "../ui/user_interface.h"

enum class ShaderType {
    NONE,
    VERTEX_SHADER,
    FRAGMENT_SHADER,
    COMPUTE_SHADER
};

class Shader {

public:

    Shader() = default;

    /**
     * @brief Creates and compiles a new Shader object from a text file with GLSL code.
     * 
     * If the file can't be read or the GLSL code isn't valid, this shader object isn't usable 
     * and not registered with OpenGL.
     * Check with isShaderValid() before using the shader to prevent errors.
     * 
     * @param fileName The name of the shader file
     * @param shaderType The type of the shader
     */
	Shader(const std::string& fileName, ShaderType shaderType);

    Shader(const Shader& other) = delete;   //no copies, to prevent multiple destructor calls on same GL shaderID
    void operator=(const Shader& other) = delete;
    Shader(Shader&& rhs);   //move constructor
    Shader& operator=(Shader&& rhs);    //move assignment operator

    /**
     * @brief Deletes the Shader object with a call to glDeleteShader
     * 
     */
    ~Shader();

    const std::string& getFileName() const { return fileName_; }
    GLuint getShaderID() const { return shaderID_; }
    ShaderType getShaderType() const { return shaderType_; }

    /**
     * @brief Check if the shader is valid (i.e., successfully created and compiled)
     * 
     * @return true if the shader is valid, false otherwise
     */
    bool isShaderValid() const { return shaderID_ != 0; }

private:

    std::string fileName_{""};
    GLuint shaderID_{0};    //! 0 means invalid shader that can't be used with OpenGL
    ShaderType shaderType_{ShaderType::NONE};

};

#endif // SHADER_H
