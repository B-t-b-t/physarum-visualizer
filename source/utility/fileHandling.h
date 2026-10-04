#ifndef FILE_HANDLING_H
#define FILE_HANDLING_H

#include <filesystem>
#include <string>
#include <vector>

#include <SDL3/SDL_surface.h>

typedef struct {
   char character; //the character this info belongs to
   unsigned short x0,y0,x1,y1; // coordinates of bbox in bitmap
   unsigned short sizeX, sizeY; //size of the character in pixel
   float xoff,yoff,xadvance;
   float xoff2,yoff2;
   float s0, s1, t0, t1;
} FontCharInfo;

bool getFileNamesInDirectory(std::filesystem::path directoryPath, std::filesystem::path fileExtension, std::vector<std::string>& fileNames);

SDL_Surface* loadImageFromFile(std::filesystem::path filePath);
SDL_Surface* loadImageFromFont(std::filesystem::path filePath, std::vector<FontCharInfo>& fontInfos, int* firstChar_Out, int* numberOfChars_Out, float* fontSize_Out);

bool saveImageToFile(SDL_Surface* surface, std::filesystem::path filePath, bool isFlipped);

#endif // FILE_HANDLING_H