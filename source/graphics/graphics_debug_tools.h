#ifndef GRAPHICS_DEBUG_TOOLS_H
#define GRAPHICS_DEBUG_TOOLS_H

#include <GL/glew.h>

/** @short Measures the time taken by a block of OpenGL code using GPU timer queries.
 * 
 *  Timer begins with object construction and ends with object destruction.
 *  Prints the elapsed time in milliseconds to the standard output.
 *  
 *  Usage: Wrap the code to measure in curly braces and initialize thhis timer object at the beginning of the block.
 */
class GraphicsDebugTimer {
public:
    GraphicsDebugTimer(const char* name);
    ~GraphicsDebugTimer();
private:
    const char* name_;
    GLuint query_;
    GLuint64 elapsedTime_;
};

#endif // GRAPHICS_DEBUG_TOOLS_H