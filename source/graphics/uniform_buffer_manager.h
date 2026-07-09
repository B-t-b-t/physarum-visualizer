#ifndef UNIFORM_BUFFER_MANAGER_H
#define UNIFORM_BUFFER_MANAGER_H

#include <initializer_list>
#include <memory>
#include <string>
#include <unordered_map>

#include "uniform_buffer_object.h"
#include "../application_state.h"

class UniformBufferManager {

public:
    explicit UniformBufferManager(ApplicationState* uiState);
    ~UniformBufferManager() = default;

    UniformBufferManager() = delete;
    UniformBufferManager(const UniformBufferManager&) = delete;
    UniformBufferManager& operator=(const UniformBufferManager&) = delete;
    UniformBufferManager(UniformBufferManager&&) = delete;
    UniformBufferManager& operator=(UniformBufferManager&&) = delete;

    void attachUBOs(std::initializer_list<GLuint> shaderProgramIDs);
    void updateUBOs(ApplicationState* uiState);

private:

    std::unordered_map<std::string, std::unique_ptr<UniformBufferObject>> uboMap_;

};

#endif // UNIFORM_BUFFER_MANAGER_H