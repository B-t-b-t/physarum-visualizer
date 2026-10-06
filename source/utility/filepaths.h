#ifndef FILEPATHS_H
#define FILEPATHS_H

#include <filesystem>

struct FilePaths {
    const std::filesystem::path shaderDir = "./res/";
    const std::filesystem::path vertexShaderExtension = ".vs";
    const std::filesystem::path fragmentShaderExtension = ".fs";
    const std::filesystem::path computeShaderExtension = ".cs";
    const std::filesystem::path trailDiffusionFile = "TrailDiffusion.cs";
    const std::filesystem::path particleBehaviourFile = "ParticleBehaviour.cs";
    const std::filesystem::path vertexShaderFile = "vertex.vs";
    const std::filesystem::path fragmentShaderFile = "fragment.fs";
    const std::filesystem::path textVertexShaderFile = "text_vertex.vs";
    const std::filesystem::path textFragmentShaderFile = "text_fragment.fs";

    const std::filesystem::path sceneFilePath = "./presets/scenes.toml";

    const std::filesystem::path behaviorPresetFilePath = "./presets/behaviourPresets.toml";
    const std::filesystem::path colorPresetFilePath = "./presets/colorPresets.toml";

    const std::filesystem::path timeTableFilePath = "./res/pictures/timeTable.toml";

    const std::filesystem::path pictureDir = "./res/pictures/";
    const std::filesystem::path pictureFileExtension = ".png";

    const std::filesystem::path fontFileDir = "./res/fonts/";
    const std::filesystem::path fontFile = "Roboto-Medium.ttf";
    const std::filesystem::path fontAwesomeFile = "Font_Awesome_Solid.otf";
    const std::filesystem::path fontFileExtension = ".ttf";
    const std::filesystem::path fontAtlasTestOutputImage = "fontAtlas.png";
};

#endif // FILEPATHS_H