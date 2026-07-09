#include "uniform_buffer_manager.h"

UniformBufferManager::UniformBufferManager(ApplicationState* appState)
 : appState_(appState) {
    uboMap_.try_emplace("UniversalShaderSettings", std::make_unique<UniformBufferObject>("UniversalShaderSettings", appState_->universalShaderSettings, 0));
    uboMap_.try_emplace("SlimeSettings", std::make_unique<UniformBufferObject>("SlimeSettings", appState_->slimeSettings, 1));
    uboMap_.try_emplace("TrailDiffusionSettings", std::make_unique<UniformBufferObject>("TrailDiffusionSettings", appState_->trailDiffusionSettings, 2));
    uboMap_.try_emplace("FragmentShaderSettings", std::make_unique<UniformBufferObject>("FragmentShaderSettings", appState_->fragmentShaderSettings, 3));
    uboMap_.try_emplace("ParameterSettings", std::make_unique<UniformBufferObject>("ParameterSettings", appState_->parameterSettings, 4));
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

void UniformBufferManager::updateUBOs() {
    uboMap_["UniversalShaderSettings"]->updateUniformBufferObject(appState_->universalShaderSettings);
    uboMap_["SlimeSettings"]->updateUniformBufferObject(appState_->slimeSettings);
    uboMap_["TrailDiffusionSettings"]->updateUniformBufferObject(appState_->trailDiffusionSettings);
    uboMap_["FragmentShaderSettings"]->updateUniformBufferObject(appState_->fragmentShaderSettings);
    uboMap_["ParameterSettings"]->updateUniformBufferObject(appState_->parameterSettings);
}

void UniformBufferManager::onNotify(const Event event) {
    switch(event) {
        case Event::NEW_CANVAS:
            updateUBOs();
            break;
        default:
            break;
    }
}