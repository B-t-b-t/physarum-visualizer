#pragma once
#include <GL/glew.h>



class Texture
{
public:
	Texture(int width, int height, GLuint textureUnit, bool generateMipmaps = true);
	Texture(const Texture&) = delete; // Prevent copying because of OpenGL resource management
	Texture& operator=(Texture&& other) noexcept;
	Texture(Texture&& other) noexcept;
	~Texture();

	GLuint getID() { return textureID_; }
	GLuint getTextureUnit() { return textureUnit_; }
	int getWidth() { return width_; }
	int getHeight() { return height_; }
	
	void setTextureUnit(GLuint textureUnit) { textureUnit_ = textureUnit; }
	void resizeTexture(int width, int height);
	void generateMipMap() { glBindTexture(GL_TEXTURE_2D, textureID_); glGenerateMipmap(GL_TEXTURE_2D); generateMipmaps_ = true; }

private:
	GLuint textureID_;
	GLuint textureUnit_;

	int width_;
	int height_;
	bool generateMipmaps_;
};

