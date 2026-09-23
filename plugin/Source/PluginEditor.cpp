#include "PluginEditor.h"
#include "EngineLauncher.h"

namespace
{
    double now() { return juce::Time::getMillisecondCounterHiRes() * 0.001; }

    juce::Font uiFont (float height, bool bold = false)
    {
        return juce::Font (juce::FontOptions ("Segoe UI", height, bold ? juce::Font::bold : juce::Font::plain));
    }

    void drawPanel (juce::Graphics& g, juce::Rectangle<int> r, const juce::String& title)
    {
        g.setColour (vjui::panel);
        g.fillRoundedRectangle (r.toFloat(), 10.0f);
        g.setColour (vjui::outline);
        g.drawRoundedRectangle (r.toFloat().reduced (0.5f), 10.0f, 1.0f);
        g.setColour (vjui::dim);
        g.setFont (uiFont (11.0f, true));
        g.drawText (title, r.reduced (14, 8).removeFromTop (14), juce::Justification::topLeft);
    }

    // Glowing lamp that decays over ~200 ms after `lastTime`.
    void drawLamp (juce::Graphics& g, juce::Rectangle<float> r, double lastTime, juce::Colour colour, const juce::String& label)
    {
        auto level = (float) juce::jlimit (0.0, 1.0, 1.0 - (now() - lastTime) / 0.2);
        auto dot = r.removeFromTop (24.0f).withSizeKeepingCentre (16.0f, 16.0f);
        r.removeFromTop (4.0f);
        if (level > 0.0f)
        {
            g.setColour (colour.withAlpha (0.25f * level));
            g.fillEllipse (dot.expanded (5.0f * level));
        }
        g.setColour (vjui::outline.interpolatedWith (colour, level));
        g.fillEllipse (dot);
        g.setColour (level > 0.3f ? vjui::text : vjui::dim);
        g.setFont (uiFont (10.0f, true));
        g.drawText (label, r, juce::Justification::centredTop);
    }
}

//==============================================================================
vjui::LookAndFeel::LookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, background);
    setColour (juce::ComboBox::backgroundColourId, panel.brighter (0.05f));
    setColour (juce::ComboBox::outlineColourId, outline);
    setColour (juce::ComboBox::textColourId, text);
    setColour (juce::ComboBox::arrowColourId, dim);
    setColour (juce::PopupMenu::backgroundColourId, panel);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, mint.withAlpha (0.2f));
    setColour (juce::ListBox::backgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::ToggleButton::textColourId, text);
    setColour (juce::ScrollBar::thumbColourId, outline.brighter (0.3f));
}

void vjui::LookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                                          float start, float end, juce::Slider& slider)
{
    auto accent = slider.findColour (juce::Slider::rotarySliderFillColourId);
    auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (4.0f);
    auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    auto centre = bounds.getCentre();
    auto thickness = juce::jmax (3.0f, radius * 0.16f);
    auto angle = start + pos * (end - start);

    juce::Path track;
    track.addCentredArc (centre.x, centre.y, radius - thickness, radius - thickness, 0.0f, start, end, true);
    g.setColour (outline.brighter (0.15f));
    g.strokePath (track, juce::PathStrokeType (thickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path value;
    value.addCentredArc (centre.x, centre.y, radius - thickness, radius - thickness, 0.0f, start, angle, true);
    g.setColour (accent.withAlpha (0.25f));
    g.strokePath (value, juce::PathStrokeType (thickness * 2.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (accent);
    g.strokePath (value, juce::PathStrokeType (thickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Body + pointer.
    auto body = radius - thickness * 2.6f;
    g.setColour (background);
    g.fillEllipse (centre.x - body, centre.y - body, body * 2.0f, body * 2.0f);
    g.setColour (outline.brighter (0.2f));
    g.drawEllipse (centre.x - body, centre.y - body, body * 2.0f, body * 2.0f, 1.0f);
    auto tip = centre.getPointOnCircumference (body * 0.8f, angle);
    auto base = centre.getPointOnCircumference (body * 0.35f, angle);
    g.setColour (text);
    g.drawLine ({ base, tip }, juce::jmax (1.5f, radius * 0.06f));

    g.setColour (text);
    g.setFont (uiFont (juce::jmax (10.0f, body * 0.42f), true));
    g.drawText (juce::String (juce::roundToInt (slider.getValue() * (slider.getMaximum() <= 1.0 ? 100.0 : 1.0))),
                juce::Rectangle<float> (centre.x - body, centre.y + body * 0.15f, body * 2.0f, body * 0.7f).toNearestInt(),
                juce::Justification::centred);
}

void vjui::LookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced (1.0f);
    auto accent = b.findColour (juce::TextButton::buttonOnColourId);
    auto on = b.getToggleState();
    g.setColour (on ? accent.withAlpha (0.9f) : panel.brighter (over ? 0.12f : 0.05f));
    if (down) g.setColour (accent.withAlpha (0.6f));
    g.fillRoundedRectangle (r, 7.0f);
    g.setColour (on ? accent : outline.brighter (over ? 0.4f : 0.15f));
    g.drawRoundedRectangle (r, 7.0f, 1.0f);
}

void vjui::LookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& b, bool, bool)
{
    g.setColour (b.getToggleState() ? background : text);
    g.setFont (getTextButtonFont (b, b.getHeight()));
    g.drawText (b.getButtonText(), b.getLocalBounds(), juce::Justification::centred);
}

juce::Font vjui::LookAndFeel::getTextButtonFont (juce::TextButton&, int height)
{
    return uiFont (juce::jlimit (11.0f, 15.0f, height * 0.42f), true);
}

void vjui::LookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool over, bool)
{
    auto r = b.getLocalBounds().toFloat();
    auto sw = r.removeFromLeft (30.0f).withSizeKeepingCentre (26.0f, 14.0f);
    g.setColour (b.getToggleState() ? mint : outline.brighter (over ? 0.3f : 0.1f));
    g.fillRoundedRectangle (sw, 7.0f);
    auto knob = sw.withWidth (10.0f).reduced (0.0f, 2.0f).translated (b.getToggleState() ? 14.0f : 2.0f, 0.0f);
    g.setColour (b.getToggleState() ? background : text);
    g.fillEllipse (knob.withSizeKeepingCentre (10.0f, 10.0f));
    g.setColour (text);
    g.setFont (uiFont (12.0f));
    g.drawText (b.getButtonText(), r.withTrimmedLeft (4.0f), juce::Justification::centredLeft);
}

//==============================================================================
MacroKnob::MacroKnob (juce::AudioProcessorValueTreeState& state, const juce::String& paramId, const juce::String& l,
                      juce::Colour a, bool isLarge)
    : label (l), accent (a), large (isLarge)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    slider.setColour (juce::Slider::rotarySliderFillColourId, accent);
    slider.setPopupDisplayEnabled (true, true, nullptr);
    addAndMakeVisible (slider);
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (state, paramId, slider);
}

void MacroKnob::resized()
{
    auto r = getLocalBounds();
    r.removeFromBottom (large ? 18 : 15);
    slider.setBounds (r);
}

void MacroKnob::paint (juce::Graphics& g)
{
    g.setColour (large ? vjui::text : vjui::dim);
    g.setFont (uiFont (large ? 12.0f : 10.5f, true));
    g.drawText (label.toUpperCase(), getLocalBounds().removeFromBottom (large ? 18 : 15), juce::Justification::centred);
}

//==============================================================================
void ColourSwatch::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    auto chip = r.removeFromTop (r.getHeight() - 15.0f).reduced (4.0f, 2.0f);
    g.setColour (colour);
    g.fillRoundedRectangle (chip, 6.0f);
    g.setColour (isMouseOver() ? vjui::text : vjui::outline.brighter (0.4f));
    g.drawRoundedRectangle (chip, 6.0f, 1.2f);
    g.setColour (vjui::dim);
    g.setFont (uiFont (10.0f, true));
    g.drawText (label, r, juce::Justification::centred);
}

namespace
{
    // Colour picker shown in a call-out; every change goes straight to the processor.
    class ColourPicker : public juce::Component, private juce::ChangeListener
    {
    public:
        ColourPicker (juce::Colour start, std::function<void (juce::Colour)> onChangeIn)
            : onChange (std::move (onChangeIn))
        {
            selector.setCurrentColour (start, juce::dontSendNotification);
            selector.addChangeListener (this);
            addAndMakeVisible (selector);
            setSize (300, 320);
        }
        void resized() override { selector.setBounds (getLocalBounds()); }

    private:
        void changeListenerCallback (juce::ChangeBroadcaster*) override { onChange (selector.getCurrentColour()); }
        juce::ColourSelector selector { juce::ColourSelector::showColourAtTop | juce::ColourSelector::showSliders
                                        | juce::ColourSelector::showColourspace };
        std::function<void (juce::Colour)> onChange;
    };
}

//==============================================================================
VJAnalyzerEditor::VJAnalyzerEditor (VJAnalyzerProcessor& p)
    : AudioProcessorEditor (p), processor (p),
      sensitivity (p.getState(), "sensitivity", "Hit Sens", vjui::amber, false),
      trim (p.getState(), "trim", "Trim", vjui::dim, false)
{
    setLookAndFeel (&lookAndFeel);
    auto& state = processor.getState();

    for (int i = 0; i < VJAnalyzerProcessor::roleNames.size(); ++i)
    {
        auto* b = roleButtons.add (new juce::TextButton (VJAnalyzerProcessor::roleNames[i].toUpperCase()));
        b->setColour (juce::TextButton::buttonOnColourId, vjui::mint);
        b->setClickingTogglesState (false);
        b->onClick = [this, i] { setParameter ("role", (float) i); };
        b->setTooltip ("What is on THIS track - the analysis role of this instance");
        addAndMakeVisible (b);
    }

    const juce::Colour accents[] = { vjui::magenta, vjui::mint, vjui::violet, vjui::amber };
    for (int i = 0; i < 8; ++i)
        addAndMakeVisible (macros.add (new MacroKnob (state, "macro" + juce::String (i + 1), VJAnalyzerProcessor::macroNames[i],
                                                      i < 4 ? accents[i] : vjui::dim.brighter (0.3f), i < 4)));

    addAndMakeVisible (sensitivity);
    addAndMakeVisible (trim);

    response.addItemList ({ "Adaptive", "Locked" }, 1);
    addAndMakeVisible (response);
    responseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (state, "normalizer", response);

    addAndMakeVisible (sendControls);
    sendAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (state, "sendControls", sendControls);

    blackout.setClickingTogglesState (true);
    blackout.setColour (juce::TextButton::buttonOnColourId, vjui::danger);
    addAndMakeVisible (blackout);
    blackoutAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (state, "blackout", blackout);

    hit.setColour (juce::TextButton::buttonOnColourId, vjui::magenta);
    hit.onClick = [this] { processor.getWorker().sendUserTrigger(); hitFlash = now(); };
    addAndMakeVisible (hit);

    previous.onClick = [this] { selectPreset (juce::jmax (0, status.presetIndex - 1)); };
    next.onClick = [this] { selectPreset (status.presetIndex + 1); };
    addAndMakeVisible (previous);
    addAndMakeVisible (next);

    engineButton.setColour (juce::TextButton::buttonOnColourId, vjui::mint);
    engineButton.onClick = [this] { openEngine(); };
    addAndMakeVisible (engineButton);

    fullscreenButton.setColour (juce::TextButton::buttonOnColourId, vjui::violet);
    fullscreenButton.onClick = [this] { processor.getWorker().toggleEngineFullscreen(); };
    addAndMakeVisible (fullscreenButton);

    const juce::Colour reactAccents[] = { vjui::magenta, vjui::violet, vjui::mint, vjui::amber, vjui::text };
    for (int i = 0; i < VJAnalyzerProcessor::reactIds.size(); ++i)
    {
        auto* b = reactButtons.add (new juce::TextButton (VJAnalyzerProcessor::reactNames[i].toUpperCase()));
        b->setClickingTogglesState (true);
        b->setColour (juce::TextButton::buttonOnColourId, reactAccents[i]);
        b->setTooltip ("Visuals react to " + VJAnalyzerProcessor::reactNames[i].toLowerCase() + " (global - sent by the instance with Send macros on)");
        addAndMakeVisible (b);
        reactAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
            state, VJAnalyzerProcessor::reactIds[i], *b));
    }

    // LOOK row (image treatment), FILM row (35mm physics), then Reactivity.
    struct KnobDef { int slot; juce::Colour accent; };
    const KnobDef lookRow[] = { { 0, vjui::amber }, { 1, vjui::magenta }, { 7, vjui::violet }, { 3, vjui::violet },
                                { 4, vjui::mint }, { 5, vjui::text }, { 2, vjui::danger }, { 6, vjui::amber } };
    const KnobDef filmRow[] = { { 8, vjui::danger }, { 9, vjui::amber }, { 10, vjui::text }, { 11, vjui::dim.brighter (0.4f) },
                                { 12, vjui::mint } };
    for (auto& k : lookRow)
        addAndMakeVisible (lookKnobs.add (new MacroKnob (state, VJAnalyzerProcessor::lookIds[k.slot], VJAnalyzerProcessor::lookNames[k.slot], k.accent, false)));
    for (auto& k : filmRow)
        addAndMakeVisible (filmKnobs.add (new MacroKnob (state, VJAnalyzerProcessor::lookIds[k.slot], VJAnalyzerProcessor::lookNames[k.slot], k.accent, false)));

    calm.setClickingTogglesState (true);
    calm.setColour (juce::TextButton::buttonOnColourId, vjui::mint);
    calm.setTooltip ("Fades every audio reaction out over one bar (and back in) - for breakdowns");
    addAndMakeVisible (calm);
    calmAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (state, "calm", calm);

    paletteBox.addItemList (VJAnalyzerProcessor::paletteNames, 1);
    addAndMakeVisible (paletteBox);
    paletteAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (state, "palette", paletteBox);

    const char* swatchNames[] = { "SHADOW", "MID", "LIGHT" };
    for (int i = 0; i < 3; ++i)
    {
        auto* s = swatches.add (new ColourSwatch (swatchNames[i]));
        s->onClick = [this, i] { editColour (i); };
        addAndMakeVisible (s);
    }

    presetList.setModel (this);
    presetList.setRowHeight (24);
    presetList.setColour (juce::ListBox::outlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible (presetList);

    setSize (1060, 700);
    startTimerHz (30);
}

VJAnalyzerEditor::~VJAnalyzerEditor()
{
    stopTimer();
    presetList.setModel (nullptr);
    setLookAndFeel (nullptr);
}

void VJAnalyzerEditor::setParameter (const juce::String& id, float plainValue)
{
    if (auto* param = processor.getState().getParameter (id))
    {
        param->beginChangeGesture();
        param->setValueNotifyingHost (param->convertTo0to1 (plainValue));
        param->endChangeGesture();
    }
}

void VJAnalyzerEditor::openEngine()
{
    // Already running: just bring the window forward.
    if (EngineLauncher::bringEngineToFront())
        return;

    juce::File exe (processor.getState().state.getProperty ("enginePath").toString());
    if (exe.existsAsFile())
    {
        launchEngineAt (exe);
        return;
    }

    // Not where this checkout builds it: let the user point at VJ Engine.exe once.
    engineChooser = std::make_unique<juce::FileChooser> ("Locate VJ Engine.exe", exe.getParentDirectory(), "*.exe");
    engineChooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                                [this] (const juce::FileChooser& chooser) {
                                    auto picked = chooser.getResult();
                                    if (picked.existsAsFile())
                                    {
                                        processor.getState().state.setProperty ("enginePath", picked.getFullPathName(), nullptr);
                                        launchEngineAt (picked);
                                    }
                                });
}

void VJAnalyzerEditor::launchEngineAt (const juce::File& exe)
{
    if (EngineLauncher::launchEngine (exe.getFullPathName().toRawUTF8()))
        launchedAt = now();
}

void VJAnalyzerEditor::editColour (int index)
{
    auto start = processor.getPaletteColours()[(size_t) index];
    auto picker = std::make_unique<ColourPicker> (start, [this, index] (juce::Colour c) {
        processor.setCustomColour (index, c);
    });
    juce::CallOutBox::launchAsynchronously (std::move (picker), swatches[index]->getScreenBounds(), nullptr);
}

void VJAnalyzerEditor::selectPreset (int engineIndex)
{
    if (status.numPresets > 0)
        engineIndex = juce::jlimit (0, status.numPresets - 1, engineIndex);

    // Through the host parameter (so Live records/recalls it) and immediately.
    setParameter ("preset", (float) (engineIndex + 1));
    processor.getWorker().selectPresetNow (engineIndex);
}

void VJAnalyzerEditor::timerCallback()
{
    meters = processor.getWorker().getMeters();
    auto newStatus = processor.getWorker().getEngineStatus();
    const bool listChanged = newStatus.presetNames != status.presetNames || newStatus.presetIndex != status.presetIndex;
    status = newStatus;

    if (listChanged)
    {
        presetList.updateContent();
        presetList.repaint();
        if (juce::isPositiveAndBelow (status.presetIndex, status.presetNames.size()))
            presetList.scrollToEnsureRowIsOnscreen (status.presetIndex);
    }

    // Engine button reflects reality: launching... / show / open.
    auto starting = ! status.connected && now() - launchedAt < 8.0;
    engineButton.setButtonText (status.connected ? "SHOW ENGINE" : (starting ? "STARTING..." : "OPEN ENGINE"));
    engineButton.setToggleState (! status.connected && ! starting, juce::dontSendNotification);
    fullscreenButton.setEnabled (status.connected);

    auto role = (int) processor.getState().getRawParameterValue ("role")->load();
    for (int i = 0; i < roleButtons.size(); ++i)
        roleButtons[i]->setToggleState (i == role, juce::dontSendNotification);

    // Macros, LOOK and REACT TO only reach the engine from the instance with
    // "Send macros" on - everywhere else they are shown dimmed.
    const bool sending = processor.getState().getRawParameterValue ("sendControls")->load() > 0.5f;
    const auto alpha = sending ? 1.0f : 0.35f;
    for (auto* c : macros)       c->setAlpha (alpha);
    for (auto* c : lookKnobs)    c->setAlpha (alpha);
    for (auto* c : filmKnobs)    c->setAlpha (alpha);
    calm.setAlpha (alpha);
    for (auto* c : reactButtons) c->setAlpha (alpha);
    for (auto* c : swatches)     c->setAlpha (alpha);
    paletteBox.setAlpha (alpha);

    auto colours = processor.getPaletteColours();
    for (int i = 0; i < swatches.size(); ++i)
        swatches[i]->setColour (colours[(size_t) i]);

    repaint (headerArea);
    repaint (signalArea);
    repaint (scenesArea.withHeight (70));
}

int VJAnalyzerEditor::getNumRows() { return status.presetNames.size(); }

void VJAnalyzerEditor::paintListBoxItem (int row, juce::Graphics& g, int width, int height, bool)
{
    auto current = row == status.presetIndex;
    auto r = juce::Rectangle<int> (0, 0, width, height).reduced (2, 1);
    if (current)
    {
        g.setColour (vjui::mint.withAlpha (0.14f));
        g.fillRoundedRectangle (r.toFloat(), 5.0f);
        g.setColour (vjui::mint);
        g.fillRoundedRectangle (r.removeFromLeft (3).toFloat(), 1.5f);
    }
    g.setColour (current ? vjui::text : vjui::dim.brighter (0.2f));
    g.setFont (uiFont (12.5f, current));
    g.drawText (status.presetNames[row], juce::Rectangle<int> (12, 0, width - 16, height), juce::Justification::centredLeft);
}

void VJAnalyzerEditor::listBoxItemClicked (int row, const juce::MouseEvent&)
{
    selectPreset (row);
}

void VJAnalyzerEditor::paintHeader (juce::Graphics& g, juce::Rectangle<int> r)
{
    g.setColour (vjui::text);
    g.setFont (uiFont (18.0f, true));
    g.drawText ("VJ ANALYZER", r.removeFromLeft (150), juce::Justification::centredLeft);

    // Engine connection pill on the right.
    auto pill = r.removeFromRight (300).reduced (0, 8);
    g.setColour (vjui::panel);
    g.fillRoundedRectangle (pill.toFloat(), pill.getHeight() * 0.5f);
    auto dot = pill.removeFromLeft (pill.getHeight()).toFloat().reduced (9.0f);
    auto colour = status.connected ? (status.blackout ? vjui::danger : vjui::mint) : vjui::dim;
    if (status.connected)
    {
        g.setColour (colour.withAlpha (0.3f));
        g.fillEllipse (dot.expanded (3.0f));
    }
    g.setColour (colour);
    g.fillEllipse (dot);

    juce::String text = status.connected
        ? (status.blackout ? "BLACKOUT" : "ENGINE") + juce::String ("   ") + juce::String (juce::roundToInt (status.fps)) + " fps   "
              + juce::String (status.bpm, 1) + " BPM" + (status.followingHost ? "" : " free")
              + (status.renderScale < 0.99f ? "   " + juce::String (juce::roundToInt (status.renderScale * 100.0f)) + "%" : juce::String())
        : "NO ENGINE - start VJ Engine";
    g.setColour (status.connected ? vjui::text : vjui::dim);
    g.setFont (uiFont (12.0f, true));
    g.drawText (text, pill.withTrimmedRight (12), juce::Justification::centredLeft);
}

void VJAnalyzerEditor::paintSignalPanel (juce::Graphics& g, juce::Rectangle<int> area)
{
    drawPanel (g, area, "SIGNAL");
    auto r = area.reduced (14).withTrimmedTop (20);
    auto& f = meters.frame;

    // 32-band spectrum.
    auto spec = r.removeFromTop (96).toFloat();
    g.setColour (vjui::background);
    g.fillRoundedRectangle (spec, 6.0f);
    auto barW = spec.getWidth() / 32.0f;
    for (int b = 0; b < 32; ++b)
    {
        auto v = juce::jlimit (0.0f, 1.0f, f.spectrum[(size_t) b]);
        auto bar = juce::Rectangle<float> (spec.getX() + b * barW + 1.0f, spec.getBottom() - v * (spec.getHeight() - 6.0f) - 3.0f,
                                           barW - 2.0f, v * (spec.getHeight() - 6.0f));
        g.setColour (vjui::violet.interpolatedWith (vjui::mint, b / 31.0f).withAlpha (0.35f + 0.65f * v));
        g.fillRoundedRectangle (bar, 1.5f);
    }
    r.removeFromTop (10);

    // Level: relative (bold) with the absolute level as a thin marker.
    auto level = r.removeFromTop (14).toFloat();
    g.setColour (vjui::background);
    g.fillRoundedRectangle (level, 4.0f);
    g.setColour (vjui::mint);
    g.fillRoundedRectangle (level.withWidth (level.getWidth() * juce::jlimit (0.0f, 1.0f, f.levelRel)), 4.0f);
    g.setColour (vjui::text);
    g.fillRect (level.getX() + level.getWidth() * juce::jlimit (0.0f, 1.0f, f.levelAbs) - 1.0f, level.getY(), 2.0f, level.getHeight());
    r.removeFromTop (4);
    g.setColour (vjui::dim);
    g.setFont (uiFont (10.5f));
    g.drawText ("LEVEL  " + (f.levelDb > -150.0f ? juce::String (f.levelDb, 1) + " dBFS" : juce::String ("-inf")),
                r.removeFromTop (14), juce::Justification::centredLeft);
    r.removeFromTop (8);

    // Band activity bars: bass / mid / high.
    const char* names[] = { "BASS", "MID", "HIGH" };
    const juce::Colour colours[] = { vjui::magenta, vjui::violet, vjui::mint };
    for (int i = 0; i < 3; ++i)
    {
        auto row = r.removeFromTop (16);
        g.setColour (vjui::dim);
        g.setFont (uiFont (10.0f, true));
        g.drawText (names[i], row.removeFromLeft (38), juce::Justification::centredLeft);
        auto bar = row.toFloat().reduced (0.0f, 4.0f);
        g.setColour (vjui::background);
        g.fillRoundedRectangle (bar, 3.0f);
        g.setColour (colours[i]);
        g.fillRoundedRectangle (bar.withWidth (bar.getWidth() * juce::jlimit (0.0f, 1.0f, f.aggregateRel[(size_t) i])), 3.0f);
    }
    r.removeFromTop (8);

    // Hit lamps.
    auto lamps = r.removeFromTop (46);
    auto lampW = lamps.getWidth() / 4;
    drawLamp (g, lamps.removeFromLeft (lampW).toFloat(), meters.lastOnset[0], vjui::magenta, "KICK/LOW");
    drawLamp (g, lamps.removeFromLeft (lampW).toFloat(), meters.lastOnset[1], vjui::violet, "MID");
    drawLamp (g, lamps.removeFromLeft (lampW).toFloat(), meters.lastOnset[2], vjui::mint, "HIGH");
    drawLamp (g, lamps.toFloat(), meters.lastMidi, vjui::amber, "MIDI");

    // Status line.
    auto& t = meters.transport;
    juce::String line = juce::String (f.gateOpen ? (f.calibrated ? "LIVE" : "CALIBRATING") : "SILENT");
    if (t.valid)
    {
        auto beat = (int) std::floor (t.ppqPosition);
        auto barLen = juce::jmax (1, t.meterNumerator * 4 / juce::jmax (1, t.meterDenominator));
        line << "   " << juce::String (t.bpm, 1) << " BPM   " << (beat / barLen + 1) << "." << (beat % barLen + 1)
             << (t.playing ? "" : "  (stopped)");
    }
    if (meters.overflowed)
        line << "   gap";
    g.setColour (f.gateOpen ? vjui::text : vjui::dim);
    g.setFont (uiFont (11.0f, true));
    g.drawText (line, r.removeFromBottom (16), juce::Justification::centredLeft);
}

void VJAnalyzerEditor::paint (juce::Graphics& g)
{
    g.fillAll (vjui::background);
    paintHeader (g, headerArea);
    paintSignalPanel (g, signalArea);
    drawPanel (g, performArea, "PERFORM");
    drawPanel (g, scenesArea, "SCENES");
    drawPanel (g, lookArea, "LOOK");
    {
        g.setColour (vjui::dim);
        g.setFont (uiFont (10.5f, true));
        g.drawText ("FILM", filmCaption, juce::Justification::centredLeft);
        g.drawText ("REACTION", reactionCaption, juce::Justification::centredLeft);
        g.drawText ("PALETTE", paletteCaption, juce::Justification::centredLeft);
        g.setColour (vjui::outline);
        g.fillRect (lookDivider);
    }

    g.setColour (vjui::dim);
    g.setFont (uiFont (10.5f, true));
    g.drawText ("VISUALS REACT TO", reactArea.withHeight (14), juce::Justification::centredLeft);

    // Current scene name, large.
    auto nameArea = scenesArea.reduced (14).withTrimmedTop (18).removeFromTop (30);
    g.setColour (status.connected ? vjui::text : vjui::dim);
    g.setFont (uiFont (19.0f, true));
    g.drawText (status.connected ? status.presetName : juce::String ("-"), nameArea, juce::Justification::centredLeft, true);

    if (now() - hitFlash < 0.15)
    {
        g.setColour (vjui::magenta.withAlpha (0.3f));
        g.fillRoundedRectangle (hit.getBounds().toFloat().expanded (3.0f), 8.0f);
    }
}

void VJAnalyzerEditor::resized()
{
    auto r = getLocalBounds().reduced (12);
    headerArea = r.removeFromTop (44);

    // Engine buttons just left of the status pill; role selector in the remaining centre.
    auto engineArea = headerArea.withTrimmedRight (306).removeFromRight (216).withSizeKeepingCentre (216, 30);
    engineButton.setBounds (engineArea.removeFromLeft (112));
    fullscreenButton.setBounds (engineArea.withTrimmedLeft (6));
    auto roles = headerArea.withTrimmedLeft (150).withTrimmedRight (530).withSizeKeepingCentre (324, 28);
    auto roleW = roles.getWidth() / roleButtons.size();
    for (auto* b : roleButtons)
        b->setBounds (roles.removeFromLeft (roleW).reduced (2, 0));

    r.removeFromTop (8);
    lookArea = r.removeFromBottom (228);
    r.removeFromBottom (10);
    signalArea = r.removeFromLeft (280);
    reactArea = signalArea.reduced (14).removeFromBottom (16 + 6 + 42).removeFromTop (42);
    {
        auto chips = reactArea.withTrimmedTop (16);
        auto chipW = chips.getWidth() / reactButtons.size();
        for (auto* b : reactButtons)
            b->setBounds (chips.removeFromLeft (chipW).reduced (2, 0));
    }
    r.removeFromLeft (10);
    scenesArea = r.removeFromRight (270);
    r.removeFromRight (10);
    performArea = r;

    // Perform: 4 large macros in a 2x2 grid, 4 small below, then response controls.
    auto p = performArea.reduced (14).withTrimmedTop (20);
    auto bottom = p.removeFromBottom (34);
    auto small = p.removeFromBottom (78);
    auto cellW = p.getWidth() / 2, cellH = p.getHeight() / 2;
    for (int i = 0; i < 4; ++i)
        macros[i]->setBounds (p.getX() + (i % 2) * cellW, p.getY() + (i / 2) * cellH, cellW, cellH);
    auto smallW = small.getWidth() / 4;
    for (int i = 4; i < 8; ++i)
        macros[i]->setBounds (small.removeFromLeft (smallW).reduced (4, 0));
    response.setBounds (bottom.removeFromLeft (110).reduced (0, 4));
    bottom.removeFromLeft (10);
    sendControls.setBounds (bottom.reduced (0, 4));

    // Look: row 1 = image treatment (8 knobs); row 2 = FILM (4) | REACTION
    // (Reactivity + CALM) | PALETTE (preset + three colour chips).
    auto l = lookArea.reduced (14).withTrimmedTop (20);
    auto row1 = l.removeFromTop (l.getHeight() / 2 - 4);
    l.removeFromTop (8);
    auto row2 = l;
    auto knobW = row1.getWidth() / lookKnobs.size();
    for (auto* k : lookKnobs)
        k->setBounds (row1.removeFromLeft (knobW).reduced (6, 0));
    lookDivider = juce::Rectangle<int> (lookArea.getX() + 14, row2.getY() - 5, lookArea.getWidth() - 28, 1);

    const int filmW = 96;
    auto film = row2.removeFromLeft (filmW * 4);
    filmCaption = film.removeFromTop (14);
    for (int i = 0; i < 4; ++i)
        filmKnobs[i]->setBounds (film.removeFromLeft (filmW).reduced (4, 0));
    row2.removeFromLeft (18);
    auto reaction = row2.removeFromLeft (filmW + 104);
    reactionCaption = reaction.removeFromTop (14);
    filmKnobs[4]->setBounds (reaction.removeFromLeft (filmW).reduced (4, 0));
    calm.setBounds (reaction.withSizeKeepingCentre (92, 34).translated (0, -8));
    row2.removeFromLeft (18);
    paletteCaption = row2.removeFromTop (14);
    paletteBox.setBounds (row2.removeFromLeft (124).withSizeKeepingCentre (124, 28).translated (0, -8));
    row2.removeFromLeft (10);
    auto swatchW = row2.getWidth() / 3;
    for (auto* s : swatches)
        s->setBounds (row2.removeFromLeft (swatchW).reduced (3, 2));

    // Scenes: name (painted), list, transport buttons, hit/blackout, sensitivity/trim.
    auto s = scenesArea.reduced (14).withTrimmedTop (52);
    auto knobs = s.removeFromBottom (74);
    sensitivity.setBounds (knobs.removeFromLeft (knobs.getWidth() / 2));
    trim.setBounds (knobs);
    s.removeFromBottom (8);
    auto actions = s.removeFromBottom (40);
    hit.setBounds (actions.removeFromLeft (actions.getWidth() / 2).reduced (0, 0).withTrimmedRight (4));
    blackout.setBounds (actions.withTrimmedLeft (4));
    s.removeFromBottom (8);
    auto nav = s.removeFromBottom (28);
    previous.setBounds (nav.removeFromLeft (nav.getWidth() / 2).withTrimmedRight (4));
    next.setBounds (nav.withTrimmedLeft (4));
    s.removeFromBottom (8);
    presetList.setBounds (s);
}
