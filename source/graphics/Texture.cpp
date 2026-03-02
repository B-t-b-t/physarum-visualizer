#include "Texture.h"

Texture::Texture(int width, int height, TextureType textureType, GLuint textureUnit, bool generateMipmaps, bool useImageBinding) {
	width_ = width;
	height_ = height;
	textureFormat_ = resolveFormat(textureType);
	generateMipmaps_ = generateMipmaps;

	// Integer textures don't support GL_LINEAR, only GL_NEAREST
    bool isIntegerFormat = (textureType == TextureType::R_UINT);

	glGenTextures(1, &textureID_);
	glActiveTexture(GL_TEXTURE0 + textureUnit);
	glBindTexture(GL_TEXTURE_2D, textureID_);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	if (generateMipmaps_ && !isIntegerFormat) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	} else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, isIntegerFormat ? GL_NEAREST : GL_LINEAR);
	}

	glTexImage2D(GL_TEXTURE_2D, 0, textureFormat_.internalFormat, width_, height_, 0, textureFormat_.format, textureFormat_.type, NULL);

	if (generateMipmaps_ && !isIntegerFormat) {
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	
	if(useImageBinding) {
		glBindImageTexture(textureUnit, textureID_, 0, GL_FALSE, 0, GL_READ_WRITE, static_cast<GLenum>(textureFormat_.internalFormat));
	}

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
	glTexImage2D(GL_TEXTURE_2D, 0, textureFormat_.internalFormat, width_, height_, 0, textureFormat_.format, textureFormat_.type, NULL);
	if (generateMipmaps_) {
		glGenerateMipmap(GL_TEXTURE_2D);
	}
}

Texture::~Texture() {
	  if (textureID_ != 0) glDeleteTextures(1, &textureID_);
}

Texture::TextureFormats Texture::resolveFormat(TextureType textureType) {
	switch (textureType) {
		case TextureType::RGBA_FLOAT: return { GL_RGBA32F, GL_RGBA, GL_FLOAT };
		case TextureType::R_UINT: return { GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT };
		default: return { GL_RGBA32F, GL_RGBA, GL_FLOAT };	//return standard
	}
}
