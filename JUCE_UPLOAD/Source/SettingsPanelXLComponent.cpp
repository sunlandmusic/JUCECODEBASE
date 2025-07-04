#include "SettingsPanelXLComponent.h"
#include <iostream>

// Constructor updated to take ValueTree
SettingsPanelXLComponent::SettingsPanelXLComponent(juce::ValueTree applicationState)
    : appState(applicationState) // Store the ValueTree
{
    // Add this component as a listener to the appState ValueTree
    appState.addListener(this);

    // Set initial size
    setSize(928, static_cast<int>(panelHeight));

    // Initialize buttons (rest of the constructor is similar)
    addAndMakeVisible(eyeButton);
    eyeButton.setBackgroundColour(buttonColor);
    eyeButton.setBorderColour(buttonBorder);

    addAndMakeVisible(skinButton);
    skinButton.setBackgroundColour(buttonColor);
    skinButton.setBorderColour(buttonBorder);

    addAndMakeVisible(memoryButton);
    memoryButton.setBackgroundColour(buttonColor);
    memoryButton.setBorderColour(buttonBorder);

    addAndMakeVisible(disableButton);
    disableButton.setBackgroundColour(buttonColor);
    disableButton.setBorderColour(buttonBorder);

    addAndMakeVisible(bassOffsetButton);
    bassOffsetButton.setBackgroundColour(buttonColor);
    bassOffsetButton.setBorderColour(buttonBorder);

    instrumentSelector.setLookAndFeel(&customLookAndFeel);
    addAndMakeVisible(instrumentSelector);
    instrumentSelector.addItem("BALAFON", 1);
    instrumentSelector.setSelectedId(1);

    modeSelector.setLookAndFeel(&customLookAndFeel);
    addAndMakeVisible(modeSelector);
    modeSelector.addItem("FREE", 1);
    modeSelector.setSelectedId(1);
    modeSelector.addListener(this);
    modeSelector.getProperties().set("isSelected", false);

    addAndMakeVisible(keyLabel);
    keyLabel.setText("KEY", juce::dontSendNotification);
    keyLabel.setFont(smallLabelFont);
    keyLabel.setColour(juce::Label::textColourId, labelColor);
    keyLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(keyValueLabel);
    keyValueLabel.setText("C", juce::dontSendNotification);
    keyValueLabel.setFont(displayFont);
    keyValueLabel.setColour(juce::Label::textColourId, textColor);
    keyValueLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(octaveLabel);
    octaveLabel.setText("OCT", juce::dontSendNotification);
    octaveLabel.setFont(smallLabelFont);
    octaveLabel.setColour(juce::Label::textColourId, labelColor);
    octaveLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(octaveValueLabel);
    octaveValueLabel.setText("0", juce::dontSendNotification); // Will be updated by ValueTree listener
    octaveValueLabel.setFont(displayFont);
    octaveValueLabel.setColour(juce::Label::textColourId, textColor);
    octaveValueLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(inversionLabel);
    inversionLabel.setText("INV", juce::dontSendNotification);
    inversionLabel.setFont(smallLabelFont);
    inversionLabel.setColour(juce::Label::textColourId, labelColor);
    inversionLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(inversionValueLabel);
    // Initialize text from appState
    inversionValueLabel.setText(juce::String(appState.getProperty(IDs::INVERSION_VALUE, 0)), juce::dontSendNotification);
    inversionValueLabel.setFont(displayFont);
    inversionValueLabel.setColour(juce::Label::textColourId, textColor);
    inversionValueLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(chordLabel);
    chordLabel.setText("CHORD", juce::dontSendNotification);
    chordLabel.setFont(smallLabelFont);
    chordLabel.setColour(juce::Label::textColourId, labelColor);
    chordLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(chordDisplay);
    chordDisplay.setText("C#", juce::dontSendNotification);
    chordDisplay.setFont(chordDisplayFont);
    chordDisplay.setColour(juce::Label::textColourId, textColor);
    chordDisplay.setJustificationType(juce::Justification::centred);

    createSelectableContainer(keyLabel, keyValueLabel, "key");
    createSelectableContainer(octaveLabel, octaveValueLabel, "octave");
    createSelectableContainer(inversionLabel, inversionValueLabel, "inversion");

    // Manually call valueTreePropertyChanged for initial setup of inversion selection visuals
    // This ensures the visual state matches the initial ValueTree state.
    // We need to ensure selectedControl is also initialized if needed for the logic in VTPC.
    // If appState says inversion is selected, then selectedControl should also be "inversion".
    if (appState.isValid() && (bool)appState.getProperty(IDs::INVERSION_SELECTED, false)) {
        selectedControl = "inversion";
    }

    if (appState.isValid()) {
        valueTreePropertyChanged(appState, IDs::INVERSION_SELECTED);
        valueTreePropertyChanged(appState, IDs::INVERSION_VALUE);
    }
}

// Destructor
SettingsPanelXLComponent::~SettingsPanelXLComponent()
{
    if (appState.isValid()) // Check if appState is valid before removing listener
        appState.removeListener(this);
    instrumentSelector.setLookAndFeel(nullptr);
    modeSelector.setLookAndFeel(nullptr);
    modeSelector.removeListener(this);
}

// Method to get current inversion value from ValueTree
int SettingsPanelXLComponent::getInversionValue() const
{
    if (!appState.isValid()) return 0; // Default if appState is not valid
    return appState.getProperty(IDs::INVERSION_VALUE, 0);
}

void SettingsPanelXLComponent::setSelectedControl(const juce::String& control)
{
    if (selectedControl != control)
    {
        juce::String oldSelectedControl = selectedControl;
        selectedControl = control;

        // If the new selection is "inversion", ensure ValueTree reflects this.
        if (control == "inversion") {
            if (!(bool)appState.getProperty(IDs::INVERSION_SELECTED, false)) {
                appState.setProperty(IDs::INVERSION_SELECTED, true, nullptr);
            }
        }
        // If "inversion" was previously selected, and now something else is, deselect inversion in ValueTree.
        else if (oldSelectedControl == "inversion") {
            if ((bool)appState.getProperty(IDs::INVERSION_SELECTED, false)) {
                appState.setProperty(IDs::INVERSION_SELECTED, false, nullptr);
            }
        }
        // Note: valueTreePropertyChanged will handle visual updates for inversion labels.
        // For other controls, direct visual updates might still be needed here or by them listening to selectedControl.
        // For now, let's ensure toggleSelection handles this logic.
    }
}


// toggleSelection now updates the ValueTree
void SettingsPanelXLComponent::toggleSelection(const juce::String& control)
{
    juce::String previouslySelectedControl = selectedControl;

    if (selectedControl == control) // Clicking the same control again
    {
        selectedControl = ""; // Deselect it (UI tracking)
        if (control == "inversion") {
            appState.setProperty(IDs::INVERSION_SELECTED, false, nullptr); // Update ValueTree
        }
    }
    else // Clicking a new control
    {
        selectedControl = control; // Select it (UI tracking)
        if (control == "inversion") {
            appState.setProperty(IDs::INVERSION_SELECTED, true, nullptr); // Update ValueTree
        } else {
            // If a non-inversion control is selected, ensure inversion is marked as not selected in ValueTree
            if ((bool)appState.getProperty(IDs::INVERSION_SELECTED, false)) {
                appState.setProperty(IDs::INVERSION_SELECTED, false, nullptr);
            }
        }
    }

    // If the previously selected control was "inversion" and it's no longer the current (or any) selection,
    // ensure its ValueTree state is false, unless the current selection IS "inversion".
    if (previouslySelectedControl == "inversion" && selectedControl != "inversion") {
        if ((bool)appState.getProperty(IDs::INVERSION_SELECTED, false)) {
             appState.setProperty(IDs::INVERSION_SELECTED, false, nullptr);
        }
    }

    // Visual updates for non-inversion labels (key, octave)
    auto updateLabelPairVisuals = [this](juce::Label& label, juce::Label& value, const juce::String& ctrlName) {
        bool isSelected = selectedControl == ctrlName; // Based on internal selectedControl
        
        label.setColour(juce::Label::backgroundColourId, buttonColor);
        value.setColour(juce::Label::backgroundColourId, buttonColor);
        if (isSelected) {
            label.setColour(juce::Label::outlineColourId, selectedBorder);
            value.setColour(juce::Label::outlineColourId, selectedBorder);
            label.setBorderSize(juce::BorderSize<int>(2));
            value.setBorderSize(juce::BorderSize<int>(2));
            label.setColour(juce::Label::backgroundColourId, buttonColor.brighter(0.1f));
            value.setColour(juce::Label::backgroundColourId, buttonColor.brighter(0.1f));
        } else {
            label.setColour(juce::Label::outlineColourId, juce::Colours::transparentBlack);
            value.setColour(juce::Label::outlineColourId, juce::Colours::transparentBlack);
            label.setBorderSize(juce::BorderSize<int>(0));
            value.setBorderSize(juce::BorderSize<int>(0));
        }
        label.repaint();
        value.repaint();
    };

    updateLabelPairVisuals(keyLabel, keyValueLabel, "key");
    updateLabelPairVisuals(octaveLabel, octaveValueLabel, "octave");
    // Inversion label visuals are handled by valueTreePropertyChanged.

    modeSelector.getProperties().set("isSelected", selectedControl == "mode");
    modeSelector.repaint();

    std::cout << "Selected control (UI): " << (selectedControl.isEmpty() ? "none" : selectedControl) << std::endl;
}


// setInversionValue now updates the ValueTree
void SettingsPanelXLComponent::setInversionValue(int newValue)
{
    if (!appState.isValid()) return;
    if ((int)appState.getProperty(IDs::INVERSION_VALUE, 0) != newValue)
    {
        appState.setProperty(IDs::INVERSION_VALUE, newValue, nullptr);
    }
}

// valueTreePropertyChanged implementation
void SettingsPanelXLComponent::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property)
{
    if (treeWhosePropertyHasChanged == appState)
    {
        if (property == IDs::INVERSION_SELECTED)
        {
            bool isSelectedFromState = appState.getProperty(IDs::INVERSION_SELECTED, false);
            int val = appState.getProperty(IDs::INVERSION_VALUE, 0);

            // Update visual state for inversion labels
            // Visual selection should depend on BOTH appState's INVERSION_SELECTED AND internal `selectedControl`
            bool showVisualSelection = isSelectedFromState && (selectedControl == "inversion");

            inversionLabel.setColour(juce::Label::backgroundColourId, buttonColor);
            inversionValueLabel.setColour(juce::Label::backgroundColourId, buttonColor);

            if (showVisualSelection)
            {
                inversionLabel.setColour(juce::Label::outlineColourId, selectedBorder);
                inversionValueLabel.setColour(juce::Label::outlineColourId, selectedBorder);
                inversionLabel.setBorderSize(juce::BorderSize<int>(2));
                inversionValueLabel.setBorderSize(juce::BorderSize<int>(2));
                inversionLabel.setColour(juce::Label::backgroundColourId, buttonColor.brighter(0.1f));
                inversionValueLabel.setColour(juce::Label::backgroundColourId, buttonColor.brighter(0.1f));
            }
            else
            {
                inversionLabel.setColour(juce::Label::outlineColourId, juce::Colours::transparentBlack);
                inversionValueLabel.setColour(juce::Label::outlineColourId, juce::Colours::transparentBlack);
                inversionLabel.setBorderSize(juce::BorderSize<int>(0));
                inversionValueLabel.setBorderSize(juce::BorderSize<int>(0));
            }
            inversionLabel.repaint();
            inversionValueLabel.repaint();

            listeners.call([isSelectedFromState, val](Listener& l) { l.inversionSelectionChanged(isSelectedFromState, val); });
            std::cout << "VT: INVERSION_SELECTED changed to: " << isSelectedFromState << std::endl;
        }
        else if (property == IDs::INVERSION_VALUE)
        {
            int newValue = appState.getProperty(IDs::INVERSION_VALUE, 0);
            inversionValueLabel.setText(juce::String(newValue), juce::dontSendNotification);

            bool isSelectedFromState = appState.getProperty(IDs::INVERSION_SELECTED, false);
            if (isSelectedFromState)
            {
                 listeners.call([isSelectedFromState, newValue](Listener& l) { l.inversionSelectionChanged(isSelectedFromState, newValue); });
            }
            std::cout << "VT: INVERSION_VALUE changed to: " << newValue << std::endl;
        }
    }
}

void SettingsPanelXLComponent::mouseDown(const juce::MouseEvent& event)
{
    auto* clickedComponent = event.eventComponent;

    if (auto* label = dynamic_cast<juce::Label*>(clickedComponent))
    {
        juce::String controlName;

        if (label == &keyLabel || label == &keyValueLabel)
            controlName = "key";
        else if (label == &octaveLabel || label == &octaveValueLabel)
            controlName = "octave";
        else if (label == &inversionLabel || label == &inversionValueLabel)
            controlName = "inversion";
        
        if (controlName.isNotEmpty())
        {
            toggleSelection(controlName);
        }
    }
}

void SettingsPanelXLComponent::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == &modeSelector)
    {
        toggleSelection("mode");
    }
}

void SettingsPanelXLComponent::createSelectableContainer(juce::Label& label, juce::Label& value, const juce::String& controlName)
{
    label.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    value.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    label.setInterceptsMouseClicks(true, false);
    value.setInterceptsMouseClicks(true, false);
    label.setName(controlName);
    value.setName(controlName);
    label.setColour(juce::Label::backgroundColourId, buttonColor);
    value.setColour(juce::Label::backgroundColourId, buttonColor);
}

void SettingsPanelXLComponent::paint(juce::Graphics& g)
{
    g.setColour(backgroundColor);
    g.fillRoundedRectangle(getLocalBounds().toFloat(), cornerRadius);
    auto drawIcon = [&](const juce::Rectangle<float>& bounds, const juce::String& text, const juce::Colour& iconColor) {
        g.setColour(iconColor);
        g.setFont(displayFont);
        g.drawText(text, bounds, juce::Justification::centred);
    };
    drawIcon(eyeButton.getBounds().toFloat(), String::fromUTF8("\xF0\x9F\x91\x81"), textColor);
    drawIcon(skinButton.getBounds().toFloat(), String::fromUTF8("\xE2\x86\x91"), textColor);
    drawIcon(memoryButton.getBounds().toFloat(), String::fromUTF8("\xF0\x9F\x96\xAB"), textColor);
    drawIcon(disableButton.getBounds().toFloat(), "X", juce::Colour::fromFloatRGBA(0.6f, 0.6f, 0.6f, 1.0f));
    drawIcon(bassOffsetButton.getBounds().toFloat(), String::fromUTF8("\xF0\x9D\x84\xA2"), textColor);
}

void SettingsPanelXLComponent::resized()
{
    auto bounds = getLocalBounds();
    const float padding = 11.0f;
    float x = padding + 45.0f;
    auto layoutCircularButton = [&](IconButton& button) {
        button.setBounds(
            static_cast<int>(x),
            static_cast<int>((panelHeight - circularButtonSize) / 2),
            static_cast<int>(circularButtonSize),
            static_cast<int>(circularButtonSize)
        );
        x += circularButtonSize + padding;
    };
    layoutCircularButton(eyeButton);
    layoutCircularButton(skinButton);
    layoutCircularButton(memoryButton);
    layoutCircularButton(disableButton);
    layoutCircularButton(bassOffsetButton);
    const float comboWidth = 132.0f;
    const float comboHeight = 50.6f;
    const float comboY = (panelHeight - comboHeight) / 2;
    instrumentSelector.setBounds(
        static_cast<int>(x),
        static_cast<int>(comboY),
        static_cast<int>(comboWidth),
        static_cast<int>(comboHeight)
    );
    x += comboWidth + padding;
    const float keyWidth = 66.0f;
    const float labelHeight = 16.5f;
    const float valueHeight = 27.5f;
    const float totalHeight = labelHeight + valueHeight;
    const float labelY = (panelHeight - totalHeight) / 2;
    auto layoutStackedLabels = [&](juce::Label& label, juce::Label& value, float width) {
        label.setBounds(
            static_cast<int>(x),
            static_cast<int>(labelY),
            static_cast<int>(width),
            static_cast<int>(labelHeight)
        );
        value.setBounds(
            static_cast<int>(x),
            static_cast<int>(labelY + labelHeight),
            static_cast<int>(width),
            static_cast<int>(valueHeight)
        );
        x += width + padding;
    };
    layoutStackedLabels(keyLabel, keyValueLabel, keyWidth);
    modeSelector.setBounds(
        static_cast<int>(x),
        static_cast<int>(comboY),
        static_cast<int>(88.0f),
        static_cast<int>(comboHeight)
    );
    x += 88.0f + padding;
    const float numberWidth = 66.0f;
    layoutStackedLabels(octaveLabel, octaveValueLabel, numberWidth);
    layoutStackedLabels(inversionLabel, inversionValueLabel, numberWidth);
    float chordLabelX = x - 20.0f;
    float chordDisplayX = x - 45.0f;
    chordLabel.setBounds(
        static_cast<int>(chordLabelX),
        static_cast<int>(labelY),
        static_cast<int>(numberWidth),
        static_cast<int>(labelHeight)
    );
    chordDisplay.setBounds(
        static_cast<int>(chordDisplayX),
        static_cast<int>(labelY + labelHeight),
        static_cast<int>(numberWidth * 2),
        static_cast<int>(valueHeight)
    );
}
