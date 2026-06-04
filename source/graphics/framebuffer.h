#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <GL/glew.h>

class FrameBuffer {
public:
    FrameBuffer();

    FrameBuffer(FrameBuffer&& rhs);
    FrameBuffer& operator=(FrameBuffer&& rhs);

    ~FrameBuffer();

    void bind();
    void unbind();

    void attachTexture(GLenum attachment, GLenum textarget, GLuint textureID, GLint level);

    GLuint getID() const { return framebufferID_; }

    bool checkFramebufferStatus();

private:
    GLuint framebufferID_;
};

#endif // FRAMEBUFFER_H