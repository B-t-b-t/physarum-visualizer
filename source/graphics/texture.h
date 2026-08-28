#ifndef TEXTURE_H
#define TEXTURE_H

#include <optional>

#include <GL/glew.h>

// How the texture should be sampled for out-of-bounds pixels
enum class TextureWrap {
	CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE,				//extends the edge pixel to infinity (OPENGL default)
	CLAMP_TO_BORDER = GL_CLAMP_TO_BORDER, 			//extends a border color to infinity (default border color is black)
	REPEAT = GL_REPEAT, 							//repeats the texture to infinity
	MIRRORED_REPEAT = GL_MIRRORED_REPEAT, 			//repeats the texture to infinity in alternating mirrored orientations
	MIRROR_CLAMP_TO_EDGE = GL_MIRROR_CLAMP_TO_EDGE	//repeats the texture once in mirrored orientation and then clamps to the edge pixel
};

// How the texture should be sampled if it gets too small on screen (minification)
enum class TextureMinFilter {
	NEAREST = GL_NEAREST,								//nearest single pixel to sampled coordinate
	LINEAR = GL_LINEAR,									//weighed average of four nearest pixels to sampled coordinate
	NEAREST_MIPMAP_NEAREST = GL_NEAREST_MIPMAP_NEAREST, //chooses single closest mipmap, and samples mipmap with NEAREST filter (needs generated mipmaps!)
	LINEAR_MIPMAP_NEAREST = GL_LINEAR_MIPMAP_NEAREST,  	//chooses single closest mipmap, and samples mipmap with LINEAR filter (needs generated mipmaps!)
	NEAREST_MIPMAP_LINEAR = GL_NEAREST_MIPMAP_LINEAR, 	//weighed average of two closest mipmaps, and samples mipmaps with NEAREST filter (OPENGL default) (needs generated mipmaps!)
	LINEAR_MIPMAP_LINEAR = GL_LINEAR_MIPMAP_LINEAR    	//weighed average of two closest mipmaps, and samples mipmaps with LINEAR filter (needs generated mipmaps!)
};

// How the texture should be sampled if it gets too big on screen (magnification)
enum class TextureMagFilter {
	NEAREST = GL_NEAREST,			//nearest single pixel to sampled coordinate
	LINEAR = GL_LINEAR				//weighed average of four nearest pixels to sampled coordinate (OPENGL default)
};

// Internal data format used by the texture for it's texels
enum class TexelFormat {
	RGBA32F = GL_RGBA32F,	//32-bit floating point RGBA
	R_UINT = GL_R32UI		//32-bit unsigned integer R
};

// In which texel layout should data be interpreted, when transfering to GPU memory
enum class TextureDataFormat {
	RGBA = GL_RGBA,	//RGBA data format
	R = GL_RED_INTEGER,		//R data format
	NOT_PROVIDED = 0	//to prevent calling the constructor with data but no dataFormat
};

// In which data type should the color channels of the data be interpreted, when transfering to GPU memory
enum class TextureDataType {
	FLOAT = GL_FLOAT,
	UINT = GL_UNSIGNED_INT,
	UBYTE = GL_UNSIGNED_BYTE,
	NOT_PROVIDED = 0	//to prevent calling the constructor with data but no dataType
};

struct TextureProperties {
	int width{0};
	int height{0};
	TexelFormat texelFormat{TexelFormat::RGBA32F};

	//std::optional because not all textures need a fixed texture or image unit
	std::optional<GLuint> textureUnit{};
	std::optional<GLuint> imageUnit{}; // whether to bind the texture as an image for read/write-access like in a compute shader

	TextureWrap wrapX{TextureWrap::CLAMP_TO_EDGE};	//WRAP_S
	TextureWrap wrapY{TextureWrap::CLAMP_TO_EDGE};	//WRAP_T
	float borderColor[4]{0.0f, 0.0f, 0.0f, 1.0f};

	bool generateMipmaps{true};
	//std::optional because not all texel types support min/mag filtering, e.g. R_UINT
	//when not set, OPENGL defaults these values to (min: NEAREST_MIPMAP_LINEAR and mag: LINEAR)
	std::optional<TextureMinFilter> minFilter{};
	std::optional<TextureMagFilter> magFilter{};
};

class Texture {
public:

	Texture() = default;
	Texture(TextureProperties properties, const void* data = nullptr, TextureDataFormat dataFormat = TextureDataFormat::NOT_PROVIDED, TextureDataType dataType = TextureDataType::NOT_PROVIDED, int bytesPerRow = -1);
	Texture(const Texture&) = delete; // Prevent copying because of OpenGL resource management
	Texture& operator=(Texture&& other) noexcept;
	Texture(Texture&& other) noexcept;
	~Texture();

	GLuint getID() { return textureID_; }
	GLuint getTextureUnit() { return properties_.textureUnit.value_or(0); }
	int getWidth() { return properties_.width; }
	int getHeight() { return properties_.height; }
	
	void setTextureUnit(GLuint textureUnit) { properties_.textureUnit = textureUnit; }
	void resizeTexture(int width, int height);	//Don't use this function for textures that have been initialized with external image data, it will delete the data!

private:

	void ensureValidProperties(TextureProperties* properties, void* data, TextureDataFormat* dataFormat, TextureDataType* dataType, int bytesPerRow);

	GLuint textureID_{0};
	TextureProperties properties_;
};

#endif // TEXTURE_H
