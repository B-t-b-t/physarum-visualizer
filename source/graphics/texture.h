#ifndef TEXTURE_H
#define TEXTURE_H

#include <GL/glew.h>

class Texture {
public:
	enum class TextureType {
		RGBA_FLOAT,
		R_UINT
	};

	Texture() = default;
	Texture(int width, int height, TextureType textureType, GLuint textureUnit, bool generateMipmaps = true, bool useImageBinding = true);
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
	struct TextureFormats {
		GLint internalFormat;
		GLenum format;
		GLenum type;
	};

	static TextureFormats resolveFormat(TextureType textureType);

	GLuint textureID_{0};
	GLuint textureUnit_{0};
	TextureFormats textureFormat_;

	int width_{0};
	int height_{0};
	bool generateMipmaps_{false};
};

#endif // TEXTURE_H
