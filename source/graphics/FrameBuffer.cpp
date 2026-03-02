#include "FrameBuffer.h"
#include <iostream>

FrameBuffer::FrameBuffer() {
    glGenFramebuffers(1, &framebufferID_);
}

FrameBuffer::~FrameBuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &framebufferID_);
}

void FrameBuffer::bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferID_);
    FrameBuffer::checkFramebufferStatus();
}

void FrameBuffer::unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    FrameBuffer::checkFramebufferStatus();
}

void FrameBuffer::attachTexture(GLenum attachment, GLenum textarget, GLuint textureID, GLint level) {
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferID_);      //use GL function instead of class method to prevent incomplete framebuffer error from checkFramebufferStatus
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, textarget, textureID, level);
    FrameBuffer::checkFramebufferStatus();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool FrameBuffer::checkFramebufferStatus() {
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer incomplete! " << "ID: " << framebufferID_ << " Status: " << status << std::endl;
        return false;
    }
    return true;
}
