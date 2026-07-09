#include "ui_state.h"

UIState* UIState::singleton_ = nullptr;

UIState* UIState::getInstance(Parameters& params) {
	if(singleton_ == nullptr) {
		singleton_ = new UIState(params);
	}

	return singleton_;
}

UIState::UIState(Parameters& params) 
 : numParticles{params.numParticles},
   newNumParticles{params.numParticles},
   slimeRatio{params.slimeRatio},
   workGroupDivider{params.workGroupDivider}
{  
	universalShaderSettings.windowWidth = params.width;
	universalShaderSettings.windowHeight = params.height;
}