/*******************************************************************************

 BEGIN_JUCE_MODULE_DECLARATION

  ID:               lh_thunderforge
  vendor:           Lukas Hansen Audio
  version:          1.0.0
  name:             AeroTone Thunderforge Core
  description:      Boutique GUI and DSP modules for the Thunderforge suite.
  website:          https://github.com/Tehuti01/thunderforge-vst
  license:          Proprietary

  dependencies:     juce_gui_basics juce_graphics juce_events juce_audio_basics

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>

#include "LookAndFeel/ThunderforgeLookAndFeel.h"
#include "Components/KnobComponent.h"
#include "Components/VU_Meter.h"
#include "Components/LCDComponent.h"
#include "Components/AmpFaceComponent.h"
#include "Components/LEDComponent.h"
#include "Components/SignalChainComponent.h"
#include "Components/StatusBarComponent.h"
