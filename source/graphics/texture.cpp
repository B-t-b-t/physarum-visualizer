#include "texture.h"

#include <cassert>

Texture::Texture(TextureProperties properties, const void* data, TextureDataFormat dataFormat, TextureDataType dataType, int bytesPerRow) {

	ensureValidProperties(&properties, (void*)data, &dataFormat, &dataType, bytesPerRow);
	properties_ = properties;

	glGenTextures(1, &textureID_);

	if(properties_.textureUnit) {
		glActiveTexture(GL_TEXTURE0 + *(properties_.textureUnit));
	}

	glBindTexture(GL_TEXTURE_2D, textureID_);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (GLint) properties_.wrapX);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (GLint) properties_.wrapY);

	if(properties_.minFilter) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLint) *(properties_.minFilter));
	}
	if(properties_.magFilter) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLint) *(properties_.magFilter));
	}

	//set alignment of bytes per row
	if(data) {
		//get largest possible alignment
		int byteAlignment = 1;
		if(bytesPerRow % 2 == 0) { byteAlignment = 2; }
		if(bytesPerRow % 4 == 0) { byteAlignment = 4; }
		if(bytesPerRow % 8 == 0) { byteAlignment = 8; }

		glPixelStorei(GL_UNPACK_ALIGNMENT, byteAlignment);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, properties.width);
	}

	glTexImage2D(GL_TEXTURE_2D, 0, (GLint) properties.texelFormat, properties_.width, properties_.height, 0, (GLenum) dataFormat, (GLenum) dataType, data);

	// restore alignment to default values
	if(data) {
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	}

	if (properties.generateMipmaps) {
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	
	if(properties.imageUnit) {
		glBindImageTexture(*(properties.imageUnit), textureID_, 0, GL_FALSE, 0, GL_READ_WRITE, (GLenum) properties.texelFormat);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::Texture(Texture&& other) noexcept 
	:	textureID_(other.textureID_),
		properties_(other.properties_)
{
	other.textureID_ = 0; // Prevent the moved-from object from deleting the texture
}

Texture& Texture::operator=(Texture&& other) noexcept {
	if (this != &other) {
		if (textureID_ != 0) {
			glDeleteTextures(1, &textureID_);
		}
		textureID_ = other.textureID_;
		properties_ = other.properties_;
		
		other.textureID_ = 0; // Prevent the moved-from object from deleting the texture
	}
	return *this;
}

void Texture::resizeTexture(int width, int height) {
	properties_.width = width;
	properties_.height = height;

	if(properties_.textureUnit) {
		glActiveTexture(GL_TEXTURE0 + *(properties_.textureUnit));
	}
	
	glBindTexture(GL_TEXTURE_2D, textureID_);
	glTexImage2D(GL_TEXTURE_2D, 0, (GLint) properties_.texelFormat, properties_.width, properties_.height, 0, GL_RGBA, GL_FLOAT, NULL);
	if (properties_.generateMipmaps) {
		glGenerateMipmap(GL_TEXTURE_2D);
	}
}

Texture::~Texture() {
	  if (textureID_ != 0) glDeleteTextures(1, &textureID_);
}

void Texture::ensureValidProperties(TextureProperties* properties, void* data, TextureDataFormat* dataFormat, TextureDataType* dataType, int bytesPerRow) {
	//===== ASSERT Section =====
	//just to make errors visible during development and prevent silent failures (like empty textures...)
	assert(properties->width > 0 && properties->height > 0);
	//if data is provided, dataFormat and dataType must be provided as well
	assert(!data || (data && *dataFormat != TextureDataFormat::NOT_PROVIDED && *dataType != TextureDataType::NOT_PROVIDED && bytesPerRow != -1));
	//just to make sure to catch wrong configs of min/mag filters
	assert(properties->generateMipmaps == true || (properties->generateMipmaps == false && properties->minFilter <= TextureMinFilter::LINEAR));
	assert(properties->texelFormat != TexelFormat::R_UINT || (properties->texelFormat == TexelFormat::R_UINT && (properties->minFilter == TextureMinFilter::NEAREST || !properties->minFilter)));
	assert(properties->texelFormat != TexelFormat::R_UINT || (properties->texelFormat == TexelFormat::R_UINT && (properties->magFilter == TextureMagFilter::NEAREST || !properties->magFilter)));

	//===== Quality of Life Section =====
	//reduces the number of parameters required for the constructor for certain texture configs

	//if no data provided, dataFormat and dataType must align with texelFormat, otherwise OpenGL throws an error and prevents the texture from being created
	//this also enables calling this constructor with only a TextureProperties argument for non-standard texel formats, e.g. R_UINT
	if(!data) {
		switch (properties->texelFormat) {
			case TexelFormat::RGBA32F:
				*dataFormat = TextureDataFormat::RGBA;
				*dataType = TextureDataType::FLOAT;
				break;
			case TexelFormat::R_UINT:
				*dataFormat = TextureDataFormat::R;
				*dataType = TextureDataType::UINT;
				break;
		}
	}

	//ensure that R_UINT textures are always created with NEAREST filtering, otherwise OpenGL throws an error and prevents the texture from being created
	//prevents having to unnecessarily set min/mag filters for R_UINT textures, when it's the only valid option anyway
	if(properties->texelFormat == TexelFormat::R_UINT ) {
		properties->minFilter = TextureMinFilter::NEAREST;
		properties->magFilter = TextureMagFilter::NEAREST;
	}
}