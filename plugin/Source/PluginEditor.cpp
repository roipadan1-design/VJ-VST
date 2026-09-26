#include "PluginEditor.h"
#include "EngineLauncher.h"

namespace
{
    constexpr int playW = 760, editW = 1180, height = 480, sourceW = 360, sourceH = 240;

    double seconds() { return juce::Time::getMillisecondCounterHiRes() * 0.001; }

    void caption (juce::Graphics& g, const juce::String& text, int x, int y, int w = 300)
    {
        g.setColour (vjui::ash);
        g.setFont (vjui::font (9.5f, true));
        g.drawText (text.toUpperCase(), x, y, w, 12, juce::Justification::centredLeft);
    }

    void hint (juce::Graphics& g, const juce::String& text, juce::Rectangle<int> r, juce::Justification j = juce::Justification::centredLeft)
    {
        g.setColour (vjui::dust);
        g.setFont (vjui::font (10.0f));
        g.drawText (text, r, j, true);
    }

    void stripes (juce::Graphics& g, juce::Rectangle<float> r, const std::array<juce::Colour, 3>& c)
    {
        auto w = r.getWidth() / 3.0f;
        for (int i = 0; i < 3; ++i)
        {
            g.setColour (c[(size_t) i]);
            g.fillRect (r.withX (r.getX() + w * (float) i).withWidth (w));
        }
        g.setColour (vjui::line);
        g.drawRect (r, 1.0f);
    }

    void outlineOf (juce::Component& c, juce::Colour colour) { c.getProperties().set ("outline", (int) colour.getARGB()); }

    void styleButton (juce::TextButton& b, juce::Colour fill, juce::Colour text, juce::Colour onFill, juce::Colour onText)
    {
        b.setColour (juce::TextButton::buttonColourId, fill);
        b.setColour (juce::TextButton::textColourOffId, text);
        b.setColour (juce::TextButton::buttonOnColourId, onFill);
        b.setColour (juce::TextButton::textColourOnId, onText);
    }

    // The value in a knob's centre, in the parameter's own words.
    juce::String knobText (juce::Slider& s)
    {
        const auto v = s.getValue();
        if ((bool) s.getProperties().getWithDefault ("offAtZero", false) && v <= 0.001)
            return "off";
        auto t = s.getTextFromValue (v).trim();
        if (t.endsWith (" %"))
            return t.upToLastOccurrenceOf (" %", false, false) + "%";
        if (t.containsOnly ("0123456789.-") && s.getMaximum() <= 1.0 && s.getMinimum() >= 0.0)
            return juce::String (juce::roundToInt (v * 100.0)) + "%";
        if (t.length() <= 6)
            return t;
        // Keep the part that carries the number: "0.25 bars" -> "0.25", "decay x1.4" -> "x1.4".
        for (auto& token : juce::StringArray::fromTokens (t, " ", ""))
            if (token.containsAnyOf ("0123456789"))
                return token;
        return t.substring (0, 6);
    }

    // Palette row in the COLOUR menu: three stripes and the name.
    class PaletteItem : public juce::PopupMenu::CustomComponent
    {
    public:
        PaletteItem (std::array<juce::Colour, 3> c, juce::String n, bool isTicked) : colours (c), name (std::move (n)), ticked (isTicked) {}
        void getIdealSize (int& w, int& h) override { w = 190; h = 24; }
        void paint (juce::Graphics& g) override
        {
            if (isItemHighlighted())
                g.fillAll (vjui::ink2);
            stripes (g, juce::Rectangle<float> (10.0f, 7.0f, 36.0f, 10.0f), colours);
            g.setColour (ticked ? vjui::bone : vjui::ash);
            g.setFont (vjui::font (12.0f, ticked));
            g.drawText (name, 56, 0, getWidth() - 60, getHeight(), juce::Justification::centredLeft);
        }

    private:
        std::array<juce::Colour, 3> colours;
        juce::String name;
        bool ticked;
    };

    // Colour picker shown in a call-out; every change goes straight to the processor.
    class ColourPicker : public juce::Component, private juce::ChangeListener
    {
    public:
        ColourPicker (juce::Colour start, std::function<void (juce::Colour)> onChangeIn) : onChange (std::move (onChangeIn))
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
juce::Font vjui::font (float height, bool bold)
{
    return juce::Font (juce::FontOptions ("Bahnschrift", height, bold ? juce::Font::bold : juce::Font::plain));
}

juce::Font vjui::mono (float height)
{
    return juce::Font (juce::FontOptions ("Consolas", height, juce::Font::plain));
}

void vjui::setHelp (juce::Component& c, const juce::String& text)
{
    c.getProperties().set ("help", text);
    if (auto* t = dynamic_cast<juce::SettableTooltipClient*> (&c))
        t->setTooltip (text);
}

vjui::LookAndFeel::LookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, ink0);
    setColour (juce::TextButton::buttonColourId, ink1);
    setColour (juce::TextButton::buttonOnColourId, bone);
    setColour (juce::TextButton::textColourOffId, bone);
    setColour (juce::TextButton::textColourOnId, ink0);
    setColour (juce::PopupMenu::backgroundColourId, ink1);
    setColour (juce::PopupMenu::textColourId, bone);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, ink2);
    setColour (juce::PopupMenu::highlightedTextColourId, bone);
    setColour (juce::TooltipWindow::backgroundColourId, ink1);
    setColour (juce::TooltipWindow::textColourId, bone);
    setColour (juce::TooltipWindow::outlineColourId, line);
    setColour (juce::ScrollBar::thumbColourId, line);
}

void vjui::LookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                                          float start, float end, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h);
    auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f - 4.0f;
    auto centre = bounds.getCentre();
    const bool large = radius > 22.0f;
    const auto stroke = large ? 3.0f : 2.5f;
    auto angle = start + pos * (end - start);

    juce::Path track;
    track.addCentredArc (centre.x, centre.y, radius, radius, 0.0f, start, end, true);
    g.setColour (line);
    g.strokePath (track, juce::PathStrokeType (stroke, juce::PathStrokeType::curved, juce::PathStrokeType::butt));

    if (pos > 0.001f)
    {
        juce::Path value;
        value.addCentredArc (centre.x, centre.y, radius, radius, 0.0f, start, angle, true);
        g.setColour (slider.isMouseOverOrDragging() ? juce::Colours::white : bone);
        g.strokePath (value, juce::PathStrokeType (stroke, juce::PathStrokeType::curved, juce::PathStrokeType::butt));
    }

    auto disc = radius - stroke - 3.0f;
    g.setColour (ink1);
    g.fillEllipse (centre.x - disc, centre.y - disc, disc * 2.0f, disc * 2.0f);
    // A short tick at the value angle, inside the arc.
    auto tip = centre.getPointOnCircumference (radius - stroke * 0.5f, angle);
    auto base = centre.getPointOnCircumference (disc - 1.0f, angle);
    g.setColour (bone);
    g.drawLine ({ base, tip }, 1.5f);

    g.setColour (bone);
    g.setFont (mono (large ? 11.0f : 9.5f));
    g.drawText (knobText (slider), juce::Rectangle<float> (centre.x - disc, centre.y - 7.0f, disc * 2.0f, 14.0f).toNearestInt(),
                juce::Justification::centred, false);
}

void vjui::LookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float, float,
                                          juce::Slider::SliderStyle, juce::Slider& slider)
{
    auto r = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h);
    auto trackY = r.getCentreY();
    g.setColour (line);
    g.fillRect (r.getX(), trackY - 1.0f, r.getWidth(), 2.0f);
    g.setColour (slider.isMouseOverOrDragging() ? juce::Colours::white : bone);
    g.fillRect (r.getX(), trackY - 1.0f, pos - r.getX(), 2.0f);
    g.fillRect (pos - 2.0f, trackY - 7.0f, 4.0f, 14.0f);
}

void vjui::LookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced (0.5f);
    const bool on = b.getToggleState();
    auto fill = b.findColour (on ? juce::TextButton::buttonOnColourId : juce::TextButton::buttonColourId);
    if (! on && over && b.isEnabled())
        fill = fill == ink1 ? ink2 : fill.brighter (0.08f);
    if (down)
        fill = fill.interpolatedWith (bone, 0.25f);
    g.setColour (fill);
    g.fillRoundedRectangle (r, 2.0f);

    auto outline = b.getProperties().contains ("outline") ? juce::Colour ((juce::uint32) (int) b.getProperties()["outline"])
                                                          : (on ? fill : (over ? ash.withAlpha (0.6f) : line));
    g.setColour (b.isEnabled() ? outline : outline.withAlpha (0.4f));
    g.drawRoundedRectangle (r, 2.0f, (bool) b.getProperties().getWithDefault ("thick", false) ? 1.5f : 1.0f);
}

void vjui::LookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& b, bool, bool)
{
    auto colour = b.findColour (b.getToggleState() ? juce::TextButton::textColourOnId : juce::TextButton::textColourOffId);
    g.setColour (b.isEnabled() ? colour : colour.withAlpha (0.4f));
    g.setFont (getTextButtonFont (b, b.getHeight()));
    const bool left = (bool) b.getProperties().getWithDefault ("left", false);
    const bool bottom = (bool) b.getProperties().getWithDefault ("bottom", false);
    const auto just = left ? juce::Justification::centredLeft : (bottom ? juce::Justification::centredBottom : juce::Justification::centred);
    auto area = b.getLocalBounds().reduced (left ? (int) b.getProperties().getWithDefault ("pad", 8) : 3, bottom ? 5 : 2);
    area.removeFromLeft ((int) b.getProperties().getWithDefault ("indent", 0));
    g.drawFittedText (b.getButtonText(), area, just, 3, 1.0f);
}

juce::Font vjui::LookAndFeel::getTextButtonFont (juce::TextButton& b, int h)
{
    const auto size = (float) b.getProperties().getWithDefault ("fontSize", juce::jlimit (10.0f, 14.0f, h * 0.42f));
    return font (size, true);
}

void vjui::LookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool over, bool)
{
    auto r = b.getLocalBounds().toFloat();
    auto box = r.removeFromLeft (16.0f).withSizeKeepingCentre (12.0f, 12.0f);
    g.setColour (b.getToggleState() ? bone : (over ? ash : line));
    if (b.getToggleState()) g.fillRect (box); else g.drawRect (box, 1.0f);
    g.setColour (bone);
    g.setFont (font (11.0f, true));
    g.drawText (b.getButtonText(), r.withTrimmedLeft (6.0f), juce::Justification::centredLeft);
}

void vjui::LookAndFeel::drawPopupMenuBackground (juce::Graphics& g, int w, int h)
{
    g.fillAll (ink1);
    g.setColour (line);
    g.drawRect (0, 0, w, h, 1);
}

void vjui::LookAndFeel::drawPopupMenuItem (juce::Graphics& g, const juce::Rectangle<int>& area, bool isSeparator, bool isActive,
                                           bool isHighlighted, bool isTicked, bool, const juce::String& text,
                                           const juce::String&, const juce::Drawable*, const juce::Colour*)
{
    if (isSeparator)
    {
        g.setColour (line);
        g.fillRect (area.reduced (8, 0).withHeight (1).withY (area.getCentreY()));
        return;
    }
    if (isHighlighted && isActive)
    {
        g.setColour (ink2);
        g.fillRect (area);
    }
    g.setColour (! isActive ? dust : (isTicked ? bone : ash));
    g.setFont (font (12.0f, isTicked));
    g.drawText ((isTicked ? juce::String (juce::CharPointer_UTF8 ("\xe2\x80\xa2 ")) : juce::String ("   ")) + text,
                area.reduced (10, 0), juce::Justification::centredLeft, true);
}

juce::Font vjui::LookAndFeel::getPopupMenuFont() { return font (12.0f); }

void vjui::LookAndFeel::drawTooltip (juce::Graphics& g, const juce::String& text, int w, int h)
{
    g.fillAll (ink1);
    g.setColour (line);
    g.drawRect (0, 0, w, h, 1);
    g.setColour (bone);
    g.setFont (font (11.0f));
    g.drawFittedText (text, juce::Rectangle<int> (w, h).reduced (8, 4), juce::Justification::centredLeft, 4);
}

juce::Rectangle<int> vjui::LookAndFeel::getTooltipBounds (const juce::String& tipText, juce::Point<int> screenPos, juce::Rectangle<int> parentArea)
{
    const int w = juce::jmin (320, (int) juce::GlyphArrangement::getStringWidth (font (11.0f), tipText) + 20);
    const int lines = (int) juce::GlyphArrangement::getStringWidth (font (11.0f), tipText) / 300 + 1;
    const int h = 10 + 15 * lines;
    return juce::Rectangle<int> (screenPos.x > parentArea.getCentreX() ? screenPos.x - (w + 12) : screenPos.x + 24,
                                 screenPos.y > parentArea.getCentreY() ? screenPos.y - (h + 6) : screenPos.y + 6, w, h)
        .constrainedWithin (parentArea);
}

//==============================================================================
MacroKnob::MacroKnob (juce::AudioProcessorValueTreeState& state, const juce::String& paramId, const juce::String& l, bool isLarge, bool offAtZero)
    : label (l), large (isLarge)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    slider.setMouseDragSensitivity (220);
    slider.getProperties().set ("offAtZero", offAtZero);
    addAndMakeVisible (slider);
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (state, paramId, slider);
    if (auto* p = state.getParameter (paramId))
        slider.setDoubleClickReturnValue (true, p->convertFrom0to1 (p->getDefaultValue()));
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

void MacroKnob::setActivity (float a)
{
    a = juce::jlimit (0.0f, 1.0f, a);
    auto next = a > activity ? activity + (a - activity) * 0.6f : activity + (a - activity) * 0.12f;
    if (std::abs (next - activity) > 0.002f || (next == 0.0f) != (activity == 0.0f))
    {
        activity = next < 0.004f ? 0.0f : next;
        repaint();
    }
}

void MacroKnob::paintOverChildren (juce::Graphics& g)
{
    if (activity <= 0.0f)
        return;
    auto bounds = slider.getBounds().toFloat();
    auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f - 0.5f;
    auto centre = bounds.getCentre();
    auto params = slider.getRotaryParameters();
    auto range = params.endAngleRadians - params.startAngleRadians;
    auto from = params.startAngleRadians + (float) slider.valueToProportionOfLength (slider.getValue()) * range;
    auto to = juce::jmin (params.endAngleRadians, from + activity * range);
    if (to <= from)
        return;
    juce::Path ring;
    ring.addCentredArc (centre.x, centre.y, radius, radius, 0.0f, from, to, true);
    g.setColour (vjui::halation);
    g.strokePath (ring, juce::PathStrokeType (2.0f));
}

void MacroKnob::resized()
{
    auto r = getLocalBounds();
    slider.setBounds (large ? r.removeFromTop (60) : r.removeFromTop (44));
}

void MacroKnob::paint (juce::Graphics& g)
{
    auto r = getLocalBounds();
    r.removeFromTop (large ? 61 : 44);
    g.setColour (vjui::bone);
    g.setFont (vjui::font (large ? 10.5f : 9.5f, true));
    g.drawText (label.toUpperCase(), r.removeFromTop (13), juce::Justification::centred, true);
    if (subLabel.isNotEmpty())
    {
        g.setColour (subColour);
        g.setFont (vjui::font (large ? 10.0f : 9.0f));
        g.drawText (subLabel, r.removeFromTop (13), juce::Justification::centred, true);
    }
}

//==============================================================================
SegmentedControl::SegmentedControl (juce::StringArray cellNames) : names (std::move (cellNames))
{
    for (int i = 0; i < names.size(); ++i)
        cellHelp.add (juce::String());
}

int SegmentedControl::cellAt (juce::Point<int> p) const
{
    if (! getLocalBounds().contains (p) || names.isEmpty())
        return -1;
    return juce::jlimit (0, names.size() - 1, p.x * names.size() / juce::jmax (1, getWidth()));
}

void SegmentedControl::setSelected (int index)
{
    if (index != selected)
    {
        selected = index;
        repaint();
    }
}

void SegmentedControl::paint (juce::Graphics& g)
{
    const auto n = names.size();
    const auto w = (float) getWidth() / (float) n;
    for (int i = 0; i < n; ++i)
    {
        auto cell = juce::Rectangle<float> (w * (float) i, 0.0f, w, (float) getHeight()).reduced (1.0f, 0.5f);
        const bool sel = i == selected;
        g.setColour (sel ? vjui::bone : (i == hovered ? vjui::ink2 : vjui::ink1));
        g.fillRoundedRectangle (cell, 2.0f);
        g.setColour (i == pending ? vjui::tungsten : (sel ? vjui::bone : vjui::line));
        g.drawRoundedRectangle (cell, 2.0f, i == pending ? 1.5f : 1.0f);
        g.setColour (sel ? vjui::ink0 : (isEnabled() ? vjui::ash : vjui::dust));
        g.setFont (vjui::font (juce::jlimit (9.5f, 12.0f, (float) getHeight() * 0.42f), true));
        g.drawText (names[i], cell.toNearestInt(), juce::Justification::centred, true);
    }
}

void SegmentedControl::mouseDown (const juce::MouseEvent& e)
{
    auto c = cellAt (e.getPosition());
    if (c >= 0 && isEnabled() && onChange)
        onChange (c);
}

void SegmentedControl::mouseMove (const juce::MouseEvent& e)
{
    auto c = cellAt (e.getPosition());
    if (c != hovered)
    {
        hovered = c;
        vjui::setHelp (*this, c >= 0 ? cellHelp[c] : juce::String());
        repaint();
    }
}

void SegmentedControl::mouseExit (const juce::MouseEvent&)
{
    hovered = -1;
    repaint();
}

//==============================================================================
void SceneGrid::setState (const State& s)
{
    const bool changed = s.names != st.names || s.usesMedia != st.usesMedia || s.live != st.live || s.cued != st.cued
                      || s.firing != st.firing || s.connected != st.connected;
    st = s;
    if (changed || st.cued >= 0 || st.firing >= 0)
        repaint();
}

juce::Rectangle<int> SceneGrid::tileBounds (int index) const
{
    const auto row = index / columns - scrollRow, col = index % columns;
    return { col * stepX, row * stepY, tileW, tileH };
}

int SceneGrid::tileAt (juce::Point<int> p) const
{
    const auto col = p.x / stepX, row = p.y / stepY + scrollRow;
    if (col >= columns || p.x % stepX >= tileW || p.y % stepY >= tileH)
        return -1;
    const auto index = row * columns + col;
    return juce::isPositiveAndBelow (index, st.names.size()) ? index : -1;
}

void SceneGrid::paint (juce::Graphics& g)
{
    const auto pulse = 0.55f + 0.45f * (float) std::sin (seconds() * juce::MathConstants<double>::twoPi * 1.5);
    for (int i = 0; i < st.names.size(); ++i)
    {
        auto r = tileBounds (i);
        if (r.getBottom() <= 0 || r.getY() >= getHeight())
            continue;
        auto t = r.toFloat().reduced (0.5f);
        const bool live = i == st.live && st.connected, cued = i == st.cued, firing = i == st.firing && ! live, hover = i == hovered && st.connected;

        g.setColour (live || hover ? vjui::ink2 : vjui::ink1);
        g.fillRoundedRectangle (t, 2.0f);
        if (firing)
        {
            g.setColour (vjui::tungsten.withAlpha (0.22f));
            g.fillRoundedRectangle (t, 2.0f);
        }
        g.setColour (live ? vjui::signal : (cued ? vjui::tungsten.withAlpha (pulse) : (firing ? vjui::tungsten : (hover ? vjui::ash : vjui::line))));
        g.drawRoundedRectangle (t, 2.0f, live || cued || firing ? 1.5f : 1.0f);
        if (live)
        {
            g.setColour (vjui::signal);
            g.fillRect (t.withWidth (3.0f));
        }

        auto text = r.reduced (8, 0);
        juce::String badge;
        juce::Colour badgeColour;
        if (cued) { badge = "NEXT"; badgeColour = vjui::tungsten; }
        else if (firing) { badge = "BEAT"; badgeColour = vjui::tungsten; }
        if (badge.isNotEmpty())
        {
            auto b = text.removeFromRight (28).withSizeKeepingCentre (28, 12).toFloat();
            g.setColour (badgeColour);
            g.fillRoundedRectangle (b, 2.0f);
            g.setColour (vjui::ink0);
            g.setFont (vjui::font (8.5f, true));
            g.drawText (badge, b.toNearestInt(), juce::Justification::centred);
            text.removeFromRight (3);
        }
        if (juce::isPositiveAndBelow (i, st.usesMedia.size()) && st.usesMedia[i] && badge.isEmpty())
        {
            auto b = text.removeFromRight (20).withSizeKeepingCentre (20, 11).toFloat();
            g.setColour (vjui::ash.withAlpha (0.8f));
            g.drawRoundedRectangle (b, 2.0f, 1.0f);
            g.setFont (vjui::font (7.5f, true));
            g.drawText ("IMG", b.toNearestInt(), juce::Justification::centred);
            text.removeFromRight (3);
        }
        g.setColour (! st.connected ? vjui::dust : (live || cued || firing || hover ? vjui::bone : vjui::ash));
        g.setFont (vjui::font (11.0f, live));
        g.drawText (st.names[i], text, juce::Justification::centredLeft, true);
    }

    // Scroll hint when there are more than 16 scenes.
    const auto rows = (st.names.size() + columns - 1) / columns;
    if (rows > visibleRows)
    {
        const auto h = (float) getHeight() * (float) visibleRows / (float) rows;
        g.setColour (vjui::line);
        g.fillRect ((float) getWidth() - 2.0f, (float) scrollRow / (float) rows * (float) getHeight(), 2.0f, h);
    }
}

void SceneGrid::mouseDown (const juce::MouseEvent& e)
{
    if (! st.connected || e.getNumberOfClicks() > 1)
        return;
    if (auto t = tileAt (e.getPosition()); t >= 0 && onCue)
        onCue (t);
}

void SceneGrid::mouseDoubleClick (const juce::MouseEvent& e)
{
    if (! st.connected)
        return;
    if (auto t = tileAt (e.getPosition()); t >= 0 && onFire)
        onFire (t);
}

void SceneGrid::mouseMove (const juce::MouseEvent& e)
{
    auto t = tileAt (e.getPosition());
    if (t != hovered)
    {
        hovered = t;
        vjui::setHelp (*this, t >= 0 ? st.names[t] + (t < st.descriptions.size() && st.descriptions[t].isNotEmpty() ? ": " + st.descriptions[t] : juce::String())
                                           + "  -  click = cue it (NEXT), GO = switch, double-click = both."
                                     : juce::String());
        repaint();
    }
}

void SceneGrid::mouseExit (const juce::MouseEvent&)
{
    hovered = -1;
    repaint();
}

void SceneGrid::mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& w)
{
    const auto rows = (st.names.size() + columns - 1) / columns;
    scrollRow = juce::jlimit (0, juce::jmax (0, rows - visibleRows), scrollRow + (w.deltaY < 0 ? 1 : -1));
    repaint();
}

//==============================================================================
void MomentTile::setInfo (bool isStored, bool isActive, const juce::String& r, float p, bool justSaved)
{
    if (isStored != stored || isActive != active || r != recipe || std::abs (p - progress) > 0.01f || justSaved != saved)
    {
        stored = isStored;
        active = isActive;
        recipe = r;
        progress = p;
        saved = justSaved;
        repaint();
    }
}

void MomentTile::paintButton (juce::Graphics& g, bool over, bool)
{
    auto r = getLocalBounds().toFloat().reduced (0.5f);
    g.setColour (stored ? (over ? vjui::ink2 : vjui::ink1) : vjui::ink0);
    g.fillRoundedRectangle (r, 2.0f);
    const auto outline = saved ? vjui::tungsten : (active ? vjui::bone : (stored ? vjui::line : vjui::dust.withAlpha (0.6f)));
    if (stored || saved)
    {
        g.setColour (outline);
        g.drawRoundedRectangle (r, 2.0f, active || saved ? 1.5f : 1.0f);
    }
    else
    {
        juce::Path p;
        p.addRoundedRectangle (r, 2.0f);
        juce::Path dashed;
        const float dashes[] = { 3.0f, 3.0f };
        juce::PathStrokeType (1.0f).createDashedStroke (dashed, p, dashes, 2);
        g.setColour (outline);
        g.fillPath (dashed);
    }
    g.setColour (stored ? vjui::bone : vjui::dust);
    g.setFont (vjui::font (13.0f, true));
    g.drawText (juce::String::charToString ((juce::juce_wchar) ('A' + slot)), getLocalBounds().reduced (7, 0).removeFromTop (getHeight() / 2 + 2),
                juce::Justification::bottomLeft);
    g.setColour (saved ? vjui::tungsten : (stored ? vjui::ash : vjui::dust));
    g.setFont (vjui::font (9.0f));
    g.drawText (saved ? juce::String ("SAVED") : (stored ? recipe : juce::String ("+ save")),
                getLocalBounds().reduced (7, 0).removeFromBottom (getHeight() / 2 - 1), juce::Justification::topLeft, true);
    if (progress >= 0.0f)
    {
        g.setColour (vjui::halation);
        g.fillRect (r.getX(), r.getBottom() - 2.0f, r.getWidth() * progress, 2.0f);
    }
}

//==============================================================================
VJAnalyzerEditor::VJAnalyzerEditor (VJAnalyzerProcessor& p)
    : AudioProcessorEditor (p), processor (p)
{
    setLookAndFeel (&lookAndFeel);
    auto& state = processor.getState();
    auto press = [this] (const juce::String& id) { setParameter (id, 1.0f); };

    // ------------------------------------------------------------ header
    enginePill.getProperties().set ("left", true);
    enginePill.getProperties().set ("indent", 18);
    enginePill.getProperties().set ("fontSize", 11.0f);
    enginePill.onClick = [this] { openEngine(); };
    addChildComponent (enginePill);

    fullscreenButton.onClick = [this] { processor.getWorker().toggleEngineFullscreen(); };
    vjui::setHelp (fullscreenButton, "Engine full screen on / off (F in the engine window does the same).");
    addChildComponent (fullscreenButton);

    leadChip.getProperties().set ("fontSize", 10.0f);
    leadChip.onClick = [this] {
        processor.getState().state.setProperty ("drawerOpen", true, nullptr);
        showTab (setupTab);
        updateMode();
    };
    vjui::setHelp (leadChip, "This window leads the picture: its knobs, look and scene go to the engine. Click for setup.");
    addChildComponent (leadChip);

    editButton.setClickingTogglesState (false);
    editButton.onClick = [this] {
        auto& s = processor.getState().state;
        s.setProperty ("drawerOpen", ! (bool) s.getProperty ("drawerOpen", false), nullptr);
        updateMode();
    };
    vjui::setHelp (editButton, "Open the detail drawer: look, colour, reactions, motion, media, setup.");
    addChildComponent (editButton);

    blackout.setClickingTogglesState (true);
    styleButton (blackout, vjui::ink0, vjui::signal, vjui::signal, vjui::ink0);
    outlineOf (blackout, vjui::signal);
    blackout.getProperties().set ("thick", true);
    vjui::setHelp (blackout, "Fades the output to black. Click again to bring it back. (Live: Blackout)");
    blackoutAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (state, "blackout", blackout);
    addChildComponent (blackout);

    // ------------------------------------------------------------ scenes
    previous.onClick = [press] { press ("scenePrev"); };
    next.onClick = [press] { press ("sceneNext"); };
    vjui::setHelp (previous, "Cue the previous scene. GO switches. (Live: Previous Scene)");
    vjui::setHelp (next, "Cue the next scene. GO switches. (Live: Next Scene)");
    go.onClick = [this, press] {
        if (processor.getCuedScene() < 0)
            setInfo ("Cue a scene first: click a tile, then GO.", vjui::tungsten);
        press ("sceneGo");
    };
    go.getProperties().set ("fontSize", 14.0f);
    styleButton (go, vjui::ink1, vjui::dust, vjui::tungsten, vjui::ink0);
    vjui::setHelp (go, "Switch to the cued scene. While Live plays it lands on the beat. (Live: Go)");
    cueField.getProperties().set ("left", true);
    cueField.getProperties().set ("fontSize", 11.0f);
    cueField.onClick = [this] { processor.cueScene (-1); };
    vjui::setHelp (cueField, "The scene waiting for GO. Click to cancel the cue.");
    for (auto* c : std::initializer_list<juce::Component*> { &previous, &next, &go, &cueField })
        addChildComponent (c);

    sceneGrid.setComponentID ("sceneList");
    sceneGrid.onCue = [this] (int i) {
        processor.cueScene (i == status.presetIndex || i == processor.getCuedScene() ? -1 : i);
        shownCue = -2;
    };
    sceneGrid.onFire = [this] (int i) { selectPreset (i); };
    addChildComponent (sceneGrid);

    // ------------------------------------------------------------ react row + lamps
    auto reactChange = [this] (int i) {
        if (i == 0)
            setParameter ("calm", 1.0f);
        else
        {
            setParameter ("calm", 0.0f);
            setParameter ("reactStyle", (float) (i - 1));
        }
    };
    const char* reactHelp[] = {
        "STILL - reactions fade out over one bar; the picture keeps moving on its own. Click a character to come back. (Live: Calm)",
        "BREATHE - the picture swells with the music; only the big moments land. For ambient and the dance show.",
        "PULSE - every beat is felt, the strong hits land, silence rests. The default.",
        "PUNCH - tight, sharp hits, more of them; cut-ready." };
    for (auto* row : { &reactRow, &reactRow2 })
    {
        row->onChange = reactChange;
        for (int i = 0; i < 4; ++i)
            row->setCellHelp (i, reactHelp[i]);
    }
    addChildComponent (reactRow);

    const char* lampNames[] = { "KICK", "SNARE", "HAT", "BASS", "LEVEL" };
    for (int i = 0; i < 5; ++i)
    {
        auto* b = lamps.add (new juce::TextButton (lampNames[i]));
        b->setClickingTogglesState (true);
        b->getProperties().set ("fontSize", 9.5f);
        b->getProperties().set ("left", true);
        b->getProperties().set ("pad", 3);
        b->getProperties().set ("indent", 13);
        styleButton (*b, vjui::ink0, vjui::dust, vjui::ink1, vjui::bone);
        vjui::setHelp (*b, juce::String ("What the picture follows: ") + juce::String (lampNames[i]).toLowerCase()
                           + ". The lamp flashes with it; click to make the picture ignore it (and again to follow).");
        lampAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
            state, VJAnalyzerProcessor::reactIds[i], *b));
        addChildComponent (b);
    }

    // ------------------------------------------------------------ style
    colourPicker.getProperties().set ("left", true);
    colourPicker.getProperties().set ("indent", 36);
    colourPicker.getProperties().set ("fontSize", 11.0f);
    colourPicker.onClick = [this] { showPaletteMenu(); };
    vjui::setHelp (colourPicker, "The picture's three colours. Click to choose. (Live: Palette)");
    lookPicker.getProperties().set ("left", true);
    lookPicker.getProperties().set ("fontSize", 11.0f);
    lookPicker.onClick = [this] { showLookMenu(); };
    vjui::setHelp (lookPicker, "The film / digital treatment over every scene. Click to choose. (Live: Look)");
    for (auto* s : { &lookAmount, &lookAmount2 })
    {
        s->setSliderStyle (juce::Slider::LinearHorizontal);
        s->setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        s->setDoubleClickReturnValue (true, 1.0);
        vjui::setHelp (*s, "How strong the look is. 0 = clean picture, 100% = the look as set. Double-click = 100%. (Live: Look Amount)");
    }
    lookAmountAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (state, "lookAmount", lookAmount);
    lookAmount2Attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (state, "lookAmount", lookAmount2);
    for (auto* c : std::initializer_list<juce::Component*> { &colourPicker, &lookPicker, &lookAmount })
        addChildComponent (c);

    // ------------------------------------------------------------ media + moments
    mediaChip.getProperties().set ("left", true);
    mediaChip.getProperties().set ("fontSize", 10.0f);
    mediaChip.onClick = [this] { processor.getState().state.setProperty ("drawerOpen", true, nullptr); showTab (mediaTab); updateMode(); };
    vjui::setHelp (mediaChip, "Your image or clip in the active slot. Click to manage the slots. Drop a file anywhere on this window to load it.");
    loadButton.onClick = [this] { loadMedia (processor.getMediaSlot()); };
    vjui::setHelp (loadButton, "Load an image (PNG, JPEG) or a short clip into the active slot. If the live scene can't show it, the plug-in switches to a scene that can.");
    morphButton.getProperties().set ("fontSize", 10.0f);
    styleButton (morphButton, vjui::ink0, vjui::ash, vjui::ink0, vjui::ash);
    outlineOf (morphButton, vjui::ink0);
    morphButton.onClick = [this] { showMorphMenu(); };
    vjui::setHelp (morphButton, "How long a recalled moment takes to blend in. (Live: Snapshot Morph)");
    for (auto* c : std::initializer_list<juce::Component*> { &mediaChip, &loadButton, &morphButton })
        addChildComponent (c);
    for (int i = 0; i < VJAnalyzerProcessor::numSnapshots; ++i)
    {
        auto* m = moments.add (new MomentTile (i));
        m->onClick = [this, i] { momentClicked (i, nullptr); };
        m->onRightClick = [this, i] (const juce::MouseEvent& e) { momentClicked (i, &e); };
        addChildComponent (m);
    }

    // ------------------------------------------------------------ the 8 macros, FREEZE, HIT, DROP
    const char* macroHelp[] = {
        "INTENSITY - light and energy of the scene.",
        "SPEED - how fast everything moves. 0 = frozen, middle = the designed speed, full = x4.",
        "FORM - the scene's character (the word under the knob says what it is here).",
        "SCALE - bigger as you turn it up.",
        "REACT - how much the music moves the picture. 0 = ignores it, 50% = as designed, 100% = wild. Picks which hits become events, how dark silence gets, how hard the music pushes. (Live: React)",
        "ERODE - from pristine to worn, torn and dissolved.",
        "GLIDE - how long speed changes take to arrive: 0 to 4 bars.",
        "DETAIL - density and fineness." };
    for (int i = 0; i < 8; ++i)
    {
        auto* k = macros.add (new MacroKnob (state, "macro" + juce::String (i + 1), VJAnalyzerProcessor::macroNames[i], true));
        k->setHelpText (macroHelp[i]);
        addChildComponent (k);
    }
    freeze.setClickingTogglesState (true);
    freeze.getProperties().set ("fontSize", 10.0f);
    vjui::setHelp (freeze, "Stops all motion at once; the film grain keeps running. Click again to go on at the same speed. (Live: Freeze)");
    freezeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (state, "freeze", freeze);
    addChildComponent (freeze);
    hit.getProperties().set ("fontSize", 18.0f);
    hit.onClick = [this, press] { press ("hit"); hitFlash = now(); };
    vjui::setHelp (hit, "A manual hit: always lands, even in STILL or with REACT at 0. (Live: Hit)");
    drop.getProperties().set ("fontSize", 11.0f);
    drop.onClick = [this, press] { press ("drop"); dropFlash = now(); };
    vjui::setHelp (drop, "The big moment by hand: a bloom, a lurch forward, the build-up released. Also detected by itself on real drops. (Live: Drop)");
    addChildComponent (hit);
    addChildComponent (drop);

    // ------------------------------------------------------------ drawer tabs
    const char* tabNames[] = { "LOOK", "COLOUR", "REACT", "MOTION", "MEDIA", "SETUP" };
    for (int i = 0; i < numTabs; ++i)
    {
        auto* b = tabButtons.add (new juce::TextButton (tabNames[i]));
        b->getProperties().set ("fontSize", 10.0f);
        styleButton (*b, vjui::ink0, vjui::ash, vjui::ink2, vjui::bone);
        b->onClick = [this, i] { showTab (i); };
        addChildComponent (b);
    }

    // LOOK
    lookRow.onChange = [this] (int i) { setParameter ("lookPreset", (float) (i + 1)); };
    const char* lookHelp[] = { "CLEAN - almost no treatment: a clean digital picture.",
                               "FILM - 35 mm: grain, halation, weave, dust, lifted blacks. The default.",
                               "WORN - an old print: heavier grain, dust and weave.",
                               "BROKEN - digital damage: glitch, smear, flashes and hit cuts.",
                               "PRINT - photocopy: hard two-tone, grey paper blacks, grain.",
                               "DATA - instrument: glyph strips, HUD overlay, hit cuts." };
    for (int i = 0; i < 6; ++i)
        lookRow.setCellHelp (i, lookHelp[i]);
    tabContents[lookTab].addArray (std::initializer_list<juce::Component*> { &lookRow, &lookAmount2, &lookReset });
    struct KnobDef { int slot; const char* name; const char* tip; };
    const KnobDef film[] = { { 0, "Grain", "Film grain at 24 fps, following the image's brightness." },
                             { 8, "Halation", "Red glow bleeding out of the highlights." },
                             { 9, "Gate Weave", "Gate weave, rare frame slips, flicker, soft colour fringes." },
                             { 10, "Dust", "Dust specks, hairs and scratches - rare and correlated. Builds up through a section." },
                             { 11, "Film Base", "Lifted, tinted blacks: the film base." } };
    const KnobDef digital[] = { { 1, "Crush", "Soft S-curve -> posterised -> hard two-tone." },
                                { 4, "Trails", "Frame memory: motion leaves a trail." },
                                { 3, "Glitch", "Row tears, pixel blocks, RGB split, pulsed by hits." },
                                { 7, "Smear", "VHS tracking drift on rows, in bursts." },
                                { 5, "Glyphs", "Braille glyph strips, re-dealt on snares." } };
    for (auto& k : film)
    {
        auto* knob = filmKnobs.add (new MacroKnob (state, VJAnalyzerProcessor::lookIds[k.slot], k.name, false, true));
        knob->setHelpText (k.tip);
        tabContents[lookTab].add (knob);
    }
    for (auto& k : digital)
    {
        auto* knob = digitalKnobs.add (new MacroKnob (state, VJAnalyzerProcessor::lookIds[k.slot], k.name, false, true));
        knob->setHelpText (k.tip);
        tabContents[lookTab].add (knob);
    }
    {
        auto* flash = hitKnobs.add (new MacroKnob (state, "flash", "Flash", false, true));
        flash->setHelpText ("Chance an accent drops a black or colour frame (max 3 a second).");
        auto* shots = hitKnobs.add (new MacroKnob (state, "shots", "Shots", false, true));
        shots->setHelpText ("Accents become short cuts: new framing, negative, lens punch, black frame. A drop always cuts.");
        auto* hud = hitKnobs.add (new MacroKnob (state, "hud", "HUD", false, true));
        hud->setHelpText ("Measuring-instrument overlay: crosshairs, linked squares, faint circles.");
        for (auto* k : hitKnobs)
            tabContents[lookTab].add (k);
    }
    lookReset.setButtonText ("RESET LOOK");
    lookReset.getProperties().set ("fontSize", 10.0f);
    lookReset.onClick = [this] {
        auto preset = (int) param ("lookPreset");
        if (preset == VJAnalyzerProcessor::customLook)
            preset = 2;
        setParameter ("lookPreset", (float) VJAnalyzerProcessor::customLook);
        setParameter ("lookPreset", (float) preset);
    };
    vjui::setHelp (lookReset, "Put every knob back to the chosen look (Film if the look is Custom).");

    // COLOUR
    const int cardOrder[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 13, 11, 12 };
    for (auto index : cardOrder)
    {
        auto* b = paletteCards.add (new juce::TextButton (VJAnalyzerProcessor::paletteNames[index]));
        b->getProperties().set ("palette", index);
        b->getProperties().set ("fontSize", 10.0f);
        b->getProperties().set ("bottom", true);
        b->setClickingTogglesState (false);
        styleButton (*b, vjui::ink1, vjui::ash, vjui::ink2, vjui::bone);
        b->onClick = [this, index] { setParameter ("palette", (float) index); };
        vjui::setHelp (*b, index == VJAnalyzerProcessor::sceneColours ? "Every scene keeps its own colours (palette off)."
                         : index == 13 ? "Black / red / cyan, made for Negative."
                                       : VJAnalyzerProcessor::paletteNames[index] + " palette.");
        tabContents[colourTab].add (b);
    }
    const char* swatchNames[] = { "SHADOW", "MID", "LIGHT" };
    for (int i = 0; i < 3; ++i)
    {
        auto* s = swatches.add (new juce::TextButton (swatchNames[i]));
        s->getProperties().set ("fontSize", 9.5f);
        s->onClick = [this, i] { editColour (i); };
        vjui::setHelp (*s, "Click to change this colour. The palette becomes Custom.");
        tabContents[colourTab].add (s);
    }

    // REACT
    tabContents[reactTab].addArray (std::initializer_list<juce::Component*> { &reactRow2, &dropButton2 });
    {
        struct R { const char* id; const char* name; const char* tip; };
        const R defs[] = { { "macro5", "React", macroHelp[4] },
                           { "reactivity", "Follow", "How much the steady flow (loudness, bass, brightness) moves the picture, on top of REACT. 100% = as designed. (Live: Reactivity)" },
                           { "softness", "Decay", "How long each hit reaction lingers, on top of the character: 50% = as the character says. (Live: Softness)" },
                           { "push", "Music Push", "How hard loud passages lean on the speed, on top of the character: 30% = as the character says. (Live: Push)" } };
        for (auto& d : defs)
        {
            auto* k = reactKnobs.add (new MacroKnob (state, d.id, d.name, false));
            k->setHelpText (d.tip);
            tabContents[reactTab].add (k);
        }
    }
    for (int i = 0; i < 5; ++i)
    {
        auto* b = follows.add (new juce::TextButton (lampNames[i]));
        b->setClickingTogglesState (true);
        b->getProperties().set ("fontSize", 10.0f);
        styleButton (*b, vjui::ink0, vjui::dust, vjui::ink2, vjui::bone);
        vjui::setHelp (*b, juce::String ("The picture follows ") + juce::String (lampNames[i]).toLowerCase() + ". Off = ignored.");
        followAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
            state, VJAnalyzerProcessor::reactIds[i], *b));
        tabContents[reactTab].add (b);
    }
    dropButton2.getProperties().set ("fontSize", 10.0f);
    dropButton2.onClick = [this, press] { press ("drop"); dropFlash = now(); };
    vjui::setHelp (dropButton2, drop.getProperties()["help"].toString());

    // MOTION
    driftKnob = std::make_unique<MacroKnob> (state, "drift", "Camera Drift", false, true);
    driftKnob->setHelpText ("A slow camera over the whole picture: push-in, turn, pan. Stops with Speed 0 or Freeze. (Live: Drift)");
    tabContents[motionTab].add (driftKnob.get());
    struct T { juce::TextButton* b; const char* id; const char* tip; };
    const T motionToggles[] = { { &freeze2, "freeze", "Stops all motion; the film grain keeps running. (Live: Freeze)" },
                                { &reverse, "reverse", "Runs all motion backwards. (Live: Reverse)" },
                                { &sync, "sync", "Speeds follow Live's tempo: the designed speed is at 120 BPM. (Live: Sync)" } };
    for (auto& t : motionToggles)
    {
        t.b->setClickingTogglesState (true);
        t.b->getProperties().set ("fontSize", 10.0f);
        styleButton (*t.b, vjui::ink1, vjui::ash, vjui::bone, vjui::ink0);
        vjui::setHelp (*t.b, t.tip);
        motionAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (state, t.id, *t.b));
        tabContents[motionTab].add (t.b);
    }
    cutRow.onChange = [this] (int i) {
        const float values[] = { 0.0f, 0.1f, 0.3f, 0.5f, 0.7f, 0.8f, 0.95f };
        setParameter ("cutRate", values[i]);
    };
    const char* cutBeats[] = { "", "16", "8", "4", "2", "1" };
    for (int i = 0; i < 7; ++i)
        cutRow.setCellHelp (i, i == 0 ? juce::String ("No automatic scene cuts (the default).")
                             : i == 6 ? juce::String ("A cut on every accent (max 3 a second). None in STILL.")
                                      : "An automatic cut to another scene every " + juce::String (cutBeats[i]) + " beats.");
    tabContents[motionTab].add (&cutRow);

    // MEDIA
    for (int i = 0; i < 8; ++i)
    {
        auto* b = mediaCards.add (new juce::TextButton (juce::String (i + 1)));
        b->getProperties().set ("fontSize", 9.5f);
        b->onClick = [this, i] { setParameter ("mediaSlot", (float) (i + 1)); };
        vjui::setHelp (*b, "Slot " + juce::String (i + 1) + ". Click to show it; drop a file here to load it. (Live: Media Slot)");
        tabContents[mediaTab].add (b);
    }
    mediaLoad.onClick = [this] { loadMedia (processor.getMediaSlot()); };
    vjui::setHelp (mediaLoad, "Load an image or a short clip into the selected slot.");
    mediaClear.onClick = [this] { processor.setMediaPath (processor.getMediaSlot(), {}); };
    vjui::setHelp (mediaClear, "Empty the selected slot; scenes go back to their own forms.");
    useMedia.setClickingTogglesState (true);
    useMedia.getProperties().set ("fontSize", 10.0f);
    vjui::setHelp (useMedia, "Put your image into every scene that can use one (marked IMG). (Live: Use Media)");
    useMediaAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (state, "useMedia", useMedia);
    useMedia.onClick = [this] { if (useMedia.getToggleState()) ensureMediaScene(); };
    clipModeRow.onChange = [this] (int i) { setParameter ("clipMode", (float) i); };
    clipModeRow.setCellHelp (0, "Clips loop from the end back to the start. (Live: Clip Mode)");
    clipModeRow.setCellHelp (1, "Clips play forward and back. (Live: Clip Mode)");
    clipSyncRow.onChange = [this] (int i) { setParameter ("clipSync", (float) i); };
    for (int i = 0; i < 6; ++i)
        clipSyncRow.setCellHelp (i, i == 0 ? "The clip plays at its own speed x Speed. (Live: Clip Sync)"
                                           : "The clip is stretched over " + VJAnalyzerProcessor::clipSyncNames[i].toLowerCase() + ", locked to the tempo. (Live: Clip Sync)");
    tabContents[mediaTab].addArray (std::initializer_list<juce::Component*> { &mediaLoad, &mediaClear, &useMedia, &clipModeRow, &clipSyncRow });

    // SETUP
    auto roleChange = [this] (int i) { processor.setRoleByHand (i); };
    for (auto* row : { &roleRow, &sourceRole })
    {
        row->onChange = roleChange;
        for (int i = 0; i < 6; ++i)
            row->setCellHelp (i, "What is on this track: " + VJAnalyzerProcessor::roleNames[i] + ". Picking by hand turns AUTO off.");
    }
    for (auto* b : { &roleAuto, &sourceAuto })
    {
        b->getProperties().set ("fontSize", 10.0f);
        styleButton (*b, vjui::ink1, vjui::ash, vjui::bone, vjui::ink0);
        b->onClick = [this] { processor.setRoleAuto(); };
        vjui::setHelp (*b, "The role follows the track name (Kick, Snare, Hat, Bass, Master ...).");
    }
    canLead.setClickingTogglesState (true);
    canLead.getProperties().set ("fontSize", 10.0f);
    styleButton (canLead, vjui::ink1, vjui::ash, vjui::bone, vjui::ink0);
    vjui::setHelp (canLead, "Off = this instance never leads the picture (for example a test copy). (Live: Send Controls)");
    canLeadAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (state, "sendControls", canLead);
    for (auto* b : { &makeLead, &sourceMakeLead })
    {
        b->getProperties().set ("fontSize", 10.0f);
        b->onClick = [this] { processor.makeLead(); };
        vjui::setHelp (*b, "Make this window the one that plays the picture.");
    }
    {
        struct I { const char* id; const char* name; const char* tip; };
        const I defs[] = { { "sensitivity", "Detect", "How easily hits are found on this track. More = more hits. (Live: Hit Sensitivity)" },
                           { "trim", "Trim", "Input level for the analysis only; the sound is never changed. (Live: Input Trim)" },
                           { "lookahead", "Lookahead", "Delays the sound (Live compensates) so the picture lands early. Only for playback / DJ sets: live instruments through Live would be late too. (Live: Visual Lookahead)" } };
        for (auto& d : defs)
        {
            auto* k = inputKnobs.add (new MacroKnob (state, d.id, d.name, false, juce::String (d.id) == "lookahead"));
            k->setHelpText (d.tip);
            tabContents[setupTab].add (k);
        }
        for (int i = 0; i < 2; ++i)
        {
            auto* k = sourceKnobs.add (new MacroKnob (state, defs[i].id, defs[i].name, false));
            k->setHelpText (defs[i].tip);
            addChildComponent (k);
        }
    }
    levelRow.onChange = [this] (int i) { setParameter ("normalizer", (float) i); };
    levelRow.setCellHelp (0, "Follows each song's loudness. (Live: Response)");
    levelRow.setCellHelp (1, "Stays put after calibration. (Live: Response)");
    locateEngine.getProperties().set ("fontSize", 10.0f);
    locateEngine.onClick = [this] { processor.getState().state.setProperty ("enginePath", "", nullptr); openEngine(); };
    vjui::setHelp (locateEngine, "Point at VJ Engine.exe (only needed if it moved).");
    tabContents[setupTab].addArray (std::initializer_list<juce::Component*> { &roleRow, &roleAuto, &canLead, &makeLead, &levelRow, &locateEngine });

    for (auto& tab : tabContents)
        for (auto* c : tab)
            addChildComponent (c);
    for (auto* c : std::initializer_list<juce::Component*> { &sourceRole, &sourceAuto, &sourceMakeLead })
        addChildComponent (c);

    currentTab = juce::jlimit (0, numTabs - 1, (int) processor.getState().state.getProperty ("drawerTab", 0));
    updateMode();
    startTimerHz (30);
}

VJAnalyzerEditor::~VJAnalyzerEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

double VJAnalyzerEditor::now() const { return seconds(); }

void VJAnalyzerEditor::setParameter (const juce::String& id, float plainValue)
{
    if (auto* p = processor.getState().getParameter (id))
    {
        p->beginChangeGesture();
        p->setValueNotifyingHost (p->convertTo0to1 (plainValue));
        p->endChangeGesture();
    }
}

float VJAnalyzerEditor::param (const juce::String& id) const
{
    return processor.getState().getRawParameterValue (id)->load();
}

//==============================================================================
void VJAnalyzerEditor::updateMode()
{
    const auto desired = ! processor.isLead() ? Mode::source
                       : (bool) processor.getState().state.getProperty ("drawerOpen", false) ? Mode::edit : Mode::play;
    mode = desired;

    const bool lead = mode != Mode::source, drawer = mode == Mode::edit;
    for (auto* c : std::initializer_list<juce::Component*> { &enginePill, &fullscreenButton, &leadChip, &editButton, &blackout,
                                                             &previous, &next, &go, &cueField, &sceneGrid, &reactRow, &colourPicker,
                                                             &lookPicker, &lookAmount, &mediaChip, &loadButton, &morphButton,
                                                             &freeze, &hit, &drop })
        c->setVisible (lead);
    for (auto* c : lamps)   c->setVisible (lead);
    for (auto* c : moments) c->setVisible (lead);
    for (auto* c : macros)  c->setVisible (lead);
    for (auto* c : tabButtons) c->setVisible (drawer);
    for (int t = 0; t < numTabs; ++t)
        for (auto* c : tabContents[t])
            c->setVisible (drawer && t == currentTab);
    for (auto* c : std::initializer_list<juce::Component*> { &sourceRole, &sourceAuto, &sourceMakeLead })
        c->setVisible (! lead);
    for (auto* c : sourceKnobs) c->setVisible (! lead);

    const int w = mode == Mode::source ? sourceW : (drawer ? editW : playW), h = mode == Mode::source ? sourceH : height;
    if (getWidth() != w || getHeight() != h)
        setSize (w, h);
    else
        resized();
    repaint();
}

void VJAnalyzerEditor::showTab (int tab)
{
    currentTab = juce::jlimit (0, numTabs - 1, tab);
    processor.getState().state.setProperty ("drawerTab", currentTab, nullptr);
    for (int i = 0; i < tabButtons.size(); ++i)
        tabButtons[i]->setToggleState (i == currentTab, juce::dontSendNotification);
    if (mode == Mode::edit)
        for (int t = 0; t < numTabs; ++t)
            for (auto* c : tabContents[t])
                c->setVisible (t == currentTab);
    repaint();
}

void VJAnalyzerEditor::resized()
{
    if (mode == Mode::source)
    {
        layoutSource();
        return;
    }
    layoutPlay();
    if (mode == Mode::edit)
        layoutDrawer();
}

void VJAnalyzerEditor::layoutPlay()
{
    enginePill.setBounds (124, 8, 280, 24);
    fullscreenButton.setBounds (408, 8, 48, 24);
    leadChip.setBounds (462, 8, 102, 24);
    editButton.setBounds (572, 8, 76, 24);
    blackout.setBounds (660, 6, 84, 28);

    previous.setBounds (16, 116, 32, 36);
    cueField.setBounds (52, 116, 276, 36);
    next.setBounds (332, 116, 32, 36);
    go.setBounds (372, 116, 92, 36);
    sceneGrid.setBounds (16, 162, 450, 134);

    reactRow.setBounds (480, 64, 264, 28);
    for (int i = 0; i < lamps.size(); ++i)
        lamps[i]->setBounds (480 + 53 * i, 96, 51, 18);
    colourPicker.setBounds (480, 142, 130, 24);
    lookPicker.setBounds (614, 142, 130, 24);
    lookAmount.setBounds (566, 172, 138, 20);
    mediaChip.setBounds (480, 206, 196, 24);
    loadButton.setBounds (680, 206, 64, 24);
    morphButton.setBounds (636, 240, 108, 20);
    for (int i = 0; i < moments.size(); ++i)
        moments[i]->setBounds (480 + 67 * i, 264, 63, 32);

    const int knobX[] = { 16, 392, 88, 160, 552, 232, 464, 304 }; // by slot
    for (int i = 0; i < macros.size(); ++i)
        macros[i]->setBounds (knobX[i], 338, 72, 94);
    freeze.setBounds (456, 314, 80, 22);
    hit.setBounds (640, 338, 104, 60);
    drop.setBounds (640, 404, 104, 26);
    infoArea = { 16, 446, (mode == Mode::edit ? editW : playW) - 32, 22 };
}

void VJAnalyzerEditor::layoutDrawer()
{
    drawerArea = { playW, 0, editW - playW, height };
    for (int i = 0; i < tabButtons.size(); ++i)
        tabButtons[i]->setBounds (776 + 65 * i, 8, 62, 24);

    // LOOK
    lookRow.setBounds (776, 48, 388, 26);
    lookAmount2.setBounds (850, 82, 270, 20);
    for (int i = 0; i < filmKnobs.size(); ++i)    filmKnobs[i]->setBounds (776 + 78 * i, 128, 74, 66);
    for (int i = 0; i < digitalKnobs.size(); ++i) digitalKnobs[i]->setBounds (776 + 78 * i, 220, 74, 66);
    for (int i = 0; i < hitKnobs.size(); ++i)     hitKnobs[i]->setBounds (776 + 78 * i, 312, 74, 66);
    lookReset.setBounds (1060, 404, 104, 24);

    // COLOUR
    for (int i = 0; i < paletteCards.size(); ++i)
        paletteCards[i]->setBounds (776 + 98 * (i % 4), 48 + 52 * (i / 4), 94, 48);
    for (int i = 0; i < swatches.size(); ++i)
        swatches[i]->setBounds (776 + 130 * i, 284, 124, 44);

    // REACT
    reactRow2.setBounds (776, 48, 388, 28);
    for (int i = 0; i < reactKnobs.size(); ++i) reactKnobs[i]->setBounds (776 + 97 * i, 132, 93, 66);
    for (int i = 0; i < follows.size(); ++i)    follows[i]->setBounds (776 + 78 * i, 224, 74, 26);
    dropButton2.setBounds (1060, 404, 104, 24);

    // MOTION
    driftKnob->setBounds (776, 64, 93, 66);
    freeze2.setBounds (880, 64, 90, 28);
    reverse.setBounds (976, 64, 90, 28);
    sync.setBounds (1072, 64, 92, 28);
    cutRow.setBounds (776, 164, 388, 28);

    // MEDIA
    for (int i = 0; i < mediaCards.size(); ++i)
        mediaCards[i]->setBounds (776 + 98 * (i % 4), 48 + 78 * (i / 4), 94, 72);
    mediaLoad.setBounds (776, 212, 90, 24);
    mediaClear.setBounds (872, 212, 70, 24);
    useMedia.setBounds (948, 212, 216, 24);
    clipModeRow.setBounds (776, 264, 150, 24);
    clipSyncRow.setBounds (940, 264, 224, 24);

    // SETUP
    roleRow.setBounds (776, 64, 318, 24);
    roleAuto.setBounds (1100, 64, 64, 24);
    makeLead.setBounds (980, 96, 90, 24);
    canLead.setBounds (1076, 96, 88, 24);
    for (int i = 0; i < inputKnobs.size(); ++i) inputKnobs[i]->setBounds (776 + 97 * i, 148, 93, 66);
    levelRow.setBounds (1068, 164, 96, 24);
    signalSpectrum = { 776, 250, 240, 60 };
    signalBands = { 1028, 250, 136, 40 };
    signalLamps = { 1028, 296, 136, 16 };
    locateEngine.setBounds (1070, 404, 94, 24);
}

void VJAnalyzerEditor::layoutSource()
{
    sourceRole.setBounds (12, 60, 280, 24);
    sourceAuto.setBounds (296, 60, 52, 24);
    for (int i = 0; i < sourceKnobs.size(); ++i)
        sourceKnobs[i]->setBounds (88 + 68 * i, 130, 64, 66);
    sourceMakeLead.setBounds (268, 208, 80, 24);
}

//==============================================================================
void VJAnalyzerEditor::openEngine()
{
    if (EngineLauncher::bringEngineToFront())
        return;

    juce::File exe (processor.getState().state.getProperty ("enginePath").toString());
    if (exe.existsAsFile())
    {
        launchEngineAt (exe);
        return;
    }

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
    auto picker = std::make_unique<ColourPicker> (start, [this, index] (juce::Colour c) { processor.setCustomColour (index, c); });
    juce::CallOutBox::launchAsynchronously (std::move (picker), swatches[index]->getScreenBounds(), nullptr);
}

void VJAnalyzerEditor::loadMedia (int slot)
{
    juce::File start (processor.getMediaPath (slot));
    if (! start.existsAsFile())
        for (int s = 0; s < 8 && ! start.existsAsFile(); ++s)
            start = juce::File (processor.getMediaPath (s));
    mediaChooser = std::make_unique<juce::FileChooser> ("Load an image or a short clip", start.existsAsFile() ? start.getParentDirectory() : juce::File(),
                                                        "*.png;*.jpg;*.jpeg;*.mp4;*.mov;*.m4v;*.avi;*.wmv;*.mkv;*.webm");
    mediaChooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                               [this, slot] (const juce::FileChooser& chooser) {
                                   auto picked = chooser.getResult();
                                   if (picked.existsAsFile())
                                   {
                                       processor.setMediaPath (slot, picked.getFullPathName());
                                       setParameter ("mediaSlot", (float) (slot + 1));
                                       ensureMediaScene();
                                   }
                               });
}

bool VJAnalyzerEditor::isInterestedInFileDrag (const juce::StringArray& files)
{
    for (auto& f : files)
        if (juce::File (f).hasFileExtension ("png;jpg;jpeg;mp4;mov;m4v;avi;wmv;mkv;webm"))
            return true;
    return false;
}

void VJAnalyzerEditor::filesDropped (const juce::StringArray& files, int x, int y)
{
    int slot = processor.getMediaSlot();
    for (int i = 0; i < mediaCards.size(); ++i)
        if (mediaCards[i]->isVisible() && mediaCards[i]->getBounds().contains (x, y))
            slot = i;
    for (auto& f : files)
        if (juce::File (f).hasFileExtension ("png;jpg;jpeg;mp4;mov;m4v;avi;wmv;mkv;webm"))
        {
            processor.setMediaPath (slot, f);
            setParameter ("mediaSlot", (float) (slot + 1));
            ensureMediaScene();
            setInfo ("Loaded " + juce::File (f).getFileName() + " into slot " + juce::String (slot + 1) + ".", vjui::bone);
            return;
        }
}

void VJAnalyzerEditor::ensureMediaScene()
{
    if (! status.connected || sceneUsesMedia (status.presetIndex))
        return;
    int target = status.presetNames.indexOf ("Media Negative");
    for (int i = 0; target < 0 && i < status.presetNames.size(); ++i)
        if (sceneUsesMedia (i))
            target = i;
    if (target >= 0)
    {
        selectPreset (target);
        setInfo ("Switched to " + status.presetNames[target] + " so your image shows.", vjui::bone);
    }
}

void VJAnalyzerEditor::selectPreset (int engineIndex)
{
    if (status.numPresets > 0)
        engineIndex = juce::jlimit (0, status.numPresets - 1, engineIndex);
    processor.fireScene (engineIndex);
}

juce::String VJAnalyzerEditor::recipeName (int palette, int look) const
{
    return VJAnalyzerProcessor::lookPresetNames[juce::jlimit (0, 6, look)] + juce::String (juce::CharPointer_UTF8 (" \xc2\xb7 "))
         + VJAnalyzerProcessor::paletteNames[juce::jlimit (0, VJAnalyzerProcessor::paletteNames.size() - 1, palette)];
}

void VJAnalyzerEditor::momentClicked (int slot, const juce::MouseEvent* rightClick)
{
    if (rightClick != nullptr)
    {
        juce::PopupMenu m;
        m.addItem (1, "Save here");
        m.addItem (2, "Clear", processor.hasSnapshot (slot));
        m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (moments[slot]), [this, slot] (int r) {
            if (r == 1) { processor.storeSnapshot (slot); savedSlot = slot; savedFlash = now(); }
            if (r == 2) processor.clearSnapshot (slot);
        });
        return;
    }
    if (processor.hasSnapshot (slot))
        setParameter ("snap" + juce::String (slot + 1), 1.0f);
    else
    {
        processor.storeSnapshot (slot);
        savedSlot = slot;
        savedFlash = now();
        setInfo ("Saved as moment " + juce::String::charToString ((juce::juce_wchar) ('A' + slot)) + " ("
                 + recipeName ((int) param ("palette"), (int) param ("lookPreset")) + "). Click it to come back here.", vjui::tungsten);
    }
}

void VJAnalyzerEditor::showPaletteMenu()
{
    juce::PopupMenu m;
    const int order[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 13, 11, 12 };
    const auto current = (int) param ("palette");
    for (auto i : order)
    {
        auto colours = VJAnalyzerProcessor::isPresetPalette (i) ? VJAnalyzerProcessor::presetPalette (i) : processor.getPaletteColours();
        if (i == VJAnalyzerProcessor::sceneColours)
            colours = { juce::Colour (0xff202020), juce::Colour (0xff606060), juce::Colour (0xffb0b0b0) };
        m.addCustomItem (i + 1, std::make_unique<PaletteItem> (colours, VJAnalyzerProcessor::paletteNames[i], i == current), nullptr);
    }
    m.addSeparator();
    m.addItem (100, "Edit colours...");
    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&colourPicker), [this] (int r) {
        if (r >= 1 && r <= 14)
            setParameter ("palette", (float) (r - 1));
        else if (r == 100)
        {
            processor.getState().state.setProperty ("drawerOpen", true, nullptr);
            showTab (colourTab);
            updateMode();
        }
    });
}

void VJAnalyzerEditor::showLookMenu()
{
    juce::PopupMenu m;
    const auto current = (int) param ("lookPreset");
    for (int i = 1; i < VJAnalyzerProcessor::lookPresetNames.size(); ++i)
        m.addItem (i, VJAnalyzerProcessor::lookPresetNames[i], true, i == current);
    m.addSeparator();
    m.addItem (50, "Custom (edited)", false, current == VJAnalyzerProcessor::customLook);
    m.addItem (100, "Edit look...");
    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&lookPicker), [this] (int r) {
        if (r >= 1 && r <= 6)
            setParameter ("lookPreset", (float) r);
        else if (r == 100)
        {
            processor.getState().state.setProperty ("drawerOpen", true, nullptr);
            showTab (lookTab);
            updateMode();
        }
    });
}

void VJAnalyzerEditor::showMorphMenu()
{
    juce::PopupMenu m;
    const auto current = (int) param ("morphTime");
    for (int i = 0; i < VJAnalyzerProcessor::morphNames.size(); ++i)
        m.addItem (i + 1, VJAnalyzerProcessor::morphNames[i], true, i == current);
    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&morphButton), [this] (int r) {
        if (r >= 1)
            setParameter ("morphTime", (float) (r - 1));
    });
}

juce::StringArray VJAnalyzerEditor::cachedSceneNames() const
{
    return juce::StringArray::fromLines (processor.getState().state.getProperty ("sceneCache").toString());
}

void VJAnalyzerEditor::rememberScenes()
{
    auto joined = status.presetNames.joinIntoString ("\n");
    auto& s = processor.getState().state;
    if (joined.isNotEmpty() && s.getProperty ("sceneCache").toString() != joined)
        s.setProperty ("sceneCache", joined, nullptr);
}

//==============================================================================
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
        auto word = status.connected ? status.macroLabels[slot] : juce::String();
        if (word.equalsIgnoreCase (VJAnalyzerProcessor::macroNames[slot]))
            word = {};
        macros[slot]->setSubLabel (word, vjui::ash);
    }

    juce::String speedText;
    auto speedColour = vjui::ash;
    if (status.connected)
    {
        auto s = status.speed;
        if (std::abs (s) < 0.005f) { speedText = "FROZEN"; speedColour = vjui::signal; }
        else speedText = "now x" + juce::String (std::abs (s), 2) + (s < 0.0f ? " rev" : "");
    }
    macros[1]->setSubLabel (speedText, speedColour);
    macros[6]->setSubLabel (paramText ("macro7"), vjui::dust);

    // REACT: the word for the amount, and "still" during STILL.
    const auto r = param ("macro5");
    juce::String word = r < 0.01f ? "still" : r <= 0.15f ? "breathing" : r <= 0.40f ? "gentle" : r <= 0.60f ? "as designed" : r <= 0.85f ? "strong" : "wild";
    if (param ("calm") > 0.5f)
        word = "STILL";
    macros[4]->setSubLabel (word, param ("calm") > 0.5f ? vjui::tungsten : vjui::ash);
}

void VJAnalyzerEditor::setInfo (const juce::String& text, juce::Colour colour)
{
    stateHint = text;
    stateColour = colour;
    infoHoldUntil = now() + 4.0; // the message stays up for a few seconds (updateInfoLine)
}

void VJAnalyzerEditor::updateInfoLine()
{
    juce::String text;
    auto colour = vjui::dust;

    if (auto* under = juce::Desktop::getInstance().getMainMouseSource().getComponentUnderMouse();
        under != nullptr && (under == this || isParentOf (under)))
        for (auto* c = under; c != nullptr && c != this; c = c->getParentComponent())
            if (auto h = c->getProperties()["help"].toString(); h.isNotEmpty())
            {
                text = h;
                colour = vjui::ash;
                break;
            }

    if (text.isEmpty() && now() < infoHoldUntil)
    {
        text = stateHint;
        colour = stateColour;
    }

    if (text.isEmpty())
    {
        const auto cue = processor.getCuedScene(), fired = processor.getFiredScene();
        const bool starting = ! status.connected && now() - launchedAt < 8.0;
        if (! status.connected)
            text = starting ? "Starting the engine..." : "Start the engine (top left) to see the picture.", colour = starting ? vjui::tungsten : vjui::bone;
        else if (status.blackout)
            text = "Output is dark. Click BLACKOUT to bring it back.", colour = vjui::signal;
        else if (fired >= 0 && fired != status.presetIndex && juce::isPositiveAndBelow (fired, status.presetNames.size()))
            text = "Switching to " + status.presetNames[fired] + " on the next beat...", colour = vjui::tungsten;
        else if (cue >= 0 && juce::isPositiveAndBelow (cue, status.presetNames.size()))
            text = status.presetNames[cue] + " is cued - press GO.", colour = vjui::tungsten;
        else if (! meters.frame.gateOpen)
            text = "SILENT - play something in Live (the picture rests).";
        else if (param ("calm") > 0.5f)
            text = "STILL: reactions faded out. Click BREATHE, PULSE or PUNCH to bring them back.", colour = vjui::tungsten;
        else if (status.rest > 0.5f)
            text = "Resting: the music is quiet, so the picture has gone dark. The next hit wakes it.";
        else if (status.tension > 0.35f)
            text = "Build-up " + juce::String (juce::roundToInt (status.tension * 100.0f)) + "% - the drop will release it (or press DROP).";
        else
        {
            const auto slot = processor.getMediaSlot();
            auto name = juce::File (processor.getMediaPath (slot)).getFileName();
            if (name.isNotEmpty() && ! sceneUsesMedia (status.presetIndex))
                text = status.presetName + " doesn't show your image - pick a scene marked IMG.";
        }
    }

    if (text != infoText || colour != infoColour)
    {
        infoText = text;
        infoColour = colour;
        repaint (infoArea);
    }
}

void VJAnalyzerEditor::timerCallback()
{
    meters = processor.getWorker().getMeters();
    status = processor.getWorker().getEngineStatus();
    if (status.connected)
        rememberScenes();

    // Lead / source / drawer.
    {
        const auto desired = ! processor.isLead() ? Mode::source
                           : (bool) processor.getState().state.getProperty ("drawerOpen", false) ? Mode::edit : Mode::play;
        if (desired != mode)
            updateMode();
    }

    if (mode == Mode::source)
    {
        const auto role = (int) param ("role");
        sourceRole.setSelected (role);
        sourceAuto.setToggleState (processor.isRoleAuto(), juce::dontSendNotification);
        repaint();
        return;
    }

    // --- header
    const bool starting = ! status.connected && now() - launchedAt < 8.0;
    if (status.connected)
    {
        auto beat = (int) std::floor (meters.transport.ppqPosition);
        auto barLen = juce::jmax (1, meters.transport.meterNumerator * 4 / juce::jmax (1, meters.transport.meterDenominator));
        juce::String text = juce::String (status.blackout ? "OUTPUT DARK" : juce::String (juce::roundToInt (status.fps)) + " fps")
                          + juce::String (juce::CharPointer_UTF8 ("  \xc2\xb7  ")) + juce::String (status.bpm, 1) + " BPM";
        if (meters.transport.valid)
            text << juce::String (juce::CharPointer_UTF8 ("  \xc2\xb7  ")) << (beat / barLen + 1) << "." << (beat % barLen + 1);
        if (status.renderScale < 0.99f)
            text << juce::String (juce::CharPointer_UTF8 ("  \xc2\xb7  ")) << juce::roundToInt (status.renderScale * 100.0f) << "%";
        enginePill.setButtonText (text);
        styleButton (enginePill, vjui::ink1, vjui::bone, vjui::ink1, vjui::bone);
        vjui::setHelp (enginePill, "The picture engine. Click to bring its window to the front.");
    }
    else
    {
        enginePill.setButtonText (starting ? "STARTING..." : "START ENGINE  -  click here");
        if (starting) styleButton (enginePill, vjui::ink1, vjui::tungsten, vjui::ink1, vjui::tungsten);
        else          styleButton (enginePill, vjui::signal, vjui::ink0, vjui::signal, vjui::ink0);
        vjui::setHelp (enginePill, "The picture engine is not running. Click to start it.");
    }
    fullscreenButton.setEnabled (status.connected);
    {
        auto track = processor.getTrackName();
        leadChip.setButtonText ("LEAD" + (track.isNotEmpty() ? juce::String (juce::CharPointer_UTF8 (" \xc2\xb7 ")) + track : juce::String()));
    }
    editButton.setButtonText (mode == Mode::edit ? juce::String (juce::CharPointer_UTF8 ("EDIT \xe2\x97\x82")) : juce::String (juce::CharPointer_UTF8 ("EDIT \xe2\x96\xb8")));
    editButton.setToggleState (mode == Mode::edit, juce::dontSendNotification);

    // --- scenes
    const auto cue = processor.getCuedScene(), fired = processor.getFiredScene();
    {
        SceneGrid::State s;
        s.connected = status.connected;
        s.names = status.connected && ! status.presetNames.isEmpty() ? status.presetNames : cachedSceneNames();
        s.usesMedia = status.presetUsesMedia;
        s.live = status.presetIndex;
        s.cued = cue;
        s.firing = fired;
        s.descriptions.clear();
        if (juce::isPositiveAndBelow (status.presetIndex, s.names.size()))
        {
            for (int i = 0; i < s.names.size(); ++i)
                s.descriptions.add (i == status.presetIndex ? status.sceneDescription : juce::String());
        }
        sceneGrid.setState (s);
    }
    go.setToggleState (cue >= 0, juce::dontSendNotification);
    {
        juce::String text;
        auto textColour = vjui::dust;
        auto fill = vjui::ink0;
        auto outline = vjui::line;
        if (fired >= 0 && fired != status.presetIndex && juce::isPositiveAndBelow (fired, status.presetNames.size()))
        {
            text = status.presetNames[fired].toUpperCase() + juce::String (juce::CharPointer_UTF8 ("  \xe2\x96\xb8  on the next beat"));
            textColour = vjui::tungsten;
            fill = vjui::ink0.interpolatedWith (vjui::tungsten, 0.15f);
            outline = vjui::tungsten;
        }
        else if (cue >= 0 && juce::isPositiveAndBelow (cue, status.presetNames.size()))
        {
            text = juce::String (juce::CharPointer_UTF8 ("NEXT \xe2\x96\xb8 ")) + status.presetNames[cue].toUpperCase() + "   press GO";
            textColour = vjui::tungsten;
            outline = vjui::tungsten;
        }
        else
            text = status.connected ? "click a scene to cue it" : "the engine is not running";
        cueField.setButtonText (text);
        styleButton (cueField, fill, textColour, fill, textColour);
        outlineOf (cueField, outline);
    }

    // --- react row, lamps, style
    reactRow.setSelected (param ("calm") > 0.5f ? 0 : 1 + (int) param ("reactStyle"));
    reactRow2.setSelected (reactRow.getSelected());
    {
        const auto palette = (int) param ("palette");
        colourPicker.setButtonText (VJAnalyzerProcessor::paletteNames[palette]
                                    + juce::String (juce::CharPointer_UTF8 (" \xe2\x96\xbe")));
        const auto look = (int) param ("lookPreset");
        lookPicker.setButtonText (juce::String ("LOOK  ") + VJAnalyzerProcessor::lookPresetNames[look]
                                  + juce::String (juce::CharPointer_UTF8 (" \xe2\x96\xbe")));
        lookRow.setSelected (look - 1);
        for (auto* card : paletteCards)
            card->setToggleState ((int) card->getProperties()["palette"] == palette, juce::dontSendNotification);
    }
    {
        const auto slot = processor.getMediaSlot();
        auto name = juce::File (processor.getMediaPath (slot)).getFileName();
        if (name.isEmpty() && status.connected)
            name = status.media[(size_t) slot].name;
        mediaChip.setButtonText ("IMAGE " + juce::String (slot + 1) + "   "
                                 + (name.isEmpty() ? juce::String ("none - drop a file here") : name));
        styleButton (mediaChip, vjui::ink1, name.isEmpty() ? vjui::dust : vjui::bone, vjui::ink1, vjui::bone);
        for (int i = 0; i < mediaCards.size(); ++i)
        {
            auto file = juce::File (processor.getMediaPath (i));
            auto label = file.getFileName();
            if (label.isEmpty() && status.connected)
                label = status.media[(size_t) i].name;
            mediaCards[i]->setButtonText (juce::String (i + 1) + (label.isEmpty() ? juce::String ("\nempty") : "\n" + label));
            mediaCards[i]->setToggleState (i == slot, juce::dontSendNotification);
            styleButton (*mediaCards[i], vjui::ink1, label.isEmpty() ? vjui::dust : vjui::ash, vjui::ink2, vjui::bone);
            outlineOf (*mediaCards[i], i == slot ? vjui::bone : vjui::line);
        }
        clipModeRow.setSelected ((int) param ("clipMode"));
        clipSyncRow.setSelected ((int) param ("clipSync"));
    }
    morphButton.setButtonText (juce::String (juce::CharPointer_UTF8 ("morph \xc2\xb7 ")) + VJAnalyzerProcessor::morphNames[(int) param ("morphTime")].toLowerCase()
                               + juce::String (juce::CharPointer_UTF8 (" \xe2\x96\xbe")));
    for (int i = 0; i < moments.size(); ++i)
    {
        int palette = 0, look = 0;
        const bool stored = processor.getSnapshotRecipe (i, palette, look);
        const bool active = i == processor.getActiveSnapshot();
        moments[i]->setInfo (stored, active, stored ? recipeName (palette, look) : juce::String(),
                             active ? processor.getMorphProgress() : -1.0f, i == savedSlot && now() - savedFlash < 0.8);
        vjui::setHelp (*moments[i], stored ? "Moment " + juce::String::charToString ((juce::juce_wchar) ('A' + i)) + " (" + recipeName (palette, look)
                                                 + "). Click to blend to it. Right-click: save here / clear. (Live: Snapshot "
                                                 + juce::String::charToString ((juce::juce_wchar) ('A' + i)) + ")"
                                           : "Empty. Click to save the knobs, look, colour and reactions here.");
    }

    // --- drawer state
    {
        const float cut = param ("cutRate");
        cutRow.setSelected (cut < 0.04f ? 0 : cut < 0.2f ? 1 : cut < 0.4f ? 2 : cut < 0.6f ? 3 : cut < 0.75f ? 4 : cut < 0.9f ? 5 : 6);
        roleRow.setSelected ((int) param ("role"));
        roleAuto.setToggleState (processor.isRoleAuto(), juce::dontSendNotification);
        levelRow.setSelected ((int) param ("normalizer"));
        auto colours = processor.getPaletteColours();
        for (int i = 0; i < swatches.size(); ++i)
        {
            styleButton (*swatches[i], colours[(size_t) i], colours[(size_t) i].getPerceivedBrightness() > 0.5f ? vjui::ink0 : vjui::bone,
                         colours[(size_t) i], vjui::ink0);
        }
    }

    // --- knobs
    updateKnobLabels();
    for (int i = 0; i < macros.size(); ++i)
        macros[i]->setActivity (status.connected ? (i == 4 ? juce::jmax (status.macroActivity[4], status.accentEnv) : status.macroActivity[(size_t) i]) : 0.0f);

    if (status.drops != lastDrops)
    {
        if (lastDrops != 0 || status.drops == 1)
            dropFlash = now();
        lastDrops = status.drops;
    }

    updateInfoLine();
    repaint (0, 40, playW, 110);   // scene name / description / react hint
    repaint (480, 92, 264, 26);    // lamps
    repaint (drop.getBounds().expanded (4));
    repaint (hit.getBounds().expanded (4));
    repaint (colourPicker.getBounds());
    repaint (480, 168, 264, 28);   // look amount value
    if (mode == Mode::edit)
        repaint (drawerArea);
}

//==============================================================================
void VJAnalyzerEditor::paint (juce::Graphics& g)
{
    g.fillAll (vjui::ink0);
    if (mode == Mode::source)
    {
        paintSource (g);
        return;
    }
    paintHeader (g);
    paintPlay (g);
    if (mode == Mode::edit)
        paintDrawer (g);

    g.setColour (infoColour);
    g.setFont (vjui::font (10.5f));
    g.drawText (infoText, infoArea, juce::Justification::centredLeft, true);
}

void VJAnalyzerEditor::paintHeader (juce::Graphics& g)
{
    g.setColour (vjui::ink1);
    g.fillRect (0, 0, getWidth(), 40);
    g.setColour (vjui::line);
    g.fillRect (0, 40, getWidth(), 1);
    g.setColour (vjui::bone);
    g.setFont (vjui::font (14.0f, true));
    g.drawText ("VJ ANALYZER", 16, 10, 104, 20, juce::Justification::centredLeft);
}

void VJAnalyzerEditor::paintPlay (juce::Graphics& g)
{
    // Scene: LIVE tag, name, description.
    caption (g, "Scene", 16, 48);
    hint (g, juce::String (juce::CharPointer_UTF8 ("click = cue \xc2\xb7 GO = switch \xc2\xb7 double-click = both")), { 200, 48, 264, 12 },
          juce::Justification::centredRight);
    if (status.connected)
    {
        auto tag = juce::Rectangle<float> (16.0f, 70.0f, status.blackout ? 72.0f : 32.0f, 14.0f);
        g.setColour (vjui::signal);
        g.fillRoundedRectangle (tag, 2.0f);
        g.setColour (vjui::ink0);
        g.setFont (vjui::font (9.0f, true));
        g.drawText (status.blackout ? "OUTPUT DARK" : "LIVE", tag.toNearestInt(), juce::Justification::centred);
    }
    g.setColour (status.connected ? vjui::bone : vjui::dust);
    g.setFont (vjui::font (20.0f, true));
    g.drawText (status.connected ? status.presetName : juce::String ("No engine"), ! status.connected ? 16 : (status.blackout ? 96 : 56), 62, 400, 28,
                juce::Justification::centredLeft, true);
    g.setColour (vjui::dust);
    g.setFont (vjui::font (10.5f));
    g.drawText (status.connected ? status.sceneDescription
                                 : juce::String (juce::CharPointer_UTF8 ("1  Start the engine  \xc2\xb7  2  Play something in Live  \xc2\xb7  3  Pick a scene, press GO")),
                16, 94, 448, 16, juce::Justification::centredLeft, true);

    // REACT caption + live state on the right.
    caption (g, "React", 480, 48);
    {
        juce::String state = "how the picture follows the music";
        auto colour = vjui::dust;
        if (now() - dropFlash < 1.2) { state = "DROP"; colour = vjui::signal; }
        else if (param ("calm") > 0.5f) { state = "still - reactions faded"; colour = vjui::tungsten; }
        else if (status.connected && status.rest > 0.5f) state = "resting (quiet)";
        else if (status.connected && status.tension > 0.35f)
        {
            state = "build-up";
            colour = vjui::halation;
            g.setColour (vjui::line);
            g.fillRect (700, 53, 44, 3);
            g.setColour (vjui::halation);
            g.fillRect (700, 53, (int) (44 * status.tension), 3);
        }
        g.setColour (colour);
        g.setFont (vjui::font (10.0f, colour != vjui::dust));
        g.drawText (state, 540, 48, colour == vjui::halation ? 156 : 204, 12, juce::Justification::centredRight);
    }

    caption (g, "Style", 480, 126);
    caption (g, "Look amount", 480, 176, 90);
    g.setColour (vjui::bone);
    g.setFont (vjui::mono (10.5f));
    g.drawText (juce::String (juce::roundToInt (param ("lookAmount") * 100.0f)) + "%", 706, 172, 38, 20, juce::Justification::centredRight);
    caption (g, "Moments", 480, 244);

    // Bottom band.
    g.setColour (vjui::line);
    g.fillRect (16, 306, playW - 32, 1);
    caption (g, juce::String (juce::CharPointer_UTF8 ("Shape \xc2\xb7 this scene")), 16, 318);
    caption (g, "Motion", 392, 318);
    caption (g, "React", 552, 318);
    g.setColour (vjui::line);
    g.fillRect (382, 318, 1, 110);
    g.fillRect (542, 318, 1, 110);
}

void VJAnalyzerEditor::paintOverChildren (juce::Graphics& g)
{
    if (mode == Mode::source)
        return;

    // Engine dot inside the pill.
    {
        auto dot = juce::Rectangle<float> (134.0f, 15.0f, 10.0f, 10.0f);
        g.setColour (status.connected ? vjui::signal : (now() - launchedAt < 8.0 ? vjui::tungsten : vjui::ink0));
        g.fillEllipse (dot);
        if (! status.connected && now() - launchedAt >= 8.0)
        {
            juce::Path play; // a small "start" triangle
            play.addTriangle (dot.getX() + 2.5f, dot.getY() + 1.5f, dot.getX() + 2.5f, dot.getBottom() - 1.5f, dot.getRight() - 0.5f, dot.getCentreY());
            g.setColour (vjui::signal);
            g.fillPath (play);
        }
    }

    // Palette stripes in the colour picker.
    {
        auto r = colourPicker.getBounds().toFloat();
        stripes (g, juce::Rectangle<float> (r.getX() + 9.0f, r.getCentreY() - 4.0f, 30.0f, 8.0f), processor.getPaletteColours());
    }

    // Lamps: a dot that flashes with each hit (kick / snare / hat), bars for bass and level.
    {
        const double last[] = { meters.lastOnset[0], meters.lastOnset[1], meters.lastOnset[2] };
        for (int i = 0; i < lamps.size(); ++i)
        {
            auto r = lamps[i]->getBounds().toFloat();
            const bool open = lamps[i]->getToggleState();
            if (i < 3)
            {
                const auto level = open ? (float) juce::jlimit (0.0, 1.0, 1.0 - (now() - last[i]) / 0.2) : 0.0f;
                auto d = juce::Rectangle<float> (r.getX() + 6.0f, r.getCentreY() - 3.0f, 6.0f, 6.0f);
                g.setColour (open ? vjui::line.interpolatedWith (vjui::signal, level) : vjui::ink0);
                g.fillEllipse (d);
                g.setColour (open ? vjui::ash : vjui::dust);
                g.drawEllipse (d, 1.0f);
            }
            else
            {
                const auto v = open ? juce::jlimit (0.0f, 1.0f, i == 3 ? meters.frame.aggregateRel[0] : meters.frame.levelRel) : 0.0f;
                auto bar = juce::Rectangle<float> (r.getX() + 5.0f, r.getCentreY() - 2.0f, 9.0f, 4.0f);
                g.setColour (vjui::line);
                g.fillRect (bar);
                g.setColour (vjui::halation);
                g.fillRect (bar.withWidth (8.0f * v));
            }
            if (! open)
            {
                g.setColour (vjui::dust);
                g.drawLine (r.getX() + 17.0f, r.getCentreY(), r.getRight() - 4.0f, r.getCentreY(), 1.0f);
            }
        }
    }

    // HIT / DROP flashes.
    if (now() - hitFlash < 0.15)
    {
        g.setColour (vjui::signal.withAlpha (0.25f));
        g.fillRoundedRectangle (hit.getBounds().toFloat(), 2.0f);
    }
    if (now() - dropFlash < 0.6)
    {
        g.setColour (vjui::signal.withAlpha (0.35f * (float) (1.0 - (now() - dropFlash) / 0.6)));
        g.fillRoundedRectangle (drop.getBounds().toFloat(), 2.0f);
    }

    // BLACKOUT: a steady red frame around the whole window (never blinks).
    if (status.blackout || param ("blackout") > 0.5f)
    {
        g.setColour (vjui::signal);
        g.drawRect (getLocalBounds(), 2);
    }

    // Drawer extras drawn over their buttons.
    if (mode == Mode::edit && currentTab == colourTab)
        for (auto* card : paletteCards)
        {
            const auto index = (int) card->getProperties()["palette"];
            auto colours = VJAnalyzerProcessor::isPresetPalette (index) ? VJAnalyzerProcessor::presetPalette (index) : processor.getPaletteColours();
            if (index == VJAnalyzerProcessor::sceneColours)
                colours = { juce::Colour (0xff202020), juce::Colour (0xff606060), juce::Colour (0xffb0b0b0) };
            auto r = card->getBounds().toFloat().reduced (6.0f);
            stripes (g, r.removeFromTop (20.0f), colours);
        }
}

void VJAnalyzerEditor::paintDrawer (juce::Graphics& g)
{
    g.setColour (vjui::line);
    g.fillRect (playW, 40, 1, height - 40);

    switch (currentTab)
    {
        case lookTab:
            caption (g, "Amount", 776, 86, 70);
            g.setColour (vjui::bone);
            g.setFont (vjui::mono (10.5f));
            g.drawText (juce::String (juce::roundToInt (param ("lookAmount") * 100.0f)) + "%", 1124, 82, 40, 20, juce::Justification::centredRight);
            caption (g, juce::String (juce::CharPointer_UTF8 ("Film \xc2\xb7 35 mm physics")), 776, 112);
            caption (g, juce::String (juce::CharPointer_UTF8 ("Digital \xc2\xb7 disturbances")), 776, 204);
            caption (g, juce::String (juce::CharPointer_UTF8 ("On accents \xc2\xb7 overlay")), 776, 296);
            hint (g, "Turning a knob makes the look CUSTOM.", { 776, 404, 270, 24 });
            break;
        case colourTab:
            caption (g, "Custom colours", 776, 268);
            g.setColour (vjui::dust);
            g.setFont (vjui::font (10.0f));
            g.drawFittedText ("Click a colour to change it; the palette becomes Custom. Scene Colors = every scene keeps its own colours.",
                              776, 336, 388, 28, juce::Justification::topLeft, 2);
            break;
        case reactTab:
        {
            const char* sentences[] = { "STILL: reactions fade out over one bar; the picture moves on its own.",
                                        "BREATHE: the picture swells with the music; only the big moments land; silence goes very dark.",
                                        "PULSE: every beat is felt, strong hits land, silence rests.",
                                        "PUNCH: tight, sharp hits, more of them; the picture stays brighter in silence." };
            g.setColour (vjui::ash);
            g.setFont (vjui::font (10.5f));
            g.drawFittedText (sentences[juce::jlimit (0, 3, reactRow2.getSelected())], 776, 82, 388, 30, juce::Justification::topLeft, 2);
            caption (g, "Amount and feel", 776, 116);
            caption (g, "Follows", 776, 208);
            hint (g, "click to ignore a channel", { 900, 208, 264, 12 }, juce::Justification::centredRight);
            caption (g, "Right now", 776, 262);
            g.setColour (vjui::bone);
            g.setFont (vjui::mono (10.5f));
            juce::String line = status.connected
                ? "accents " + juce::String (status.accents) + "   drops " + juce::String (status.drops)
                      + "   rest " + juce::String (juce::roundToInt (status.rest * 100.0f)) + "%   build-up "
                      + juce::String (juce::roundToInt (status.tension * 100.0f)) + "%"
                : juce::String ("engine not running");
            g.drawText (line, 776, 278, 388, 16, juce::Justification::centredLeft);
            g.setColour (vjui::dust);
            g.setFont (vjui::font (10.0f));
            g.drawFittedText ("An accent = a hit strong enough to become an event. A drop = the big release after a build-up.",
                              776, 298, 388, 28, juce::Justification::topLeft, 2);
            break;
        }
        case motionTab:
        {
            caption (g, "Camera", 776, 48);
            caption (g, "Direction and tempo", 880, 48);
            caption (g, "Auto scene cuts", 776, 148);
            hint (g, "switches scenes by itself; GO still works", { 900, 148, 264, 12 }, juce::Justification::centredRight);
            hint (g, "Counted in beats. HIT = every accent (max 3 a second). Paused while STILL.", { 776, 196, 388, 14 });
            caption (g, "Live readout", 776, 236);
            g.setColour (vjui::bone);
            g.setFont (vjui::mono (10.5f));
            g.drawText (status.connected ? "scene clock x" + juce::String (status.speed, 2) + "   " + juce::String (status.bpm, 1) + " BPM"
                                               + (status.followingHost ? "   following Live" : "   free-running")
                                         : juce::String ("engine not running"),
                        776, 252, 388, 16, juce::Justification::centredLeft);
            hint (g, "Speed and Glide are on the main panel (MOTION).", { 776, 276, 388, 14 });
            break;
        }
        case mediaTab:
        {
            caption (g, "Clip", 776, 248);
            caption (g, "Length", 940, 248);
            caption (g, "Shown by", 776, 300);
            juce::StringArray shows;
            for (int i = 0; i < status.presetNames.size(); ++i)
                if (sceneUsesMedia (i))
                    shows.add (status.presetNames[i]);
            g.setColour (vjui::ash);
            g.setFont (vjui::font (10.5f));
            g.drawFittedText (shows.isEmpty() ? juce::String ("Media Negative, Media Lines (and with USE IN ALL: every scene marked IMG)")
                                              : shows.joinIntoString (", "),
                              776, 314, 388, 30, juce::Justification::topLeft, 2);
            const auto slot = processor.getMediaSlot();
            const auto& m = status.media[(size_t) slot];
            g.setColour (vjui::bone);
            g.setFont (vjui::mono (10.0f));
            g.drawText ("slot " + juce::String (slot + 1) + "  " + (m.name.isEmpty() ? juce::String ("empty")
                                                                   : m.name + (m.loading ? "  loading..." : "  " + juce::String (m.width) + "x" + juce::String (m.height))),
                        776, 352, 388, 14, juce::Justification::centredLeft, true);
            hint (g, "Drop a file on a slot to load it there.", { 776, 372, 388, 14 });
            break;
        }
        case setupTab:
        {
            caption (g, "This instance", 776, 48);
            hint (g, processor.getTrackName().isNotEmpty() ? "track '" + processor.getTrackName() + "'" : juce::String ("track name unknown"),
                  { 900, 48, 264, 12 }, juce::Justification::centredRight);
            g.setColour (vjui::bone);
            g.setFont (vjui::font (10.5f, true));
            g.drawText (processor.isLead() ? "LEAD - this window plays the picture" : "SOURCE", 776, 96, 200, 24, juce::Justification::centredLeft);
            caption (g, "Input", 776, 132);
            hint (g, "automatic unless you turn a knob", { 900, 132, 264, 12 }, juce::Justification::centredRight);
            caption (g, "Level", 1068, 148, 96);
            caption (g, "Signal", 776, 236);
            paintSignal (g, signalSpectrum, signalBands, signalLamps);
            caption (g, "Engine", 776, 386);
            g.setColour (vjui::dust);
            g.setFont (vjui::mono (9.5f));
            g.drawText (processor.getState().state.getProperty ("enginePath").toString(), 776, 404, 288, 24, juce::Justification::centredLeft, true);
            break;
        }
        default:
            break;
    }
}

void VJAnalyzerEditor::paintSignal (juce::Graphics& g, juce::Rectangle<int> spectrumArea, juce::Rectangle<int> bands, juce::Rectangle<int> lampArea)
{
    auto& f = meters.frame;
    auto spec = spectrumArea.toFloat();
    g.setColour (vjui::ink1);
    g.fillRect (spec);
    auto barW = spec.getWidth() / 32.0f;
    for (int b = 0; b < 32; ++b)
    {
        auto v = juce::jlimit (0.0f, 1.0f, f.spectrum[(size_t) b]);
        g.setColour (vjui::ash.withAlpha (0.35f + 0.65f * v));
        g.fillRect (spec.getX() + b * barW + 1.0f, spec.getBottom() - v * (spec.getHeight() - 4.0f) - 2.0f, barW - 2.0f, v * (spec.getHeight() - 4.0f));
    }
    const char* names[] = { "LOW", "MID", "HIGH" };
    for (int i = 0; i < 3; ++i)
    {
        auto row = bands.removeFromTop (13);
        g.setColour (vjui::ash);
        g.setFont (vjui::font (9.0f, true));
        g.drawText (names[i], row.removeFromLeft (34), juce::Justification::centredLeft);
        auto bar = row.toFloat().reduced (0.0f, 4.0f);
        g.setColour (vjui::line);
        g.fillRect (bar);
        g.setColour (vjui::halation);
        g.fillRect (bar.withWidth (bar.getWidth() * juce::jlimit (0.0f, 1.0f, f.aggregateRel[(size_t) i])));
    }
    const double last[] = { meters.lastOnset[0], meters.lastOnset[1], meters.lastOnset[2], meters.lastMidi };
    const char* lampNames[] = { "LOW", "MID", "HIGH", "MIDI" };
    const auto w = lampArea.getWidth() / 4;
    for (int i = 0; i < 4; ++i)
    {
        auto cell = lampArea.removeFromLeft (w);
        const auto level = (float) juce::jlimit (0.0, 1.0, 1.0 - (now() - last[i]) / 0.2);
        g.setColour (vjui::line.interpolatedWith (vjui::signal, level));
        g.fillEllipse (cell.removeFromLeft (10).toFloat().withSizeKeepingCentre (7.0f, 7.0f));
        g.setColour (vjui::ash);
        g.setFont (vjui::font (8.5f, true));
        g.drawText (lampNames[i], cell, juce::Justification::centredLeft);
    }
    auto& t = meters.transport;
    juce::String line = f.gateOpen ? (f.calibrated ? "LIVE" : "CALIBRATING") : "SILENT";
    line << "   " << (f.levelDb > -150.0f ? juce::String (f.levelDb, 1) + " dBFS" : juce::String ("-inf"));
    if (t.valid)
        line << "   " << juce::String (t.bpm, 1) << " BPM" << (t.playing ? "" : " (stopped)");
    g.setColour (vjui::bone);
    g.setFont (vjui::mono (10.0f));
    g.drawText (line, 776, 318, 388, 14, juce::Justification::centredLeft);
}

void VJAnalyzerEditor::paintSource (juce::Graphics& g)
{
    g.setColour (vjui::ink1);
    g.fillRect (0, 0, sourceW, 36);
    g.setColour (vjui::line);
    g.fillRect (0, 36, sourceW, 1);
    g.setColour (vjui::bone);
    g.setFont (vjui::font (13.0f, true));
    g.drawText ("VJ ANALYZER", 12, 10, 100, 16, juce::Justification::centredLeft);
    g.setColour (vjui::ash);
    g.setFont (vjui::font (10.0f, true));
    g.drawText ("SOURCE", 112, 10, 60, 16, juce::Justification::centredLeft);
    g.setColour (status.connected ? vjui::signal : vjui::dust);
    g.fillEllipse (298.0f, 14.0f, 8.0f, 8.0f);
    g.setColour (vjui::ash);
    g.drawText ("engine", 310, 10, 44, 16, juce::Justification::centredLeft);

    caption (g, "This track", 12, 44);
    if (processor.getTrackName().isNotEmpty())
        hint (g, (processor.isRoleAuto() ? "auto from '" : "track '") + processor.getTrackName() + "'", { 150, 44, 198, 12 }, juce::Justification::centredRight);

    // Big hit lamp for this role's band.
    const auto role = (int) param ("role");
    const int band = role == 2 ? 1 : (role == 3 ? 2 : 0);
    const double last = juce::jmax (meters.lastOnset[(size_t) band], meters.lastMidi);
    const auto level = (float) juce::jlimit (0.0, 1.0, 1.0 - (now() - last) / 0.2);
    auto lamp = juce::Rectangle<float> (20.0f, 100.0f, 48.0f, 48.0f);
    g.setColour (vjui::line.interpolatedWith (vjui::signal, level));
    g.fillEllipse (lamp);
    g.setColour (vjui::ash);
    g.setFont (vjui::font (9.0f));
    const auto ago = now() - last;
    g.drawText (ago < 30.0 ? "last hit " + juce::String (ago, 1) + " s" : juce::String ("no hits yet"), 8, 154, 76, 12, juce::Justification::centred);

    caption (g, band == 0 ? "Low band" : (band == 1 ? "Mid band" : "High band"), 88, 96);
    auto meter = juce::Rectangle<float> (88.0f, 112.0f, 260.0f, 8.0f);
    g.setColour (vjui::line);
    g.fillRect (meter);
    g.setColour (vjui::halation);
    g.fillRect (meter.withWidth (meter.getWidth() * juce::jlimit (0.0f, 1.0f, meters.frame.aggregateRel[(size_t) band])));

    g.setColour (vjui::line);
    g.fillRect (12, 200, sourceW - 24, 1);
    g.setColour (vjui::ash);
    g.setFont (vjui::font (10.0f));
    auto leadTrack = processor.getLeadTrackName();
    g.drawFittedText ("Feeds the picture. Knobs, look and scenes are played in the lead"
                          + (leadTrack.isNotEmpty() ? " on '" + leadTrack + "'." : juce::String (".")),
                      12, 204, 250, 30, juce::Justification::centredLeft, 2);
}
