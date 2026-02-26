#include "canvas.h"
#include <iostream>
#include <GL/glew.h>
#include <complex>

Canvas::Canvas()
{
	m_square = { -1.0f, -1.0f, 0.0f, 0.0f,
				  1.0f, -1.0f, 1.0f, 0.0f,
				  1.0f,  1.0f, 1.0f, 1.0f,
				 -1.0f, -1.0f, 0.0f, 0.0f,
				  1.0f,  1.0f, 1.0f, 1.0f,
				 -1.0f,  1.0f, 0.0f, 1.0f };

	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);
	glGenBuffers(1, &m_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(m_square.size() * sizeof(float)), &m_square.front(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
}

void Canvas::draw() {
	glBindVertexArray(m_VAO);

	glDrawArrays(GL_TRIANGLES, 0, m_square.size());
}