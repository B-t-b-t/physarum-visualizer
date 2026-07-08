#ifndef UNIFORM_BUFFER_OBJECT_H
#define UNIFORM_BUFFER_OBJECT_H

#include <iostream>
#include <string>

#include <GL/glew.h>

class UniformBufferObject {

public:

    UniformBufferObject() = default;

    template<typename T>
    UniformBufferObject(std::string blockName,const T& data, GLuint bindingPoint)
     : blockName_(blockName), bindingPoint_(bindingPoint) {
        bindUniformBufferObject(data);
    }

    UniformBufferObject(UniformBufferObject&& rhs);
    UniformBufferObject& operator=(UniformBufferObject&& rhs);

    ~UniformBufferObject();

    //implementation needs to be in the header file to avoid linker errors because of template
    template<typename T>
    void bindUniformBufferObject(const T& data) {
        if (uniformBufferObjectID_ == 0) {      //first time initialization
            glGenBuffers(1, &uniformBufferObjectID_);
            glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint_, uniformBufferObjectID_);  //bind the UBO to the global binding point

            glBindBuffer(GL_UNIFORM_BUFFER, uniformBufferObjectID_);
            glBufferData(GL_UNIFORM_BUFFER, sizeof(T), &data, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
        } else {
            updateUniformBufferObject(data);    //in case method is called again, just update the data
        }
    }

    template<typename T>
    void updateUniformBufferObject(const T& data) {
        glBindBuffer(GL_UNIFORM_BUFFER, uniformBufferObjectID_);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(T), &data);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    std::string getBlockName() const { return blockName_; }
    GLuint getUniformBufferObjectID() const { return uniformBufferObjectID_; }
    GLuint getBindingPoint() const { return bindingPoint_; }
    
private:

std::string blockName_{""};
GLuint uniformBufferObjectID_{0};
GLuint bindingPoint_{0};
};
#endif // UNIFORM_BUFFER_OBJECT_H