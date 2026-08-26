#include "fileHandling.h"

#include <dirent.h>
#include <fstream>

#include <SDL3_image/SDL_image.h>

//stb_truetype needs to be compiled with less strict warnings as the rest of the project
#if defined(__GNUC__) || defined(__clang__) || defined(__MINGW32__) || defined(__MINGW64__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wfloat-equal"
#endif

#define STB_TRUETYPE_IMPLEMENTATION
#include "../../external/stb_truetype.h"

#if defined(__GNUC__) || defined(__clang__) || defined(__MINGW32__) || defined(__MINGW64__)
#pragma GCC diagnostic pop
#endif

/*
Loads every name of the files with the specified extension
*/
void getFileNamesInDirectory(std::string directoryPath, std::string fileExtension, std::vector<std::string>& fileNames) {
    DIR *dir;
    struct dirent *ent;
    
    if ((dir = opendir(directoryPath.c_str())) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            std::string fileName = ent->d_name;
            // Check if file has the specified extension
            size_t nameLength = fileName.length();
            size_t extensionLength = fileExtension.length();

            if (nameLength > extensionLength && 
                fileName.substr(nameLength - extensionLength) == fileExtension) {
                // Remove file extension to get file name
                fileNames.push_back(fileName.substr(0, nameLength - extensionLength));
            }
        }
        
        closedir(dir);
    }
}

SDL_Surface* loadImageFromFile(std::string filePath, std::string fileName, std::string fileExtension) {
    //only PNG images, because they support alpha channel
    if(fileExtension != ".png") {
        return nullptr;
    }
    std::string fullPath = filePath + fileName + fileExtension;

    SDL_Surface* surface = IMG_Load(fullPath.c_str());
    if(surface == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load image %s: %s", fileName.c_str(), SDL_GetError());
        return nullptr;
    }

    SDL_Surface* formattedSurface = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
    SDL_DestroySurface(surface);

    if(formattedSurface == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to convert surface format for image %s: %s", fileName.c_str(), SDL_GetError());
        return nullptr;
    }

    if(!SDL_FlipSurface(formattedSurface, SDL_FLIP_VERTICAL)) { //flip vertically for OpenGL coordinate system
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to flip to OpenGL orientation for image %s: %s", fileName.c_str(), SDL_GetError());
        SDL_DestroySurface(formattedSurface);
        return nullptr;
    }

    return formattedSurface;
}

SDL_Surface* loadImageFromFont(std::string filePath, std::string fileName, std::string fileExtension) {
    if(fileExtension != ".ttf") {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unsupported font format: %s", fileExtension.c_str());
        return nullptr;
    }

    std::string fullPath = filePath + fileName + fileExtension;
    std::ifstream inputFileStream(fullPath, std::ios::binary);

    //find file size
    inputFileStream.seekg(0, std::ios::end);
    auto&& fileSize = inputFileStream.tellg();
    inputFileStream.seekg(0, std::ios::beg);

    //allocate buffer for fileStream, manually delete later!
    uint8_t* fontDataBuf = new uint8_t[static_cast<int>(fileSize)];
    inputFileStream.read((char*)fontDataBuf, fileSize);

    int fontAtlasWidth = 1024;
    int fontAtlasHeight = 1024;

    uint8_t* fontAtlasBitmap = new uint8_t[fontAtlasWidth * fontAtlasHeight];

    const int firstChar = 32;    // start at ASCII 32(Space)
    const int numberOfChars = 95;   //95 characters in total, so ends at ASCII 126(~)
    float fontSize = 64.0f;    //font height in pixel

    stbtt_packedchar packedChars[numberOfChars];    //used for rendering single char via vertex quad
    stbtt_aligned_quad alignedQuads[numberOfChars]; //used for rendering single char via vertex quad

    stbtt_pack_context packContext;
    stbtt_PackBegin(&packContext, (unsigned char*)fontAtlasBitmap, fontAtlasWidth, fontAtlasHeight, 0, 1, nullptr);
    stbtt_PackFontRange(&packContext, fontDataBuf, 0, fontSize, firstChar, numberOfChars, packedChars);
    stbtt_PackEnd(&packContext);

    for (int i = 0; i < numberOfChars; i++) {
        float xPos, yPos;
        stbtt_GetPackedQuad(packedChars, fontAtlasWidth, fontAtlasHeight, i, &xPos, &yPos, &alignedQuads[i], 0);
    }

    std::vector<uint8_t> fontAtlasRGBA(static_cast<size_t>(fontAtlasWidth * fontAtlasHeight * 4));

    //stb_truetype creates an 8-bit font atlas, but the shaders need RGBA
    for(int y = 0; y < fontAtlasHeight; ++y) {
        for(int x = 0; x < fontAtlasWidth; ++x) {
            const size_t sourceIndex = static_cast<size_t>(y * fontAtlasWidth + x);
            const size_t destinationIndex = static_cast<size_t>((y * fontAtlasWidth + x) * 4);
            const uint8_t pixelValue = fontAtlasBitmap[sourceIndex];

            fontAtlasRGBA[destinationIndex + 0] = pixelValue;   //RED
            fontAtlasRGBA[destinationIndex + 1] = pixelValue;   //GREEN
            fontAtlasRGBA[destinationIndex + 2] = pixelValue;   //BLUE
            fontAtlasRGBA[destinationIndex + 3] = pixelValue;   //ALPHA
        }
    }

    SDL_Surface* fontAtlasSurface = SDL_CreateSurface(fontAtlasWidth, fontAtlasHeight, SDL_PIXELFORMAT_RGBA32);

    if(fontAtlasSurface == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ERROR: Failed to create SDL surface for font atlas: %s", SDL_GetError());
        return nullptr;
    }

    if(!SDL_LockSurface(fontAtlasSurface)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ERROR: Failed to lock font atlas surface: %s", SDL_GetError());
        SDL_DestroySurface(fontAtlasSurface);
        return nullptr;
    }
    //SDL_CreateSurfaceFrom doesn't copy and fontAtlasRGBA is local scope, so manual copying is necessary
    SDL_memmove(fontAtlasSurface->pixels, fontAtlasRGBA.data(), fontAtlasRGBA.size());

    SDL_UnlockSurface(fontAtlasSurface);

    // Match the orientation used by loadImageFromFile().
    if(!SDL_FlipSurface(fontAtlasSurface, SDL_FLIP_VERTICAL)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ERROR: Failed to flip to OpenGL orientation for image: %s", SDL_GetError());
        SDL_DestroySurface(fontAtlasSurface);
        return nullptr;
    }

    //cleanup of buffers, as they were manually allocated
    delete[] fontAtlasBitmap;
    delete[] fontDataBuf;

    return fontAtlasSurface;
}

bool saveImageToFile(SDL_Surface* surface, std::string filePath, std::string fileName, std::string fileExtension) {
    std::string fullPath = filePath + fileName + fileExtension;
    bool saveSuccess = false;
    bool wrongExtension = false;

    if (fileExtension == ".png") {
        saveSuccess = IMG_SavePNG(surface, fullPath.c_str());
    } else if (fileExtension == ".jpg" || fileExtension == ".jpeg") {
        saveSuccess = IMG_SaveJPG(surface, fullPath.c_str(), 100); // Quality set to 100
    } else {
        wrongExtension = true;
    }
    if(!saveSuccess) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to save image %s: %s", fileName.c_str(), SDL_GetError());
        if(wrongExtension) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unsupported image format: %s", fileExtension.c_str());
        }
    }

    return saveSuccess;
}