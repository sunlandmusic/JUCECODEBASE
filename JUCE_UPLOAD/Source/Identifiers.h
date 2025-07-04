#pragma once
#include <JuceHeader.h>

namespace IDs
{
    // Main ValueTree type
    const juce::Identifier APP_STATE ("AppState");

    // Properties for SettingsPanelXL / Inversion
    const juce::Identifier INVERSION_SELECTED ("inversionSelected");
    const juce::Identifier INVERSION_VALUE ("inversionValue");

    // We can add more identifiers here later for other settings
    // const juce::Identifier SELECTED_KEY ("selectedKey");
    // const juce::Identifier SELECTED_MODE ("selectedMode");
    // const juce::Identifier SELECTED_OCTAVE ("selectedOctave");
    // const juce::Identifier SELECTED_SOUND ("selectedSound");
}
