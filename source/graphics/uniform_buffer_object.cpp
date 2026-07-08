#include "uniform_buffer_object.h"

UniformBufferObject::UniformBufferObject(UniformBufferObject&& rhs) {
    this->blockName_ = rhs.blockName_;
    this->uniformBufferObjectID_ = rhs.uniformBufferObjectID_;
    this->bindingPoint_ = rhs.bindingPoint_;

    rhs.blockName_ = "";
    rhs.uniformBufferObjectID_ = 0;
    rhs.bindingPoint_ = 0;
}

UniformBufferObject& UniformBufferObject::operator=(UniformBufferObject&& rhs) {
    this->blockName_ = rhs.blockName_;
    this->uniformBufferObjectID_ = rhs.uniformBufferObjectID_;
    this->bindingPoint_ = rhs.bindingPoint_;

    rhs.blockName_ = "";
    rhs.uniformBufferObjectID_ = 0;
    rhs.bindingPoint_ = 0;
    
    return *this;
}

UniformBufferObject::~UniformBufferObject() {
    glDeleteBuffers(1, &uniformBufferObjectID_);
}
