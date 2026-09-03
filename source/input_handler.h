#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

class ApplicationState; // Forward declaration of ApplicationState class

class InputHandler {

public:
    InputHandler(ApplicationState* appState);

    void processUserInput();

private:

    ApplicationState* appState_;
};

#endif // INPUT_HANDLER_H