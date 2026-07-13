#include "PluginEditor.h"

namespace
{
constexpr auto accent = 0xffffb000;
constexpr auto panel = 0xff242831;
constexpr auto waveform = 0xfff1b935;
constexpr int designWidth = 820;
constexpr int designHeight = 875;
constexpr float minimumScale = 0.65f;
constexpr float maximumScale = 2.0f;
constexpr auto releasesUrl = "https://github.com/orange303-afk/SampleDicer/releases";
constexpr auto latestReleaseApi = "https://api.github.com/repos/orange303-afk/SampleDicer/releases/latest";
constexpr int64 updateCheckIntervalMs = 24 * 60 * 60 * 1000;

juce::PropertiesFile::Options updateSettingsOptions()
{
    juce::PropertiesFile::Options options;
    options.applicationName = "Sample Dicer Update Check";
    options.filenameSuffix = "settings";
    options.folderName = "Ilya Orange/Sample Dicer";
    options.osxLibrarySubFolder = "Application Support";
    return options;
}

bool isNewerVersion(juce::String candidate, juce::String current)
{
    const auto parse = [] (juce::String version)
    {
        version = version.trim().trimCharactersAtStart("vV");
        auto parts = juce::StringArray::fromTokens(version, ".", {});
        std::array<int, 4> numbers {};
        for (size_t i = 0; i < numbers.size() && i < static_cast<size_t>(parts.size()); ++i)
            numbers[i] = parts[static_cast<int>(i)].retainCharacters("0123456789").getIntValue();
        return numbers;
    };

    return parse(candidate) > parse(current);
}

class AboutPanel : public juce::Component
{
public:
    AboutPanel()
        : website("ilyaorange.tilda.ws", juce::URL("https://ilyaorange.tilda.ws")),
          bandcamp("ilyaorange.bandcamp.com", juce::URL("https://ilyaorange.bandcamp.com")),
          gumroad("ilyaorange.gumroad.com", juce::URL("https://ilyaorange.gumroad.com")),
          donate("Donate via PayPal", juce::URL("https://www.paypal.com/paypalme/ilyaorange303"))
    {
        title.setText("Sample Dicer " + juce::String(JucePlugin_VersionString) + " by Ilya Orange",
                      juce::dontSendNotification);
        title.setJustificationType(juce::Justification::centred);
        title.setFont(juce::FontOptions(15.0f, juce::Font::bold));
        title.setColour(juce::Label::textColourId, juce::Colour(0xfff0f1f4));
        addAndMakeVisible(title);
        for (auto* link : { &website, &bandcamp, &gumroad, &donate })
        {
            link->setColour(juce::HyperlinkButton::textColourId, juce::Colour(accent));
            addAndMakeVisible(link);
        }
        crypto.setMultiLine(true);
        crypto.setReadOnly(true);
        crypto.setScrollbarsShown(false);
        crypto.setColour(juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
        crypto.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
        crypto.setColour(juce::TextEditor::textColourId, juce::Colour(0xffd6dae2));
        crypto.setFont(juce::FontOptions(12.0f));
        crypto.setJustification(juce::Justification::centredTop);
        crypto.setText("BTC: 3PZSPAgXpLUtnH2LH9TmxjizVUETHPa9cW\n\n"
                       "ETH: 0x6144548f3f6071136fdf18134a99345cf12ae6b5\n\n"
                       "USDT ERC20: 0x0f49f2cddf673214646a3154f60aa0c63a414ad3",
                       false);
        addAndMakeVisible(crypto);
        setSize(520, 285);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour(0xff242831));
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(14);
        title.setBounds(area.removeFromTop(31));
        area.removeFromTop(3);
        for (auto* link : { &website, &bandcamp, &gumroad, &donate })
            link->setBounds(area.removeFromTop(27));
        area.removeFromTop(5);
        crypto.setBounds(area);
    }

private:
    juce::Label title;
    juce::HyperlinkButton website, bandcamp, gumroad, donate;
    juce::TextEditor crypto;
};
}

DicerLookAndFeel::DicerLookAndFeel()
{
    setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xffdfe3ea));
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour(juce::TextButton::textColourOffId, juce::Colour(0xffe8eaf0));
}

juce::String DisplaySlider::getTextFromValue(double value)
{
    return juce::String(value * displayScale, displayScale == 1.0 ? 1 : 0) + getTextValueSuffix();
}

double DisplaySlider::getValueFromText(const juce::String& text)
{
    return text.retainCharacters("-0123456789.").getDoubleValue() / displayScale;
}

DiceButton::DiceButton() : juce::Button("DICE") {}

void DiceButton::roll()
{
    auto next = face;
    while (next == face) next = 1 + random.nextInt(6);
    face = next;
    repaint();
}

void DiceButton::paintButton(juce::Graphics& g, bool highlighted, bool down)
{
    auto background = juce::Colour(accent);
    if (highlighted) background = background.brighter(0.08f);
    if (down) background = background.darker(0.10f);
    g.setColour(background);
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 8.0f);

    const auto buttonBounds = getLocalBounds().toFloat();
    const auto dieSize = juce::jmin(38.0f, buttonBounds.getHeight() - 16.0f);
    auto die = juce::Rectangle<float>(dieSize, dieSize).withCentre(buttonBounds.getCentre());
    g.setColour(juce::Colour(0xff17191e));
    g.fillRoundedRectangle(die, 7.0f);
    g.setColour(background);

    const auto left = die.getX() + dieSize * 0.27f;
    const auto centreX = die.getCentreX();
    const auto right = die.getRight() - dieSize * 0.27f;
    const auto top = die.getY() + dieSize * 0.27f;
    const auto centreY = die.getCentreY();
    const auto bottom = die.getBottom() - dieSize * 0.27f;
    const auto pip = [&](float x, float y)
    {
        constexpr float diameter = 5.0f;
        g.fillEllipse(x - diameter * 0.5f, y - diameter * 0.5f, diameter, diameter);
    };
    if (face == 1 || face == 3 || face == 5) pip(centreX, centreY);
    if (face >= 2) { pip(left, top); pip(right, bottom); }
    if (face >= 4) { pip(right, top); pip(left, bottom); }
    if (face == 6) { pip(left, centreY); pip(right, centreY); }

}

void DicerLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                        float position, float start, float end, juce::Slider&)
{
    const auto size = static_cast<float>(juce::jmin(width, height)) - 10.0f;
    const auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                               static_cast<float>(width), static_cast<float>(height))
                            .withSizeKeepingCentre(size, size);
    const auto radius = size * 0.5f;
    const auto angle = start + position * (end - start);
    juce::Path background, value;
    background.addCentredArc(bounds.getCentreX(), bounds.getCentreY(), radius, radius, 0.0f,
                             start, end, true);
    value.addCentredArc(bounds.getCentreX(), bounds.getCentreY(), radius, radius, 0.0f,
                        start, angle, true);
    g.setColour(juce::Colour(0xff454b57));
    g.strokePath(background, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved,
                                                   juce::PathStrokeType::rounded));
    g.setColour(juce::Colour(accent));
    g.strokePath(value, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved,
                                              juce::PathStrokeType::rounded));
    const auto centre = bounds.getCentre();
    const auto pointer = juce::Point<float>(std::sin(angle), -std::cos(angle)) * (radius - 7.0f);
    g.fillEllipse(juce::Rectangle<float>(5.0f, 5.0f).withCentre(centre + pointer));
}

void DicerLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                            const juce::Colour&, bool highlighted, bool down)
{
    auto colour = button.getButtonText() == "DICE" ? juce::Colour(accent) : juce::Colour(0xff343943);
    if (highlighted) colour = colour.brighter(0.10f);
    if (down) colour = colour.darker(0.12f);
    g.setColour(colour);
    g.fillRoundedRectangle(button.getLocalBounds().toFloat(), 7.0f);
}

SlotView::SlotView(SampleDicerAudioProcessor& p, int index) : processor(p), slot(index)
{
    thumbnailFormats.registerBasicFormats();
    thumbnail.addChangeListener(this);
    title.setText("SLOT " + juce::String(index + 1), juce::dontSendNotification);
    title.setFont(juce::FontOptions(15.0f, juce::Font::bold));
    file.setColour(juce::Label::textColourId, juce::Colour(0xffc8ccd5));
    addAndMakeVisible(title); addAndMakeVisible(file); addAndMakeVisible(previous); addAndMakeVisible(next);
    slotLock.setColour(juce::ToggleButton::tickColourId, juce::Colour(accent));
    slotLock.setColour(juce::ToggleButton::textColourId, juce::Colour(0xffaab0bb));
    slotLock.setTooltip("Exclude this slot from DICE and per-trigger randomization");
    addAndMakeVisible(slotLock);
    lockLink = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processor.state, "slot" + juce::String(index + 1) + ".lock", slotLock);

    const std::array<juce::String, 5> names { "VOLUME", "PITCH", "START", "SHIFT", "FADE" };
    const std::array<juce::String, 5> ids { "gain", "pitch", "start", "shift", "fade" };
    const std::array<juce::String, 5> suffixes { " dB", " st", " %", " ms", " %" };
    for (size_t k = 0; k < knobs.size(); ++k)
    {
        labels[k].setText(names[k], juce::dontSendNotification);
        labels[k].setJustificationType(juce::Justification::centred);
        labels[k].setColour(juce::Label::textColourId, juce::Colour(0xff9399a5));
        labels[k].setFont(juce::FontOptions(10.5f, juce::Font::bold));
        knobs[k].setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        knobs[k].setTextBoxStyle(juce::Slider::TextBoxBelow, false, 72, 18);
        knobs[k].setDisplayScale(k == 2 || k == 4 ? 100.0 : 1.0);
        knobs[k].setTextValueSuffix(suffixes[k]);
        addAndMakeVisible(labels[k]); addAndMakeVisible(knobs[k]);
        links[k] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.state, "slot" + juce::String(index + 1) + "." + ids[k], knobs[k]);
    }
    knobs[2].onValueChange = [this]
    {
        const auto start = static_cast<float>(knobs[2].getValue());
        const auto fade = processor.state.getRawParameterValue(
            "slot" + juce::String(slot + 1) + ".fade")->load();
        if (start > fade)
            if (auto* parameter = processor.state.getParameter(
                    "slot" + juce::String(slot + 1) + ".fade"))
                parameter->setValueNotifyingHost(parameter->convertTo0to1(start));
    };
    knobs[4].onValueChange = [this]
    {
        const auto fade = static_cast<float>(knobs[4].getValue());
        const auto start = processor.state.getRawParameterValue(
            "slot" + juce::String(slot + 1) + ".start")->load();
        if (fade < start)
            if (auto* parameter = processor.state.getParameter(
                    "slot" + juce::String(slot + 1) + ".start"))
                parameter->setValueNotifyingHost(parameter->convertTo0to1(fade));
    };
    previous.onClick = [this] { processor.browse(slot, -1); refresh(); };
    next.onClick = [this] { processor.browse(slot, 1); refresh(); };
    refresh();
}

SlotView::~SlotView() { thumbnail.removeChangeListener(this); }

void SlotView::paint(juce::Graphics& g)
{
    g.setColour(juce::Colour(panel));
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 9.0f);
    g.setColour(juce::Colour(0xff181b21));
    g.fillRoundedRectangle(waveformArea.toFloat(), 5.0f);
    if (thumbnail.getTotalLength() > 0.0)
    {
        const auto shiftedArea = getShiftedWaveformArea();
        if (shiftedArea.getX() > waveformArea.getX())
        {
            g.setColour(juce::Colour(accent).withAlpha(0.10f));
            g.fillRoundedRectangle(juce::Rectangle<int>(waveformArea.getX(), waveformArea.getY(),
                                                        shiftedArea.getX() - waveformArea.getX(),
                                                        waveformArea.getHeight()).toFloat(), 4.0f);
        }
        g.setColour(juce::Colour(waveform));
        thumbnail.drawChannels(g, shiftedArea.reduced(5, 3), 0.0, thumbnail.getTotalLength(), 1.0f);

        const auto fade = processor.state.getRawParameterValue(
            "slot" + juce::String(slot + 1) + ".fade")->load();
        const auto start = juce::jmin(processor.state.getRawParameterValue(
            "slot" + juce::String(slot + 1) + ".start")->load(), fade);
        const auto startX = shiftedArea.getX() + juce::roundToInt(start * shiftedArea.getWidth());
        const auto startHovered = hoverTarget == DragTarget::start || dragTarget == DragTarget::start;
        if (startHovered)
        {
            g.setColour(juce::Colours::white.withAlpha(0.16f));
            g.fillRect(startX - 4, waveformArea.getY(), 9, waveformArea.getHeight());
        }
        g.setColour(juce::Colours::white.withAlpha(startHovered ? 1.0f : 0.85f));
        g.drawVerticalLine(startX, static_cast<float>(waveformArea.getY() + 2),
                          static_cast<float>(waveformArea.getBottom() - 2));
        juce::Path marker;
        const auto startMarkerHalfWidth = startHovered ? 6.0f : 4.0f;
        const auto startMarkerHeight = startHovered ? 9.0f : 7.0f;
        marker.addTriangle(static_cast<float>(startX) - startMarkerHalfWidth,
                           static_cast<float>(waveformArea.getY() + 1),
                           static_cast<float>(startX) + startMarkerHalfWidth,
                           static_cast<float>(waveformArea.getY() + 1),
                           static_cast<float>(startX),
                           static_cast<float>(waveformArea.getY() + 1) + startMarkerHeight);
        g.fillPath(marker);

        const auto fadeX = shiftedArea.getX() + juce::roundToInt(fade * shiftedArea.getWidth());
        const auto fadeLengthMs = processor.state.getRawParameterValue(
            "slot" + juce::String(slot + 1) + ".fadeLength")->load();
        const auto fadeFraction = thumbnail.getTotalLength() > 0.0
            ? static_cast<float>((fadeLengthMs * 0.001) / thumbnail.getTotalLength()) : 0.0f;
        const auto fadeStartX = shiftedArea.getX() + juce::roundToInt(
            juce::jmax(start, fade - fadeFraction) * shiftedArea.getWidth());
        if (fadeStartX < fadeX)
        {
            juce::ColourGradient fadeGradient(juce::Colour(0xffe35f6f).withAlpha(0.02f),
                                               static_cast<float>(fadeStartX), 0.0f,
                                               juce::Colour(0xffe35f6f).withAlpha(0.48f),
                                               static_cast<float>(fadeX), 0.0f, false);
            g.setGradientFill(fadeGradient);
            g.fillRect(juce::Rectangle<int>(fadeStartX, waveformArea.getY(),
                                            fadeX - fadeStartX, waveformArea.getHeight()));
            g.setColour(juce::Colour(0xffe35f6f).withAlpha(0.75f));
            g.drawLine(static_cast<float>(fadeStartX), static_cast<float>(waveformArea.getY() + 3),
                       static_cast<float>(fadeX), static_cast<float>(waveformArea.getBottom() - 3), 1.5f);
            if (fadeX - fadeStartX > 36)
            {
                g.setFont(juce::FontOptions(9.0f, juce::Font::bold));
                g.drawText(juce::String(fadeLengthMs, 0) + " ms",
                           juce::Rectangle<int>(fadeStartX, waveformArea.getY(),
                                                fadeX - fadeStartX, 13),
                           juce::Justification::centred);
            }
        }
        if (fadeX < shiftedArea.getRight())
        {
            g.setColour(juce::Colour(0xffe35f6f).withAlpha(0.18f));
            g.fillRect(juce::Rectangle<int>(fadeX, waveformArea.getY(),
                                            shiftedArea.getRight() - fadeX, waveformArea.getHeight()));
        }
        const auto fadeHovered = hoverTarget == DragTarget::fade || dragTarget == DragTarget::fade;
        if (fadeHovered)
        {
            g.setColour(juce::Colour(0xffe35f6f).withAlpha(0.22f));
            g.fillRect(fadeX - 4, waveformArea.getY(), 9, waveformArea.getHeight());
        }
        g.setColour(fadeHovered ? juce::Colour(0xffff7b89) : juce::Colour(0xffe35f6f));
        g.drawVerticalLine(fadeX, static_cast<float>(waveformArea.getY() + 2),
                           static_cast<float>(waveformArea.getBottom() - 2));
        juce::Path fadeMarker;
        const auto fadeMarkerHalfWidth = fadeHovered ? 6.0f : 4.0f;
        const auto fadeMarkerHeight = fadeHovered ? 9.0f : 7.0f;
        fadeMarker.addTriangle(static_cast<float>(fadeX) - fadeMarkerHalfWidth,
                               static_cast<float>(waveformArea.getBottom() - 1),
                               static_cast<float>(fadeX) + fadeMarkerHalfWidth,
                               static_cast<float>(waveformArea.getBottom() - 1),
                               static_cast<float>(fadeX),
                               static_cast<float>(waveformArea.getBottom() - 1) - fadeMarkerHeight);
        g.fillPath(fadeMarker);

        const auto playhead = processor.playheadPosition(slot);
        if (playhead >= 0.0f)
        {
            const auto playheadX = shiftedArea.getX()
                + juce::roundToInt(juce::jlimit(0.0f, 1.0f, playhead) * shiftedArea.getWidth());
            g.setColour(juce::Colour(0xff45c8ff));
            g.drawVerticalLine(playheadX, static_cast<float>(waveformArea.getY() + 2),
                               static_cast<float>(waveformArea.getBottom() - 2));
        }
    }
    else
    {
        g.setColour(juce::Colour(0xff6f7580));
        g.drawText("DROP AUDIO HERE", waveformArea, juce::Justification::centred);
    }
}

juce::Rectangle<int> SlotView::getShiftedWaveformArea() const
{
    if (thumbnail.getTotalLength() <= 0.0) return waveformArea;
    const auto shiftSeconds = processor.state.getRawParameterValue(
        "slot" + juce::String(slot + 1) + ".shift")->load() * 0.001;
    const auto totalSeconds = thumbnail.getTotalLength() + shiftSeconds;
    const auto offset = totalSeconds > 0.0
        ? juce::roundToInt(waveformArea.getWidth() * shiftSeconds / totalSeconds) : 0;
    return waveformArea.withTrimmedLeft(juce::jlimit(0, waveformArea.getWidth() - 12, offset));
}

SlotView::DragTarget SlotView::markerAt(juce::Point<int> position) const
{
    if (!waveformArea.contains(position) || thumbnail.getTotalLength() <= 0.0)
        return DragTarget::none;

    const auto area = getShiftedWaveformArea();
    const auto fade = processor.state.getRawParameterValue(
        "slot" + juce::String(slot + 1) + ".fade")->load();
    const auto start = juce::jmin(processor.state.getRawParameterValue(
        "slot" + juce::String(slot + 1) + ".start")->load(), fade);
    const auto startX = area.getX() + juce::roundToInt(start * area.getWidth());
    const auto fadeX = area.getX() + juce::roundToInt(fade * area.getWidth());
    const auto startDistance = std::abs(position.x - startX);
    const auto fadeDistance = std::abs(position.x - fadeX);
    constexpr int hoverRadius = 10;

    if (startDistance > hoverRadius && fadeDistance > hoverRadius)
        return DragTarget::none;
    if (startDistance <= hoverRadius && fadeDistance <= hoverRadius)
        return position.y < waveformArea.getCentreY() ? DragTarget::start : DragTarget::fade;
    return fadeDistance <= startDistance ? DragTarget::fade : DragTarget::start;
}

void SlotView::updateMarkerHover(juce::Point<int> position)
{
    const auto nextTarget = dragTarget != DragTarget::none ? dragTarget : markerAt(position);
    if (nextTarget == hoverTarget)
        return;

    hoverTarget = nextTarget;
    setMouseCursor(hoverTarget == DragTarget::none ? juce::MouseCursor::NormalCursor
                                                   : juce::MouseCursor::PointingHandCursor);
    repaint(waveformArea.expanded(7, 2));
}

void SlotView::updateStartFromMouse(const juce::MouseEvent& event)
{
    const auto area = getShiftedWaveformArea();
    const auto rawFraction = static_cast<float>(event.position.x - area.getX())
        / static_cast<float>(juce::jmax(1, area.getWidth()));
    const auto other = processor.state.getRawParameterValue(
        "slot" + juce::String(slot + 1)
            + (dragTarget == DragTarget::fade ? ".start" : ".fade"))->load();
    const auto fraction = dragTarget == DragTarget::fade
        ? juce::jlimit(juce::jmax(0.01f, other), 1.0f, rawFraction)
        : juce::jlimit(0.0f, juce::jmin(0.99f, other), rawFraction);
    if (auto* parameter = processor.state.getParameter(
            "slot" + juce::String(slot + 1)
                + (dragTarget == DragTarget::fade ? ".fade" : ".start")))
        parameter->setValueNotifyingHost(parameter->convertTo0to1(fraction));
    repaint();
}

void SlotView::mouseDown(const juce::MouseEvent& event)
{
    if (waveformArea.contains(event.getPosition()) && thumbnail.getTotalLength() > 0.0)
    {
        dragTarget = markerAt(event.getPosition());
        if (dragTarget == DragTarget::none)
            dragTarget = DragTarget::start;
        updateMarkerHover(event.getPosition());
        if (auto* parameter = processor.state.getParameter(
                "slot" + juce::String(slot + 1)
                    + (dragTarget == DragTarget::fade ? ".fade" : ".start"))) parameter->beginChangeGesture();
        updateStartFromMouse(event);
    }
}

void SlotView::mouseDrag(const juce::MouseEvent& event)
{
    if (dragTarget != DragTarget::none)
    {
        updateStartFromMouse(event);
        updateMarkerHover(event.getPosition());
    }
}

void SlotView::mouseUp(const juce::MouseEvent& event)
{
    if (dragTarget == DragTarget::none) return;
    if (auto* parameter = processor.state.getParameter(
            "slot" + juce::String(slot + 1)
                + (dragTarget == DragTarget::fade ? ".fade" : ".start"))) parameter->endChangeGesture();
    dragTarget = DragTarget::none;
    updateMarkerHover(event.getPosition());
}

void SlotView::mouseMove(const juce::MouseEvent& event) { updateMarkerHover(event.getPosition()); }

void SlotView::mouseExit(const juce::MouseEvent&)
{
    if (dragTarget == DragTarget::none)
    {
        hoverTarget = DragTarget::none;
        setMouseCursor(juce::MouseCursor::NormalCursor);
        repaint(waveformArea.expanded(7, 2));
    }
}

void SlotView::mouseWheelMove(const juce::MouseEvent& event,
                              const juce::MouseWheelDetails& wheel)
{
    if (!waveformArea.contains(event.getPosition()) || thumbnail.getTotalLength() <= 0.0)
        return;
    const auto area = getShiftedWaveformArea();
    const auto fade = processor.state.getRawParameterValue(
        "slot" + juce::String(slot + 1) + ".fade")->load();
    const auto length = processor.state.getRawParameterValue(
        "slot" + juce::String(slot + 1) + ".fadeLength")->load();
    const auto fadeFraction = static_cast<float>((length * 0.001) / thumbnail.getTotalLength());
    const auto start = processor.state.getRawParameterValue(
        "slot" + juce::String(slot + 1) + ".start")->load();
    const auto fadeX = area.getX() + juce::roundToInt(fade * area.getWidth());
    const auto fadeStartX = area.getX() + juce::roundToInt(
        juce::jmax(start, fade - fadeFraction) * area.getWidth());
    if (event.getPosition().x < fadeStartX - 8 || event.getPosition().x > fadeX + 8)
        return;
    if (auto* parameter = processor.state.getParameter(
            "slot" + juce::String(slot + 1) + ".fadeLength"))
    {
        const auto delta = wheel.deltaY != 0.0f ? wheel.deltaY : wheel.deltaX;
        const auto nextLength = juce::jlimit(1.0f, 250.0f, length + delta * 80.0f);
        parameter->beginChangeGesture();
        parameter->setValueNotifyingHost(parameter->convertTo0to1(nextLength));
        parameter->endChangeGesture();
        repaint();
    }
}

void SlotView::resized()
{
    auto area = getLocalBounds().reduced(10);
    auto header = area.removeFromTop(25);
    title.setBounds(header.removeFromLeft(66));
    previous.setBounds(header.removeFromLeft(27));
    next.setBounds(header.removeFromRight(27));
    slotLock.setBounds(header.removeFromRight(65));
    file.setBounds(header.reduced(6, 0));
    waveformArea = area.removeFromTop(36);
    area.removeFromTop(3);
    const auto width = area.getWidth() / 5;
    for (size_t k = 0; k < knobs.size(); ++k)
    {
        auto column = area.removeFromLeft(width);
        labels[k].setBounds(column.removeFromTop(15));
        knobs[k].setBounds(column);
    }
}

void SlotView::filesDropped(const juce::StringArray& files, int, int)
{
    if (!files.isEmpty() && processor.loadSample(slot, juce::File(files[0]))) refresh();
}

void SlotView::refresh()
{
    const auto current = processor.sampleFile(slot);
    file.setText(processor.sampleName(slot), juce::dontSendNotification);
    if (current != displayedFile)
    {
        displayedFile = current;
        thumbnail.clear();
        if (current.existsAsFile())
            thumbnail.setSource(new juce::FileInputSource(current));
        repaint();
    }
}

SampleDicerAudioProcessorEditor::SampleDicerAudioProcessorEditor(SampleDicerAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setLookAndFeel(&lookAndFeel);
    content.setBounds(0, 0, designWidth, designHeight);
    content.setInterceptsMouseClicks(false, true);
    addAndMakeVisible(content);
    for (size_t i = 0; i < slots.size(); ++i)
    {
        slots[i] = std::make_unique<SlotView>(processor, static_cast<int>(i));
        content.addAndMakeVisible(*slots[i]);
    }
    const std::array<juce::String, 4> names { "VOLUME", "PITCH", "START", "SHIFT" };
    const std::array<juce::String, 4> ids { "volume", "pitch", "start", "shift" };
    for (size_t i = 0; i < random.size(); ++i)
    {
        randomLabels[i].setText(names[i], juce::dontSendNotification);
        randomLabels[i].setColour(juce::Label::textColourId, juce::Colour(0xffaab0bb));
        random[i].setSliderStyle(juce::Slider::LinearHorizontal);
        random[i].setTextBoxStyle(juce::Slider::TextBoxRight, false, 62, 18);
        random[i].setDisplayScale(100.0);
        random[i].setTextValueSuffix(" %");
        content.addAndMakeVisible(randomLabels[i]); content.addAndMakeVisible(random[i]);
        randomLinks[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.state, "random." + ids[i], random[i]);
    }
    randomTitle.setText("RANDOM AMOUNT", juce::dontSendNotification);
    randomTitle.setFont(juce::FontOptions(12.0f, juce::Font::bold));
    randomTitle.setColour(juce::Label::textColourId, juce::Colour(0xff858c98));
    randomTitle.setJustificationType(juce::Justification::centredLeft);
    content.addAndMakeVisible(randomTitle);
    content.addAndMakeVisible(dice);
    content.addAndMakeVisible(back);
    content.addAndMakeVisible(samples);
    content.addAndMakeVisible(params);
    about.setColour(juce::HyperlinkButton::textColourId, juce::Colour(0xff9299a5));
    about.onClick = [this]
    {
        juce::CallOutBox::launchAsynchronously(std::make_unique<AboutPanel>(),
                                               about.getScreenBounds(), nullptr);
    };
    content.addAndMakeVisible(about);
    voicesLabel.setText("VOICES", juce::dontSendNotification);
    voicesLabel.setFont(juce::FontOptions(10.5f, juce::Font::bold));
    voicesLabel.setColour(juce::Label::textColourId, juce::Colour(0xff9299a5));
    voicesLabel.setJustificationType(juce::Justification::centredRight);
    content.addAndMakeVisible(voicesLabel);
    for (int i = 1; i <= 16; ++i) voices.addItem(juce::String(i), i);
    content.addAndMakeVisible(voices);
    voicesLink = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        processor.state, "global.voices", voices);
    burst.setColour(juce::ToggleButton::textColourId, juce::Colour(0xffdfe3ea));
    burst.setColour(juce::ToggleButton::tickColourId, juce::Colour(accent));
    burst.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colour(0xff4a505c));
    content.addAndMakeVisible(burst);
    burstLink = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processor.state, "global.burst", burst);
    pte.setColour(juce::ToggleButton::textColourId, juce::Colour(0xffdfe3ea));
    pte.setColour(juce::ToggleButton::tickColourId, juce::Colour(accent));
    pte.setTooltip("Randomize parameters on every MIDI note-on; samples stay unchanged");
    content.addAndMakeVisible(pte);
    pteLink = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processor.state, "global.pte", pte);
    key.setColour(juce::ToggleButton::textColourId, juce::Colour(0xffdfe3ea));
    key.setColour(juce::ToggleButton::tickColourId, juce::Colour(accent));
    key.setTooltip("Chromatic keytracking relative to MIDI note 60 (middle C)");
    content.addAndMakeVisible(key);
    keyLink = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processor.state, "global.key", key);
    burstRateLabel.setText("RATE", juce::dontSendNotification);
    burstRateLabel.setFont(juce::FontOptions(9.5f, juce::Font::bold));
    burstRateLabel.setColour(juce::Label::textColourId, juce::Colour(0xff9299a5));
    burstRateLabel.setJustificationType(juce::Justification::centred);
    content.addAndMakeVisible(burstRateLabel);
    burstRate.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    burstRate.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 58, 16);
    burstRate.setTextValueSuffix(" Hz");
    content.addAndMakeVisible(burstRate);
    burstRateLink = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.state, "global.burstRate", burstRate);
    dice.onClick = [this] { dice.roll(); processor.dice(); };
    back.onClick = [this] { processor.back(); };
    samples.onClick = [this] { processor.dice(true, false); };
    params.onClick = [this] { processor.dice(false, true); };
    setResizable(true, true);
    setResizeLimits(juce::roundToInt(designWidth * minimumScale),
                    juce::roundToInt(designHeight * minimumScale),
                    juce::roundToInt(designWidth * maximumScale),
                    juce::roundToInt(designHeight * maximumScale));
    if (auto* constrainer = getConstrainer())
        constrainer->setFixedAspectRatio(static_cast<double>(designWidth) / designHeight);
    setSize(designWidth, designHeight);
    beginUpdateCheck();
    startTimerHz(60);
}

SampleDicerAudioProcessorEditor::~SampleDicerAudioProcessorEditor()
{
    updateDownload.reset();
    updateResponseFile.deleteFile();
    setLookAndFeel(nullptr);
}

void SampleDicerAudioProcessorEditor::beginUpdateCheck()
{
    const auto settingsOptions = updateSettingsOptions();
    juce::PropertiesFile settings(settingsOptions);
    const auto now = juce::Time::currentTimeMillis();
    const auto lastCheck = settings.getValue("lastUpdateCheck").getLargeIntValue();
    if (lastCheck > 0 && now - lastCheck < updateCheckIntervalMs)
        return;
    settings.setValue("lastUpdateCheck", juce::String(now));
    settings.saveIfNeeded();

    updateResponseFile = juce::File::getSpecialLocation(juce::File::tempDirectory)
        .getNonexistentChildFile("sample-dicer-latest-release", ".json", false);
    const auto headers = "Accept: application/vnd.github+json\r\n"
                         "User-Agent: Sample-Dicer/" + juce::String(JucePlugin_VersionString) + "\r\n";
    updateDownload = juce::URL(latestReleaseApi).downloadToFile(
        updateResponseFile,
        juce::URL::DownloadTaskOptions().withExtraHeaders(headers).withListener(this));
}

void SampleDicerAudioProcessorEditor::finished(juce::URL::DownloadTask* task, bool success)
{
    if (!success || task == nullptr || task->statusCode() != 200)
    {
        updateResponseFile.deleteFile();
        return;
    }

    const auto response = juce::JSON::parse(updateResponseFile.loadFileAsString());
    updateResponseFile.deleteFile();
    const auto* object = response.getDynamicObject();
    if (object == nullptr)
        return;

    const auto version = object->getProperty("tag_name").toString();
    if (!isNewerVersion(version, JucePlugin_VersionString))
        return;

    auto releaseUrl = juce::URL(object->getProperty("html_url").toString());
    if (releaseUrl.isEmpty())
        releaseUrl = juce::URL(releasesUrl);
    juce::MessageManager::callAsync(
        [safeEditor = juce::Component::SafePointer<SampleDicerAudioProcessorEditor>(this),
         version, releaseUrl]
        {
            if (safeEditor != nullptr)
                safeEditor->showUpdateAvailable(version, releaseUrl);
        });
}

void SampleDicerAudioProcessorEditor::showUpdateAvailable(const juce::String& version,
                                                           const juce::URL& releaseUrl)
{
    juce::PropertiesFile settings(updateSettingsOptions());
    if (settings.getValue("lastNotifiedRelease") == version)
        return;
    settings.setValue("lastNotifiedRelease", version);
    settings.saveIfNeeded();

    const auto message = "Sample Dicer " + version +
        " is available. You are using version " + juce::String(JucePlugin_VersionString) + ".";
    const auto box = juce::MessageBoxOptions()
        .withIconType(juce::MessageBoxIconType::InfoIcon)
        .withTitle("Sample Dicer update available")
        .withMessage(message)
        .withButton("Open Releases")
        .withButton("Later")
        .withParentComponent(this);
    juce::AlertWindow::showAsync(box, [releaseUrl] (int result)
    {
        if (result == 1)
            releaseUrl.launchInDefaultBrowser();
    });
}

void SampleDicerAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff14171c));
    juce::Graphics::ScopedSaveState state(g);
    g.addTransform(content.getTransform());
    g.setColour(juce::Colour(accent));
    g.setFont(juce::FontOptions(25.0f, juce::Font::bold));
    g.drawText("SAMPLE DICER", 20, 10, 300, 36, juce::Justification::centredLeft);
}

void SampleDicerAudioProcessorEditor::resized()
{
    const auto scale = juce::jmin(static_cast<float>(getWidth()) / designWidth,
                                  static_cast<float>(getHeight()) / designHeight);
    const auto scaledWidth = designWidth * scale;
    const auto scaledHeight = designHeight * scale;
    const auto offsetX = (static_cast<float>(getWidth()) - scaledWidth) * 0.5f;
    const auto offsetY = (static_cast<float>(getHeight()) - scaledHeight) * 0.5f;

    content.setTransform(juce::AffineTransform::scale(scale).translated(offsetX, offsetY));

    about.setBounds(designWidth - 76, 12, 55, 24);
    auto area = content.getLocalBounds().reduced(20);
    area.removeFromTop(42);
    for (auto& view : slots)
    {
        if (view != nullptr) view->setBounds(area.removeFromTop(145));
        area.removeFromTop(6);
    }
    area.removeFromTop(5);
    auto titleRow = area.removeFromTop(23);
    randomTitle.setBounds(titleRow.removeFromLeft(180));
    area.removeFromTop(3);
    auto randomArea = area.removeFromLeft(510);
    for (size_t i = 0; i < random.size(); ++i)
    {
        auto row = randomArea.removeFromTop(29);
        randomLabels[i].setBounds(row.removeFromLeft(78));
        random[i].setBounds(row);
    }
    randomArea.removeFromTop(3);
    auto performanceRow = randomArea.removeFromTop(39);
    voicesLabel.setBounds(performanceRow.removeFromLeft(58));
    voices.setBounds(performanceRow.removeFromLeft(55).reduced(2, 7));
    performanceRow.removeFromLeft(12);
    burst.setBounds(performanceRow.removeFromLeft(80).reduced(0, 5));
    performanceRow.removeFromLeft(8);
    auto rateArea = performanceRow.removeFromLeft(94);
    burstRateLabel.setBounds(rateArea.removeFromLeft(34));
    burstRate.setBounds(rateArea);
    performanceRow.removeFromLeft(8);
    key.setBounds(performanceRow.removeFromLeft(68).reduced(0, 5));
    performanceRow.removeFromLeft(3);
    pte.setBounds(performanceRow.removeFromLeft(70).reduced(0, 5));
    auto buttons = area.reduced(12, 0);
    auto generationButtons = buttons.removeFromTop(58);
    back.setBounds(generationButtons.removeFromLeft(82).reduced(0, 2));
    generationButtons.removeFromLeft(6);
    dice.setBounds(generationButtons.reduced(0, 2));
    buttons.removeFromTop(6);
    samples.setBounds(buttons.removeFromLeft(buttons.getWidth() / 2).reduced(2));
    params.setBounds(buttons.reduced(2));
}

void SampleDicerAudioProcessorEditor::timerCallback()
{
    for (auto& view : slots) if (view != nullptr) { view->refresh(); view->repaint(); }
}
