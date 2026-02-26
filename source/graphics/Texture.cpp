#include "Texture.h"

Texture::Texture(int width, int height, GLuint textureUnit, bool generateMipmaps) {
	width_ = width;
	height_ = height;
	generateMipmaps_ = generateMipmaps;

	glGenTextures(1, &textureID_);
	glActiveTexture(GL_TEXTURE0 + textureUnit);
	glBindTexture(GL_TEXTURE_2D, textureID_);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	if (generateMipmaps_) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	} else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width_, height_, 0, GL_RGBA, GL_FLOAT, NULL);

	if (generateMipmaps_) {
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	
	glBindImageTexture(textureUnit, textureID_, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	textureUnit_ = textureUnit;
}

Texture::Texture(Texture&& other) noexcept 
	: textureID_(other.textureID_), textureUnit_(other.textureUnit_), width_(other.width_), height_(other.height_), generateMipmaps_(other.generateMipmaps_) {
	other.textureID_ = 0; // Prevent the moved-from object from deleting the texture
	other.generateMipmaps_ = false;
}

Texture& Texture::operator=(Texture&& other) noexcept {
	if (this != &other) {
		if (textureID_ != 0) {
			glDeleteTextures(1, &textureID_);
		}
		textureID_ = other.textureID_;
		textureUnit_ = other.textureUnit_;
		width_ = other.width_;
		height_ = other.height_;
		generateMipmaps_ = other.generateMipmaps_;
		other.textureID_ = 0; // Prevent the moved-from object from deleting the texture
		other.generateMipmaps_ = false;
	}
	return *this;
}

void Texture::resizeTexture(int width, int height) {
	width_ = width;
	height_ = height;

	glActiveTexture(GL_TEXTURE0 + textureUnit_);
	glBindTexture(GL_TEXTURE_2D, textureID_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width_, height_, 0, GL_RGBA, GL_FLOAT, NULL);
	if (generateMipmaps_) {
		glGenerateMipmap(GL_TEXTURE_2D);
	}
}

Texture::~Texture() {
	  if (textureID_ != 0) glDeleteTextures(1, &textureID_);
}
