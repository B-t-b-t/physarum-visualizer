#include "uniform_buffer_manager.h"

UniformBufferManager::UniformBufferManager(ApplicationState* uiState) {
    uboMap_.try_emplace("UniversalShaderSettings", std::make_unique<UniformBufferObject>("UniversalShaderSettings", uiState->universalShaderSettings, 0));
    uboMap_.try_emplace("SlimeSettings", std::make_unique<UniformBufferObject>("SlimeSettings", uiState->slimeSettings, 1));
    uboMap_.try_emplace("TrailDiffusionSettings", std::make_unique<UniformBufferObject>("TrailDiffusionSettings", uiState->trailDiffusionSettings, 2));
    uboMap_.try_emplace("FragmentShaderSettings", std::make_unique<UniformBufferObject>("FragmentShaderSettings", uiState->fragmentShaderSettings, 3));
    uboMap_.try_emplace("ParameterSettings", std::make_unique<UniformBufferObject>("ParameterSettings", uiState->parameterSettings, 4));
}

void UniformBufferManager::attachUBOs(std::initializer_list<GLuint> shaderProgramIDs) {

    for (GLuint programID : shaderProgramIDs) {
        for(const auto& [blockName, ubo] : uboMap_) {
            GLuint blockIndex = glGetUniformBlockIndex(programID, ubo->getBlockName().c_str());
            if (blockIndex == GL_INVALID_INDEX) {
                continue;
            }
            glUniformBlockBinding(programID, blockIndex, ubo->getBindingPoint());
        }
    }
}

void UniformBufferManager::updateUBOs(ApplicationState* uiState) {
    uboMap_["UniversalShaderSettings"]->updateUniformBufferObject(uiState->universalShaderSettings);
    uboMap_["SlimeSettings"]->updateUniformBufferObject(uiState->slimeSettings);
    uboMap_["TrailDiffusionSettings"]->updateUniformBufferObject(uiState->trailDiffusionSettings);
    uboMap_["FragmentShaderSettings"]->updateUniformBufferObject(uiState->fragmentShaderSettings);
    uboMap_["ParameterSettings"]->updateUniformBufferObject(uiState->parameterSettings);
}