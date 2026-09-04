#ifndef UNIFORM_BUFFER_MANAGER_H
#define UNIFORM_BUFFER_MANAGER_H

#include <GL/glew.h>
#include <initializer_list>
#include <memory>
#include <string>
#include <unordered_map>

#include "uniform_buffer_object.h"
#include "../utility/observer.h"

class ApplicationState;  //forward declaration to avoid circular dependency

class UniformBufferManager : public Observer {

public:
    explicit UniformBufferManager(ApplicationState* appState);
    ~UniformBufferManager() = default;

    UniformBufferManager() = delete;
    UniformBufferManager(const UniformBufferManager&) = delete;
    UniformBufferManager& operator=(const UniformBufferManager&) = delete;
    UniformBufferManager(UniformBufferManager&&) = delete;
    UniformBufferManager& operator=(UniformBufferManager&&) = delete;

    void attachUBOs(std::initializer_list<GLuint> shaderProgramIDs);
    void updateUBOs();

    void onNotify(const UserEvent event) override;

private:

    ApplicationState* appState_;
    std::unordered_map<std::string, std::unique_ptr<UniformBufferObject>> uboMap_;

};

#endif // UNIFORM_BUFFER_MANAGER_H