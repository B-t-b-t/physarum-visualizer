#include "application_state.h"

ApplicationState* ApplicationState::singleton_ = nullptr;

ApplicationState* ApplicationState::getInstance(Parameters& params) {
	if(singleton_ == nullptr) {
		singleton_ = new ApplicationState(params);
	}

	return singleton_;
}

ApplicationState::ApplicationState(Parameters& params) 
 : numParticles{params.numParticles},
   newNumParticles{params.numParticles},
   slimeRatio{params.slimeRatio},
   workGroupDivider{params.workGroupDivider}
{  
	universalShaderSettings.windowWidth = params.width;
	universalShaderSettings.windowHeight = params.height;
}