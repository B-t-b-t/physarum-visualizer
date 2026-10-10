#include "graphics_debug_tools.h"

#include <cstdio>
#include <GL/glew.h>

GraphicsDebugTimer::GraphicsDebugTimer(const char* name) : name_(name) {
    glGenQueries(1, &query_);
    glBeginQuery(GL_TIME_ELAPSED, query_);
}

GraphicsDebugTimer::~GraphicsDebugTimer() {
    glEndQuery(GL_TIME_ELAPSED);
    GLint done = GL_FALSE;
    while (!done) {
        glGetQueryObjectiv(query_, GL_QUERY_RESULT_AVAILABLE, &done);
    }

    glGetQueryObjectui64v(query_, GL_QUERY_RESULT, &elapsedTime_);
    printf("%s Time: %f ms\n", name_, elapsedTime_ / 1000000.0);
    glDeleteQueries(1, &query_);
}

