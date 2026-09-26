#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

// The plug-in window (docs/product-design/CONTROL-MAP.md):
//   PLAY    760 x 480  - what a hand touches during a song (the lead instance)
//   EDIT    1180 x 480 - PLAY plus a drawer: LOOK, COLOUR, REACT, MOTION, MEDIA, SETUP
//   SOURCE  360 x 240  - every instance that is not the lead (a drum track...)
// Visual language: warm black, bone text, red ("signal") only for what is
// live / output, amber ("tungsten") only for what is pending.
namespace vjui
{
    const juce::Colour ink0 { 0xff0c0a09 };     // window
    const juce::Colour ink1 { 0xff151210 };     // panels, tiles, buttons
    const juce::Colour ink2 { 0xff1e1a17 };     // hover, toggled on
    const juce::Colour line { 0xff3a322c };     // hairlines, knob tracks
    const juce::Colour bone { 0xffece4d6 };     // text, value arcs
    const juce::Colour ash { 0xffa89e90 };      // labels
    const juce::Colour dust { 0xff8a7f73 };     // hints, disabled
    const juce::Colour signal { 0xfff0503f };   // LIVE / output only
    const juce::Colour tungsten { 0xffe9a450 }; // pending only
    const juce::Colour halation { 0xfff2937f }; // modulation, meters
    const juce::Colour blood { 0xffb3160e };

    juce::Font font (float height, bool bold = false);
    juce::Font mono (float height);

    class LookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        LookAndFeel();
        void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos, float start, float end, juce::Slider&) override;
        void drawLinearSlider (juce::Graphics&, int x, int y, int w, int h, float pos, float minPos, float maxPos,
                               juce::Slider::SliderStyle, juce::Slider&) override;
        void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool over, bool down) override;
        void drawButtonText (juce::Graphics&, juce::TextButton&, bool over, bool down) override;
        juce::Font getTextButtonFont (juce::TextButton&, int height) override;
        void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool over, bool down) override;
        void drawPopupMenuBackground (juce::Graphics&, int width, int height) override;
        void drawPopupMenuItem (juce::Graphics&, const juce::Rectangle<int>& area, bool isSeparator, bool isActive, bool isHighlighted,
                                bool isTicked, bool hasSubMenu, const juce::String& text, const juce::String& shortcutKeyText,
                                const juce::Drawable* icon, const juce::Colour* textColour) override;
        juce::Font getPopupMenuFont() override;
        void drawTooltip (juce::Graphics&, const juce::String& text, int width, int height) override;
        juce::Rectangle<int> getTooltipBounds (const juce::String& tipText, juce::Point<int> screenPos, juce::Rectangle<int> parentArea) override;
    };

    // Help text for the info line (and the delayed tooltip).
    void setHelp (juce::Component&, const juce::String& text);
}

// A rotary knob bound to a host parameter. The value sits in the centre in
// real units; under the name, an optional line says what the knob does in
// this scene ("ripples") or a live readout ("x1.30", "FROZEN").
class MacroKnob : public juce::Component
{
public:
    MacroKnob (juce::AudioProcessorValueTreeState&, const juce::String& paramId, const juce::String& label, bool large, bool offAtZero = false);
    void resized() override;
    void paint (juce::Graphics&) override;
    void paintOverChildren (juce::Graphics&) override;

    void setSubLabel (const juce::String& text, juce::Colour colour);
    void setActivity (float a); // outer ring: how far the sound moves this knob's targets (0-1)
    void setHelpText (const juce::String& text) { vjui::setHelp (slider, text); vjui::setHelp (*this, text); }
    juce::Slider& getSlider() noexcept { return slider; }

private:
    juce::Slider slider;
    juce::String label, subLabel;
    juce::Colour subColour { vjui::ash };
    float activity = 0.0f;
    bool large;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
};

// A row of mutually exclusive cells (STILL · BREATHE · PULSE · PUNCH ...).
class SegmentedControl : public juce::Component
{
public:
    explicit SegmentedControl (juce::StringArray cellNames);
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;

    void setSelected (int index);            // -1 = none
    int getSelected() const noexcept { return selected; }
    void setCellHelp (int index, const juce::String& text) { cellHelp.set (index, text); }
    void setPending (int index) { if (index != pending) { pending = index; repaint(); } } // amber outline (e.g. STILL fading)
    int cellAt (juce::Point<int>) const;
    std::function<void (int)> onChange;

private:
    juce::StringArray names;
    juce::StringArray cellHelp;
    int selected = -1, hovered = -1, pending = -1;
};

// The scene grid: 4 columns of tiles. Click = cue (NEXT), double-click = cue + GO.
class SceneGrid : public juce::Component
{
public:
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;

    struct State
    {
        juce::StringArray names;
        juce::Array<bool> usesMedia;
        juce::StringArray descriptions;
        int live = -1, cued = -1, firing = -1;
        bool connected = false;
    };
    void setState (const State&);
    juce::Rectangle<int> tileBounds (int index) const; // in this component (harness clicks here)
    int tileAt (juce::Point<int>) const;
    int hoveredTile() const noexcept { return hovered; }
    const State& getState() const noexcept { return st; }

    std::function<void (int)> onCue, onFire;

private:
    State st;
    int hovered = -1, scrollRow = 0;
    static constexpr int columns = 4, tileW = 109, tileH = 30, stepX = 113, stepY = 34, visibleRows = 4;
};

// A tile for one moment (snapshot A-D): click an empty one to save, a stored one to recall.
class MomentTile : public juce::Button
{
public:
    explicit MomentTile (int slotIndex) : juce::Button ("moment"), slot (slotIndex) {}
    void paintButton (juce::Graphics&, bool over, bool down) override;
    void mouseDown (const juce::MouseEvent& e) override { if (e.mods.isPopupMenu()) { if (onRightClick) onRightClick (e); return; } juce::Button::mouseDown (e); }
    void mouseUp (const juce::MouseEvent& e) override { if (! e.mods.isPopupMenu()) juce::Button::mouseUp (e); }
    std::function<void (const juce::MouseEvent&)> onRightClick;
    void setInfo (bool stored, bool active, const juce::String& recipe, float progress, bool justSaved);

private:
    int slot;
    bool stored = false, active = false, saved = false;
    juce::String recipe;
    float progress = -1.0f;
};

class VJAnalyzerEditor : public juce::AudioProcessorEditor,
                         public juce::FileDragAndDropTarget,
                         private juce::Timer
{
public:
    explicit VJAnalyzerEditor (VJAnalyzerProcessor&);
    ~VJAnalyzerEditor() override;

    void paint (juce::Graphics&) override;
    void paintOverChildren (juce::Graphics&) override;
    void resized() override;

    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void filesDropped (const juce::StringArray& files, int x, int y) override;

    SceneGrid& getSceneGrid() noexcept { return sceneGrid; } // PluginHarness --cuetest

private:
    enum class Mode { play, edit, source };
    enum Tab { lookTab = 0, colourTab, reactTab, motionTab, mediaTab, setupTab, numTabs };

    void timerCallback() override;
    void setParameter (const juce::String& id, float plainValue);
    float param (const juce::String& id) const;
    void openEngine();
    void launchEngineAt (const juce::File& exe);
    void selectPreset (int engineIndex);
    void editColour (int index);
    void loadMedia (int slot);
    void ensureMediaScene();
    bool sceneUsesMedia (int index) const { return juce::isPositiveAndBelow (index, status.presetUsesMedia.size()) && status.presetUsesMedia[index]; }
    void updateMode();
    void layoutPlay();
    void layoutDrawer();
    void layoutSource();
    void showTab (int tab);
    void updateKnobLabels();
    void updateInfoLine();
    void setInfo (const juce::String& text, juce::Colour colour = vjui::dust);
    void paintHeader (juce::Graphics&);
    void paintPlay (juce::Graphics&);
    void paintDrawer (juce::Graphics&);
    void paintSource (juce::Graphics&);
    void paintSignal (juce::Graphics&, juce::Rectangle<int> spectrum, juce::Rectangle<int> bands, juce::Rectangle<int> lamps);
    void showPaletteMenu();
    void showLookMenu();
    void showMorphMenu();
    void momentClicked (int slot, const juce::MouseEvent* rightClick);
    juce::String recipeName (int palette, int look) const;
    juce::StringArray cachedSceneNames() const;
    void rememberScenes();
    double now() const;

    VJAnalyzerProcessor& processor;
    vjui::LookAndFeel lookAndFeel;
    Mode mode = Mode::play;
    int currentTab = lookTab;
    AnalysisWorker::Meters meters;
    AnalysisWorker::EngineStatus status;
    int shownCue = -1, shownFired = -1;
    double launchedAt = -100.0, hitFlash = -10.0, dropFlash = -10.0, savedFlash = -10.0;
    int savedSlot = -1, lastDrops = 0;
    juce::String infoText, stateHint;
    juce::Colour infoColour { vjui::dust }, stateColour { vjui::dust };
    double infoHoldUntil = 0.0;

    // --- header
    juce::TextButton enginePill, fullscreenButton { "FULL" }, leadChip, editButton { "EDIT" }, blackout { "BLACKOUT" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> blackoutAttachment;

    // --- PLAY: scenes
    juce::TextButton previous { "<" }, next { ">" }, go { "GO" }, cueField;
    SceneGrid sceneGrid;

    // --- PLAY: react row, lamps, style, media, moments
    SegmentedControl reactRow { { "STILL", "BREATHE", "PULSE", "PUNCH" } };
    juce::OwnedArray<juce::TextButton> lamps; // KICK SNARE HAT BASS LEVEL (click = listen / ignore)
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> lampAttachments;
    juce::TextButton colourPicker, lookPicker, mediaChip, loadButton { "LOAD" }, morphButton;
    juce::Slider lookAmount;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lookAmountAttachment;
    juce::OwnedArray<MomentTile> moments;

    // --- PLAY: the 8 macros, FREEZE, HIT, DROP
    juce::OwnedArray<MacroKnob> macros; // by slot 0-7
    juce::TextButton freeze { "FREEZE" }, hit { "HIT" }, drop { "DROP" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> freezeAttachment;

    // --- EDIT drawer
    juce::OwnedArray<juce::TextButton> tabButtons;
    juce::Array<juce::Component*> tabContents[numTabs];
    // LOOK
    SegmentedControl lookRow { { "CLEAN", "FILM", "WORN", "BROKEN", "PRINT", "DATA" } };
    juce::Slider lookAmount2;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lookAmount2Attachment;
    juce::OwnedArray<MacroKnob> filmKnobs, digitalKnobs, hitKnobs;
    juce::TextButton lookReset;
    // COLOUR
    juce::OwnedArray<juce::TextButton> paletteCards;
    juce::OwnedArray<juce::TextButton> swatches;
    // REACT
    SegmentedControl reactRow2 { { "STILL", "BREATHE", "PULSE", "PUNCH" } };
    juce::OwnedArray<MacroKnob> reactKnobs; // REACT, FOLLOW, DECAY, MUSIC PUSH
    juce::OwnedArray<juce::TextButton> follows;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> followAttachments;
    juce::TextButton dropButton2 { "DROP NOW" };
    // MOTION
    std::unique_ptr<MacroKnob> driftKnob;
    juce::TextButton freeze2 { "FREEZE" }, reverse { "REVERSE" }, sync { "TEMPO-LOCK" };
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> motionAttachments;
    SegmentedControl cutRow { { "OFF", "16", "8", "4", "2", "1", "HIT" } };
    // MEDIA
    juce::OwnedArray<juce::TextButton> mediaCards;
    juce::TextButton mediaLoad { "LOAD..." }, mediaClear { "CLEAR" }, useMedia { "USE IN ALL IMAGE SCENES" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> useMediaAttachment;
    SegmentedControl clipModeRow { { "LOOP", "PING-PONG" } };
    SegmentedControl clipSyncRow { { "FREE", "1 BEAT", "1 BAR", "2", "4", "8 BARS" } };
    std::map<juce::String, juce::Image> thumbnails;
    // SETUP
    SegmentedControl roleRow { { "MIX", "KICK", "SNARE", "HAT", "BASS", "TEXT." } };
    juce::TextButton roleAuto { "AUTO" }, canLead { "CAN LEAD" }, makeLead { "MAKE LEAD" }, locateEngine { "LOCATE..." };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> canLeadAttachment;
    juce::OwnedArray<MacroKnob> inputKnobs; // DETECT, TRIM, LOOKAHEAD
    SegmentedControl levelRow { { "AUTO", "FIXED" } };

    // --- SOURCE view
    SegmentedControl sourceRole { { "MIX", "KICK", "SNARE", "HAT", "BASS", "TEXT." } };
    juce::TextButton sourceAuto { "AUTO" }, sourceMakeLead { "MAKE LEAD" };
    juce::OwnedArray<MacroKnob> sourceKnobs; // DETECT, TRIM

    juce::TooltipWindow tooltips { this, 1200 };
    std::unique_ptr<juce::FileChooser> engineChooser, mediaChooser;

    // Painted areas.
    juce::Rectangle<int> infoArea, drawerArea, signalSpectrum, signalBands, signalLamps;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VJAnalyzerEditor)
};
