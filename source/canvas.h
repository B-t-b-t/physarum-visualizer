#ifndef CANVAS_H
#define CANVAS_H

#include <vector>

#include <GL/glew.h>

class Canvas
{
private:
	std::vector<float> m_square;

	unsigned int m_VAO;
	unsigned int m_VBO;

public:

	Canvas();
	void draw();
};
#endif // CANVAS_H