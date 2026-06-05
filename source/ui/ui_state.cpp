#include "ui_state.h"

UIState* UIState::singleton_ = nullptr;

UIState* UIState::getInstance() {
	if(singleton_ == nullptr) {
		singleton_ = new UIState();
	}

	return singleton_;
}