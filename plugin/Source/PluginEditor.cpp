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
    : label (l), accent (a), subColour (vjui::dim), large (isLarge)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    slider.setColour (juce::Slider::rotarySliderFillColourId, accent);
    slider.setPopupDisplayEnabled (true, true, nullptr);
    addAndMakeVisible (slider);
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (state, paramId, slider);
}

void MacroKnob::setSubLabel (const juce::String& text, juce::Colour colour)
{
    if (text != subLabel || colour != subColour)
    {
        subLabel = text;
        subColour = colour;
        repaint();
    }
}

void MacroKnob::resized()
{
    auto r = getLocalBounds();
    r.removeFromBottom ((large ? 18 : 15) + 14);
    slider.setBounds (r);
}

void MacroKnob::paint (juce::Graphics& g)
{
    auto r = getLocalBounds();
    auto sub = r.removeFromBottom (14);
    g.setColour (large ? vjui::text : vjui::text.withAlpha (0.8f));
    g.setFont (uiFont (large ? 12.0f : 10.5f, true));
    g.drawText (label.toUpperCase(), r.removeFromBottom (large ? 18 : 15), juce::Justification::centred);
    if (subLabel.isNotEmpty())
    {
        g.setColour (subColour);
        g.setFont (uiFont (10.5f));
        g.drawText (subLabel, sub, juce::Justification::centredTop, true);
    }
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
      trim (p.getState(), "trim", "Trim", vjui::dim, false),
      lookahead (p.getState(), "lookahead", "Lookahead", vjui::violet, false),
      drift (p.getState(), "drift", "Drift", vjui::mint, false),
      push (p.getState(), "push", "Push", vjui::mint, false),
      softness (p.getState(), "softness", "Softness", vjui::magenta, false),
      reactivity (p.getState(), "reactivity", "Reactivity", vjui::magenta, false)
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

    // Macro slots: SHAPE (0 Intensity, 2 Form, 3 Scale, 5 Erode, 7 Detail),
    // MOVE (1 Speed, 6 Glide), REACT (4 Impact).
    const juce::Colour macroAccents[] = { vjui::amber, vjui::mint, vjui::violet, vjui::violet,
                                          vjui::magenta, vjui::violet, vjui::mint, vjui::violet };
    const bool macroLarge[] = { true, true, true, true, false, true, false, true };
    const char* macroTips[] = {
        "INTENSITY - light and energy of the scene.",
        "SPEED - how fast everything in the scene moves. 0 = frozen, middle = the designed speed, full = x4. Hits never change speed.",
        "FORM - the scene's character (the line under the knob says what it is here).",
        "SCALE - bigger as you turn it up.",
        "IMPACT - how hard each hit (kick, snare, HIT) lands. 0 = no hit reactions at all.",
        "ERODE - from pristine to worn, torn and dissolved.",
        "GLIDE - inertia: how long speed changes take to arrive (0 - 4 bars).",
        "DETAIL - density and fineness." };
    for (int i = 0; i < 8; ++i)
    {
        auto* k = macros.add (new MacroKnob (state, "macro" + juce::String (i + 1), VJAnalyzerProcessor::macroNames[i],
                                             macroAccents[i], macroLarge[i]));
        k->setTooltip (macroTips[i]);
        addAndMakeVisible (k);
    }

    drift.setTooltip ("DRIFT - a slow camera over the whole picture (push-in, turn, pan). Stops with Speed 0 / Freeze.");
    push.setTooltip ("PUSH - how much the music speeds motion up (loud passages up to x2). Never moves a frozen scene.");
    softness.setTooltip ("SOFTNESS - how long every hit reaction lingers (decays x0.5 - x4).");
    reactivity.setTooltip ("REACTIVITY - how much the picture follows the sound at all (1 = as designed, 0 = only knobs and LFOs).");
    sensitivity.setTooltip ("How easily hits are detected (more or fewer hits).");
    trim.setTooltip ("Input level for the analysis only - the audio is never changed.");
    lookahead.setTooltip ("VISUAL LOOKAHEAD - delays the sound by this much (Live compensates) so the visuals can react early and land "
                          "exactly with the music. Only for playback / DJ sets: live instruments through Live would be late too. 0 = off.");
    for (auto* k : { &drift, &push, &softness, &reactivity, &sensitivity, &trim, &lookahead })
        addAndMakeVisible (k);

    struct ToggleDef { juce::TextButton* button; const char* id; juce::Colour colour; const char* tip; };
    const ToggleDef toggles[] = {
        { &freeze, "freeze", vjui::text, "Stops all motion (the film grain keeps running). Hits still land unless Impact is 0." },
        { &reverse, "reverse", vjui::violet, "Runs all motion backwards." },
        { &sync, "sync", vjui::amber, "Speeds follow the tempo (the designed speed at 120 BPM)." } };
    for (auto& t : toggles)
    {
        t.button->setClickingTogglesState (true);
        t.button->setColour (juce::TextButton::buttonOnColourId, t.colour);
        t.button->setTooltip (t.tip);
        addAndMakeVisible (t.button);
        moveAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (state, t.id, *t.button));
    }

    response.addItemList ({ "Adaptive", "Locked" }, 1);
    response.setTooltip ("Adaptive: follows each song's loudness. Locked: stays put after calibration.");
    addAndMakeVisible (response);
    responseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (state, "normalizer", response);

    sendControls.setTooltip ("Only ONE instance should send the knobs (normally the one on the master).");
    addAndMakeVisible (sendControls);
    sendAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (state, "sendControls", sendControls);

    blackout.setClickingTogglesState (true);
    blackout.setColour (juce::TextButton::buttonOnColourId, vjui::danger);
    blackout.setTooltip ("Fades the output to black");
    addAndMakeVisible (blackout);
    blackoutAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (state, "blackout", blackout);

    // HIT / scene -/+ go through their host parameters, so MIDI-mapped
    // buttons and these clicks do exactly the same thing.
    auto press = [this] (const juce::String& id) { setParameter (id, 1.0f); };
    hit.setColour (juce::TextButton::buttonOnColourId, vjui::magenta);
    hit.setTooltip ("A manual hit (acts like a kick). MIDI-mappable: parameter 'Hit'.");
    hit.onClick = [this, press] { press ("hit"); hitFlash = now(); };
    addAndMakeVisible (hit);

    previous.onClick = [press] { press ("scenePrev"); };
    next.onClick = [press] { press ("sceneNext"); };
    previous.setTooltip ("Previous scene (parameter 'Previous Scene')");
    next.setTooltip ("Next scene (parameter 'Next Scene')");
    addAndMakeVisible (previous);
    addAndMakeVisible (next);

    engineButton.setColour (juce::TextButton::buttonOnColourId, vjui::mint);
    engineButton.onClick = [this] { openEngine(); };
    addAndMakeVisible (engineButton);

    fullscreenButton.setColour (juce::TextButton::buttonOnColourId, vjui::violet);
    fullscreenButton.onClick = [this] { processor.getWorker().toggleEngineFullscreen(); };
    addAndMakeVisible (fullscreenButton);

    for (int i = 0; i < VJAnalyzerProcessor::numSnapshots; ++i)
    {
        auto* b = snapshotButtons.add (new juce::TextButton (juce::String::charToString ((juce::juce_wchar) ('A' + i))));
        b->setColour (juce::TextButton::buttonOnColourId, vjui::violet);
        b->setTooltip ("Recall snapshot (morphs over the selected time; MIDI-mappable as 'Snapshot "
                       + juce::String::charToString ((juce::juce_wchar) ('A' + i))
                       + "'). With STORE lit: save the current knobs, LOOK and palette here.");
        b->onClick = [this, i] {
            if (storeButton.getToggleState())
            {
                processor.storeSnapshot (i);
                storeButton.setToggleState (false, juce::dontSendNotification);
            }
            else
                setParameter ("snap" + juce::String (i + 1), 1.0f);
        };
        addAndMakeVisible (b);
    }
    storeButton.setClickingTogglesState (true);
    storeButton.setColour (juce::TextButton::buttonOnColourId, vjui::amber);
    storeButton.setTooltip ("Arm, then click A-D to store the current state there");
    addAndMakeVisible (storeButton);
    morphBox.addItemList (VJAnalyzerProcessor::morphNames, 1);
    morphBox.setTooltip ("How long a recalled snapshot takes to morph in");
    addAndMakeVisible (morphBox);
    morphAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (state, "morphTime", morphBox);

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

    // LOOK: FILM (35mm physics) and DIGITAL (disturbances), by look slot.
    struct KnobDef { int slot; juce::Colour accent; const char* tip; };
    const KnobDef filmRow[] = {
        { 0, vjui::amber, "Film grain, 24 fps, following the image's brightness" },
        { 8, vjui::danger, "Red glow bleeding out of the highlights" },
        { 9, vjui::amber, "Gate weave, rare frame slips, flicker, soft colour fringes" },
        { 10, vjui::text, "Dust specks, hairs and scratches - rare and correlated" },
        { 11, vjui::dim.brighter (0.4f), "Lifted, tinted blacks (the film base)" } };
    const KnobDef digitalRow[] = {
        { 1, vjui::magenta, "Soft S-curve -> posterised -> hard two-tone" },
        { 4, vjui::mint, "Frame memory: motion leaves a trail" },
        { 3, vjui::violet, "Row tears, pixel blocks, RGB split (pulsed by hits)" },
        { 7, vjui::violet, "VHS tracking drift on rows, in bursts" },
        { 5, vjui::text, "Braille symbol strips, re-dealt on snares" },
        { 2, vjui::danger, "Chance a kick drops a black / colour frame (max 3 per second)" },
        { 6, vjui::amber, "Automatic cuts between scenes: off, every 16/8/4/2/1 beats, every kick" } };
    for (auto& k : filmRow)
    {
        auto* knob = filmKnobs.add (new MacroKnob (state, VJAnalyzerProcessor::lookIds[k.slot], VJAnalyzerProcessor::lookNames[k.slot], k.accent, false));
        knob->setTooltip (k.tip);
        addAndMakeVisible (knob);
    }
    for (auto& k : digitalRow)
    {
        auto* knob = digitalKnobs.add (new MacroKnob (state, VJAnalyzerProcessor::lookIds[k.slot], VJAnalyzerProcessor::lookNames[k.slot], k.accent, false));
        knob->setTooltip (k.tip);
        addAndMakeVisible (knob);
    }
    {
        auto* shots = digitalKnobs.add (new MacroKnob (state, "shots", "Shots", vjui::amber, false));
        shots->setTooltip ("SHOTS - some strong hits become short cuts: a new framing held until the next one, a few frames of negative, "
                           "a circle / crescent / lens punch, a black frame. More with the knob; max 3 a second.");
        addAndMakeVisible (shots);
        auto* hud = digitalKnobs.add (new MacroKnob (state, "hud", "HUD", vjui::text, false));
        hud->setTooltip ("HUD - a measuring-instrument overlay: crosshairs, small squares linked by hairlines, big faint circles. Re-dealt on every shot.");
        addAndMakeVisible (hud);
    }

    calm.setClickingTogglesState (true);
    calm.setColour (juce::TextButton::buttonOnColourId, vjui::mint);
    calm.setTooltip ("Fades every audio reaction out over one bar (and back in) - for breakdowns");
    addAndMakeVisible (calm);
    calmAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (state, "calm", calm);

    paletteBox.addItemList (VJAnalyzerProcessor::paletteNames, 1);
    paletteBox.setTooltip ("3-colour palette for every scene (Split = red/cyan for Negative; Scene Colors = off)");
    addAndMakeVisible (paletteBox);
    paletteAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (state, "palette", paletteBox);

    const char* swatchNames[] = { "SHADOW", "MID", "LIGHT" };
    for (int i = 0; i < 3; ++i)
    {
        auto* s = swatches.add (new ColourSwatch (swatchNames[i]));
        s->onClick = [this, i] { editColour (i); };
        addAndMakeVisible (s);
    }

    mediaLoad.setColour (juce::TextButton::buttonOnColourId, vjui::mint);
    mediaLoad.setTooltip ("Load your own image (PNG / JPEG) or a short video clip (MP4 / MOV ...). The Media scenes use it; USE MEDIA puts it "
                          "into every scene that works on images. Clips play on the scene clock: Speed 0 freezes them, Reverse plays them backwards, "
                          "SHOTS cuts them to another section.");
    mediaLoad.onClick = [this] { loadMedia(); };
    addAndMakeVisible (mediaLoad);
    mediaClear.setTooltip ("Remove the image (scenes go back to their built-in forms)");
    mediaClear.onClick = [this] { processor.setMediaPath ({}); };
    addAndMakeVisible (mediaClear);
    useMedia.setClickingTogglesState (true);
    useMedia.setColour (juce::TextButton::buttonOnColourId, vjui::mint);
    useMedia.setTooltip ("Feed your image to Dot Relief, One Bit, Emergence and every other scene that works on images");
    addAndMakeVisible (useMedia);
    useMediaAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (state, "useMedia", useMedia);

    presetList.setModel (this);
    presetList.setRowHeight (24);
    presetList.setColour (juce::ListBox::outlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible (presetList);

    setSize (1180, 800);
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

void VJAnalyzerEditor::loadMedia()
{
    juce::File start (processor.getMediaPath());
    mediaChooser = std::make_unique<juce::FileChooser> ("Load an image or a short clip", start.existsAsFile() ? start.getParentDirectory() : juce::File(),
                                                        "*.png;*.jpg;*.jpeg;*.mp4;*.mov;*.m4v;*.avi;*.wmv;*.mkv;*.webm");
    mediaChooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                               [this] (const juce::FileChooser& chooser) {
                                   auto picked = chooser.getResult();
                                   if (picked.existsAsFile())
                                       processor.setMediaPath (picked.getFullPathName());
                               });
}

void VJAnalyzerEditor::selectPreset (int engineIndex)
{
    if (status.numPresets > 0)
        engineIndex = juce::jlimit (0, status.numPresets - 1, engineIndex);

    // Through the host parameter (so Live records/recalls it) and immediately.
    setParameter ("preset", (float) (engineIndex + 1));
    processor.getWorker().selectPresetNow (engineIndex);
}

// The line under each knob: what SHAPE knobs do in the live scene, live
// readouts for Speed / Glide / Push / Softness.
void VJAnalyzerEditor::updateKnobLabels()
{
    auto& state = processor.getState();
    auto paramText = [&state] (const juce::String& id) {
        auto* p = state.getParameter (id);
        return p != nullptr ? p->getCurrentValueAsText() : juce::String();
    };

    const int shapeSlots[] = { 0, 2, 3, 5, 7 };
    for (auto slot : shapeSlots)
    {
        auto sceneName = status.connected ? status.macroLabels[slot] : juce::String();
        if (sceneName.equalsIgnoreCase (VJAnalyzerProcessor::macroNames[slot]))
            sceneName = {};
        macros[slot]->setSubLabel (sceneName, vjui::mint.withAlpha (0.85f));
    }

    // Speed: what the scene clock actually runs at (Speed x Push, Freeze, Reverse).
    juce::String speedText;
    auto speedColour = vjui::dim;
    if (status.connected)
    {
        auto s = status.speed;
        if (std::abs (s) < 0.005f)
        {
            speedText = "FROZEN";
            speedColour = vjui::danger;
        }
        else
        {
            speedText = "x" + juce::String (std::abs (s), 2) + (s < 0.0f ? "  reverse" : "");
            speedColour = vjui::mint;
        }
    }
    else
        speedText = paramText ("macro2");
    macros[1]->setSubLabel (speedText, speedColour);
    macros[6]->setSubLabel (paramText ("macro7"), vjui::dim);
    push.setSubLabel (paramText ("push"), vjui::dim);
    lookahead.setSubLabel (paramText ("lookahead"), vjui::dim);
    softness.setSubLabel (paramText ("softness"), vjui::dim);
    auto impactName = status.connected ? status.macroLabels[4] : juce::String();
    macros[4]->setSubLabel (impactName.equalsIgnoreCase ("impact") ? juce::String() : impactName, vjui::mint.withAlpha (0.85f));
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

    // Knobs, LOOK and REACT TO only reach the engine from the instance with
    // "Send macros" on - everywhere else they are shown dimmed.
    const bool sending = processor.getState().getRawParameterValue ("sendControls")->load() > 0.5f;
    const auto alpha = sending ? 1.0f : 0.35f;
    juce::Array<juce::Component*> global;
    for (juce::Component* c : std::initializer_list<juce::Component*> { &drift, &push, &softness, &reactivity, &calm, &freeze,
                                                                        &reverse, &sync, &paletteBox, &storeButton, &morphBox })
        global.add (c);
    for (auto* c : macros)       global.add (c);
    for (auto* c : filmKnobs)    global.add (c);
    for (auto* c : digitalKnobs) global.add (c);
    for (auto* c : reactButtons) global.add (c);
    for (auto* c : swatches)     global.add (c);
    for (auto* c : global)
        c->setAlpha (alpha);

    for (int i = 0; i < snapshotButtons.size(); ++i)
    {
        auto* b = snapshotButtons[i];
        b->setToggleState (i == processor.getActiveSnapshot(), juce::dontSendNotification);
        b->setAlpha ((processor.hasSnapshot (i) || storeButton.getToggleState() ? 1.0f : 0.4f) * alpha);
    }

    auto colours = processor.getPaletteColours();
    for (int i = 0; i < swatches.size(); ++i)
        swatches[i]->setColour (colours[(size_t) i]);

    updateKnobLabels();

    repaint (headerArea);
    repaint (signalArea);
    repaint (sceneNameArea.getUnion (sceneTextArea));
    repaint (mediaInfoArea);
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
    r.removeFromTop (4);

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
    g.drawText (line, r.removeFromTop (16), juce::Justification::centredLeft);
}

void VJAnalyzerEditor::paint (juce::Graphics& g)
{
    g.fillAll (vjui::background);
    paintHeader (g, headerArea);
    paintSignalPanel (g, signalArea);
    drawPanel (g, shapeArea, "SHAPE  -  what the scene looks like");
    drawPanel (g, moveArea, "MOVE  -  0 = still");
    drawPanel (g, reactArea, "REACT  -  hits");
    drawPanel (g, scenesArea, "SCENES");
    drawPanel (g, lookArea, "LOOK");

    g.setColour (vjui::dim);
    g.setFont (uiFont (10.5f, true));
    g.drawText ("FILM", filmCaption, juce::Justification::centredLeft);
    g.drawText ("DIGITAL", digitalCaption, juce::Justification::centredLeft);
    g.drawText ("PALETTE", paletteCaption, juce::Justification::centredLeft);
    g.drawText ("SNAPSHOTS", snapshotCaption, juce::Justification::centredLeft);
    g.drawText ("VISUALS REACT TO", reactToCaption, juce::Justification::centredLeft);
    g.drawText ("MEDIA", mediaCaption, juce::Justification::centredLeft);

    // What the engine has in the media slot.
    {
        juce::File file (processor.getMediaPath());
        juce::String info = file.getFileName().isEmpty() ? juce::String ("no image or clip - scenes use their built-in forms")
                          : ! status.connected ? file.getFileName()
                          : status.mediaLoading ? file.getFileName() + "  (loading...)"
                          : status.mediaWidth > 0 ? file.getFileName() + "  " + juce::String (status.mediaWidth) + "x" + juce::String (status.mediaHeight)
                          : file.getFileName() + "  (not loaded)";
        g.setColour (file.getFileName().isEmpty() ? vjui::dim : vjui::text);
        g.setFont (uiFont (10.5f));
        g.drawText (info, mediaInfoArea, juce::Justification::centredLeft, true);
    }
    g.setColour (vjui::outline);
    g.fillRect (lookDivider);

    // Current scene name, large, and what it is.
    g.setColour (status.connected ? vjui::text : vjui::dim);
    g.setFont (uiFont (19.0f, true));
    g.drawText (status.connected ? status.presetName : juce::String ("-"), sceneNameArea, juce::Justification::centredLeft, true);
    if (status.connected && status.sceneDescription.isNotEmpty())
    {
        g.setColour (vjui::dim.brighter (0.2f));
        g.setFont (uiFont (11.0f));
        g.drawFittedText (status.sceneDescription, sceneTextArea, juce::Justification::topLeft, 4, 1.0f);
    }

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
    lookArea = r.removeFromBottom (214);
    r.removeFromBottom (10);
    signalArea = r.removeFromLeft (250);
    r.removeFromLeft (10);
    scenesArea = r.removeFromRight (250);
    r.removeFromRight (10);
    shapeArea = r.removeFromTop (236);
    r.removeFromTop (10);
    moveArea = r.removeFromLeft (372);
    r.removeFromLeft (10);
    reactArea = r;

    // SIGNAL: meters are painted; input controls at the bottom.
    {
        auto s = signalArea.reduced (14);
        auto opts = s.removeFromBottom (28);
        response.setBounds (opts.removeFromLeft (100).reduced (0, 2));
        opts.removeFromLeft (8);
        sendControls.setBounds (opts);
        s.removeFromBottom (6);
        auto knobs = s.removeFromBottom (92);
        auto third = knobs.getWidth() / 3;
        sensitivity.setBounds (knobs.removeFromLeft (third));
        trim.setBounds (knobs.removeFromLeft (third));
        lookahead.setBounds (knobs);
    }

    // SHAPE: five knobs in a row.
    {
        auto s = shapeArea.reduced (14).withTrimmedTop (22);
        const int order[] = { 0, 2, 3, 5, 7 };
        auto w = s.getWidth() / 5;
        for (auto slot : order)
            macros[slot]->setBounds (s.removeFromLeft (w).reduced (6, 0));
    }

    // MOVE: Speed large, Glide / Drift / Push, then FREEZE / REVERSE / SYNC.
    {
        auto m = moveArea.reduced (14).withTrimmedTop (22);
        auto buttons = m.removeFromBottom (32);
        m.removeFromBottom (8);
        macros[1]->setBounds (m.removeFromLeft (132));
        m.removeFromLeft (6);
        auto w = m.getWidth() / 3;
        auto row = m.withSizeKeepingCentre (m.getWidth(), juce::jmin (m.getHeight(), 118));
        macros[6]->setBounds (row.removeFromLeft (w).reduced (2, 0));
        drift.setBounds (row.removeFromLeft (w).reduced (2, 0));
        push.setBounds (row.reduced (2, 0));
        auto bw = buttons.getWidth() / 3;
        freeze.setBounds (buttons.removeFromLeft (bw).reduced (3, 0));
        reverse.setBounds (buttons.removeFromLeft (bw).reduced (3, 0));
        sync.setBounds (buttons.reduced (3, 0));
    }

    // REACT: Impact / Softness / Reactivity, CALM, REACT TO chips.
    {
        auto a = reactArea.reduced (14).withTrimmedTop (22);
        auto chips = a.removeFromBottom (28);
        reactToCaption = a.removeFromBottom (16);
        a.removeFromBottom (4);
        calm.setBounds (a.removeFromBottom (30).withSizeKeepingCentre (120, 30));
        a.removeFromBottom (6);
        auto w = a.getWidth() / 3;
        macros[4]->setBounds (a.removeFromLeft (w).reduced (2, 0));
        softness.setBounds (a.removeFromLeft (w).reduced (2, 0));
        reactivity.setBounds (a.reduced (2, 0));
        auto chipW = chips.getWidth() / reactButtons.size();
        for (auto* b : reactButtons)
            b->setBounds (chips.removeFromLeft (chipW).reduced (2, 0));
    }

    // SCENES: name + description (painted), list, < >, HIT / BLACKOUT.
    {
        auto s = scenesArea.reduced (14).withTrimmedTop (20);
        sceneNameArea = s.removeFromTop (28);
        sceneTextArea = s.removeFromTop (58);
        s.removeFromTop (6);
        auto actions = s.removeFromBottom (40);
        hit.setBounds (actions.removeFromLeft (actions.getWidth() / 2).withTrimmedRight (4));
        blackout.setBounds (actions.withTrimmedLeft (4));
        s.removeFromBottom (8);
        auto nav = s.removeFromBottom (28);
        previous.setBounds (nav.removeFromLeft (nav.getWidth() / 2).withTrimmedRight (4));
        next.setBounds (nav.withTrimmedLeft (4));
        s.removeFromBottom (8);
        presetList.setBounds (s);
    }

    // LOOK: row 1 = FILM (5) | DIGITAL (7); row 2 = PALETTE | SNAPSHOTS | media.
    {
        auto l = lookArea.reduced (14).withTrimmedTop (20);
        auto row1 = l.removeFromTop (106);
        l.removeFromTop (8);
        auto row2 = l;
        lookDivider = juce::Rectangle<int> (lookArea.getX() + 14, row2.getY() - 5, lookArea.getWidth() - 28, 1);

        const int knobW = (row1.getWidth() - 24) / (filmKnobs.size() + digitalKnobs.size());
        auto film = row1.removeFromLeft (knobW * 5);
        filmCaption = film.removeFromTop (14);
        for (auto* k : filmKnobs)
            k->setBounds (film.removeFromLeft (knobW).reduced (4, 0));
        row1.removeFromLeft (24);
        digitalCaption = row1.removeFromTop (14);
        for (auto* k : digitalKnobs)
            k->setBounds (row1.removeFromLeft (knobW).reduced (4, 0));

        auto palette = row2.removeFromLeft (330);
        paletteCaption = palette.removeFromTop (14);
        paletteBox.setBounds (palette.removeFromLeft (124).withSizeKeepingCentre (124, 28));
        palette.removeFromLeft (8);
        auto swatchW = palette.getWidth() / 3;
        for (auto* s : swatches)
            s->setBounds (palette.removeFromLeft (swatchW).reduced (3, 0));
        row2.removeFromLeft (24);

        auto snaps = row2.removeFromLeft (380);
        snapshotCaption = snaps.removeFromTop (14);
        snaps = snaps.withSizeKeepingCentre (snaps.getWidth(), 30);
        for (auto* b : snapshotButtons)
            b->setBounds (snaps.removeFromLeft (44).reduced (2, 0));
        snaps.removeFromLeft (8);
        storeButton.setBounds (snaps.removeFromLeft (76).reduced (2, 0));
        snaps.removeFromLeft (8);
        morphBox.setBounds (snaps.removeFromLeft (104).reduced (0, 1));
        row2.removeFromLeft (24);
        mediaArea = row2;
        auto m = mediaArea;
        mediaCaption = m.removeFromTop (14);
        mediaInfoArea = m.removeFromBottom (14);
        auto buttons = m.withSizeKeepingCentre (m.getWidth(), 28);
        mediaLoad.setBounds (buttons.removeFromLeft (buttons.getWidth() * 2 / 5).reduced (2, 0));
        useMedia.setBounds (buttons.removeFromLeft (buttons.getWidth() * 3 / 5).reduced (2, 0));
        mediaClear.setBounds (buttons.reduced (2, 0));
    }
}
