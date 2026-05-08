#include "fileHandling.h"

#include <dirent.h>
/*
Loads every name of the files with the specified extension
*/
void loadFileNames(std::string directoryPath, std::string fileExtension, std::vector<std::string>& fileNames) {
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