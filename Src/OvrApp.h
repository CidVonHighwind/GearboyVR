#ifndef OVRAPP_H
#define OVRAPP_H

#include "App.h"
#include "SceneView.h"
#include "SoundEffectContext.h"
#include "GuiSys.h"

namespace OVR {
	class ovrLocale;
}

class OvrApp : public OVR::VrAppInterface
{
public:
							OvrApp();
	virtual					~OvrApp();

    virtual void	        SetUpMenu();
	virtual void 			Configure( OVR::ovrSettings & settings );
	virtual void			EnteredVrMode( const OVR::ovrIntentType intentType, const char * intentFromPackage, const char * intentJSON, const char * intentURI );
	virtual void			LeavingVrMode();
	virtual bool 			OnKeyEvent( const int keyCode, const int repeatCount, const OVR::KeyEventType eventType );

	virtual OVR::ovrFrameResult Frame( const OVR::ovrFrameInput & vrFrame );

	class OVR::ovrLocale &	GetLocale() { return *Locale; }

private:
	OVR::ovrSoundEffectContext * SoundEffectContext;
	OVR::OvrGuiSys::SoundEffectPlayer * SoundEffectPlayer;
	OVR::OvrGuiSys *		GuiSys;
	OVR::ovrLocale *		Locale;

	OVR::ModelFile *		SceneModel;
	OVR::OvrSceneView		Scene;
};

#endif	// OVRAPP_H
