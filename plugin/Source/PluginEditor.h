#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

namespace vjui
{
    // One palette for the whole device.
    const juce::Colour background { 0xff0b0d12 };
    const juce::Colour panel { 0xff12151c };
    const juce::Colour outline { 0xff1f2431 };
    const juce::Colour text { 0xffe6e9f0 };
    const juce::Colour dim { 0xff7a8194 };
    const juce::Colour mint { 0xff5cf2c8 };
    const juce::Colour magenta { 0xffff4fa3 };
    const juce::Colour amber { 0xffffb547 };
    const juce::Colour violet { 0xff8c7cff };
    const juce::Colour danger { 0xffff3b3b };

    class LookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        LookAndFeel();
        void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos, float start, float end, juce::Slider&) override;
        void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool over, bool down) override;
        void drawButtonText (juce::Graphics&, juce::TextButton&, bool over, bool down) override;
        juce::Font getTextButtonFont (juce::TextButton&, int height) override;
        void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool over, bool down) override;
    };
}

// A labelled rotary knob bound to a host parameter. Under the name it can show
// a second line: what the knob does in the current scene ("ripples"), or a
// live readout ("x1.3", "FROZEN").
class MacroKnob : public juce::Component
{
public:
    MacroKnob (juce::AudioProcessorValueTreeState&, const juce::String& paramId, const juce::String& label,
               juce::Colour accent, bool large);
    void resized() override;
    void paint (juce::Graphics&) override;

    void setSubLabel (const juce::String& text, juce::Colour colour);
    // Ring outside the knob: how far the sound is moving what this knob controls (0-1).
    void setActivity (float a);
    void paintOverChildren (juce::Graphics&) override;
    void setTooltip (const juce::String& text) { slider.setTooltip (text); }

private:
    juce::Slider slider;
    juce::String label, subLabel;
    float activity = 0.0f;
    juce::Colour accent, subColour;
    bool large;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
};

// One of the three palette colours; click to open a colour picker.
class ColourSwatch : public juce::Component
{
public:
    ColourSwatch (const juce::String& labelText) : label (labelText) {}
    void setColour (juce::Colour c) { if (c != colour) { colour = c; repaint(); } }
    void paint (juce::Graphics&) override;
    void mouseUp (const juce::MouseEvent&) override { if (onClick) onClick(); }
    std::function<void()> onClick;

private:
    juce::String label;
    juce::Colour colour;
};

class VJAnalyzerEditor : public juce::AudioProcessorEditor,
                         private juce::Timer,
                         private juce::ListBoxModel
{
public:
    explicit VJAnalyzerEditor (VJAnalyzerProcessor&);
    ~VJAnalyzerEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    // Preset list (engine's library, live).
    int getNumRows() override;
    void paintListBoxItem (int row, juce::Graphics&, int width, int height, bool selected) override;
    void listBoxItemClicked (int row, const juce::MouseEvent&) override;
    void listBoxItemDoubleClicked (int row, const juce::MouseEvent&) override;

    void setParameter (const juce::String& id, float plainValue);
    void openEngine();
    void launchEngineAt (const juce::File& exe);
    void selectPreset (int engineIndex);

    void editColour (int index);
    void paintSignalPanel (juce::Graphics&, juce::Rectangle<int>);
    void paintHeader (juce::Graphics&, juce::Rectangle<int>);
    void updateKnobLabels();

    VJAnalyzerProcessor& processor;
    vjui::LookAndFeel lookAndFeel;

    juce::OwnedArray<juce::TextButton> roleButtons;
    juce::OwnedArray<MacroKnob> macros;              // by slot 0-7 (placed in SHAPE / MOVE / REACT)
    MacroKnob sensitivity, trim, lookahead, drift, push, softness, reactivity;
    juce::ComboBox response;
    juce::ToggleButton sendControls { "Send macros" };
    juce::TextButton previous { "<" }, next { ">" }, go { "GO" }, hit { "HIT" }, blackout { "BLACKOUT" };
    juce::TextButton freeze { "FREEZE" }, reverse { "REVERSE" }, sync { "SYNC" };
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> moveAttachments;
    juce::ListBox presetList;
    juce::TextButton engineButton { "OPEN ENGINE" }, fullscreenButton { "FULLSCREEN" };
    std::unique_ptr<juce::FileChooser> engineChooser;
    double launchedAt = -100.0;

    juce::OwnedArray<juce::TextButton> snapshotButtons;
    juce::TextButton storeButton { "STORE" };
    juce::ComboBox morphBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> morphAttachment;
    juce::Rectangle<int> snapshotCaption;

    juce::OwnedArray<juce::TextButton> reactButtons;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> reactAttachments;
    juce::TooltipWindow tooltips { this, 500 };
    juce::Rectangle<int> reactToCaption;

    juce::OwnedArray<MacroKnob> filmKnobs, digitalKnobs;
    juce::TextButton calm { "CALM" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> calmAttachment;
    juce::Rectangle<int> filmCaption, digitalCaption, paletteCaption, lookDivider;
    juce::ComboBox paletteBox;
    juce::OwnedArray<ColourSwatch> swatches;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> responseAttachment, paletteAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> sendAttachment, blackoutAttachment;

    AnalysisWorker::Meters meters;
    AnalysisWorker::EngineStatus status;
    juce::Rectangle<int> headerArea, signalArea, shapeArea, moveArea, reactArea, scenesArea, lookArea;
    juce::Rectangle<int> sceneNameArea, sceneTextArea, speedTextArea;
    juce::Rectangle<int> mediaArea, mediaCaption, mediaInfoArea;
    juce::TextButton mediaLoad { "LOAD" }, mediaClear { "CLEAR" }, useMedia { "USE MEDIA" }, clipMode { "LOOP" };
    juce::OwnedArray<juce::TextButton> mediaSlots;
    juce::ComboBox clipSync;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> clipSyncAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> useMediaAttachment;
    std::unique_ptr<juce::FileChooser> mediaChooser;
    void loadMedia();
    // After LOAD / USE MEDIA from the UI: if the live scene can't show media,
    // switch to one that can (Media Negative), so the image appears at once.
    void ensureMediaScene();
    bool sceneUsesMedia (int index) const { return juce::isPositiveAndBelow (index, status.presetUsesMedia.size()) && status.presetUsesMedia[index]; }
    double hitFlash = -10.0;
    int shownCue = -1, shownFired = -1; // what the scene list last painted

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VJAnalyzerEditor)
};
