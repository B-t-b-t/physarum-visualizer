#include "uniform_buffer_object.h"

UniformBufferObject::UniformBufferObject(GLuint bindingPoint) : uniformBufferObjectID_(0), bindingPoint_(bindingPoint) {
}

UniformBufferObject::~UniformBufferObject() {
    glDeleteBuffers(1, &uniformBufferObjectID_);
}
