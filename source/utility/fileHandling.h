#ifndef FILE_HANDLING_H
#define FILE_HANDLING_H

#include <string>
#include <vector>

#include <SDL3/SDL.h>

void getFileNamesInDirectory(std::string directoryPath, std::string fileExtension, std::vector<std::string>& fileNames);

SDL_Surface* loadImageFromFile(std::string filePath, std::string fileName, std::string fileExtension);
SDL_Surface* loadImageFromFont(std::string filePath, std::string fileName, std::string fileExtension);

bool saveImageToFile(SDL_Surface* surface, std::string filePath, std::string fileName, std::string fileExtension);

#endif // FILE_HANDLING_H