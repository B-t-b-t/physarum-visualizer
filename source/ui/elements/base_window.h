#ifndef BASE_WINDOW_H
#define BASE_WINDOW_H

#include "../../application_state.h"
#include "../../utility/observable.h"

class BaseWindow : public Observable {
public:
    virtual ~BaseWindow() = default;

    virtual void render(ApplicationState* appState) = 0;
    bool visible = false;
    float widgetWidth = 200.0f; //used for ImGui::PushItemWidth in child windows
};

#endif // BASE_WINDOW_H