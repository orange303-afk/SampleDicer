#include "PluginEditor.h"

namespace
{
constexpr auto accent = 0xffffb000;
constexpr auto panel = 0xff242831;
constexpr auto waveform = 0xfff1b935;

class AboutPanel : public juce::Component
{
public:
    AboutPanel()
        : website("ilyaorange.tilda.ws", juce::URL("https://ilyaorange.tilda.ws")),
          bandcamp("ilyaorange.bandcamp.com", juce::URL("https://ilyaorange.bandcamp.com")),
          gumroad("ilyaorange.gumroad.com", juce::URL("https://ilyaorange.gumroad.com"))
    {
        title.setText("Sample Dicer " + juce::String(JucePlugin_VersionString) + " by Ilya Orange",
                      juce::dontSendNotification);
        title.setJustificationType(juce::Justification::centred);
        title.setFont(juce::FontOptions(15.0f, juce::Font::bold));
        title.setColour(juce::Label::textColourId, juce::Colour(0xfff0f1f4));
        addAndMakeVisible(title);
        for (auto* link : { &website, &bandcamp, &gumroad })
        {
            link->setColour(juce::HyperlinkButton::textColourId, juce::Colour(accent));
            addAndMakeVisible(link);
        }
        setSize(330, 155);
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
        for (auto* link : { &website, &bandcamp, &gumroad })
            link->setBounds(area.removeFromTop(29));
    }

private:
    juce::Label title;
    juce::HyperlinkButton website, bandcamp, gumroad;
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

        const auto start = processor.state.getRawParameterValue(
            "slot" + juce::String(slot + 1) + ".start")->load();
        const auto startX = shiftedArea.getX() + juce::roundToInt(start * shiftedArea.getWidth());
        g.setColour(juce::Colours::white.withAlpha(0.85f));
        g.drawVerticalLine(startX, static_cast<float>(waveformArea.getY() + 2),
                          static_cast<float>(waveformArea.getBottom() - 2));
        juce::Path marker;
        marker.addTriangle(static_cast<float>(startX - 4), static_cast<float>(waveformArea.getY() + 1),
                           static_cast<float>(startX + 4), static_cast<float>(waveformArea.getY() + 1),
                           static_cast<float>(startX), static_cast<float>(waveformArea.getY() + 7));
        g.fillPath(marker);

        const auto fade = processor.state.getRawParameterValue(
            "slot" + juce::String(slot + 1) + ".fade")->load();
        const auto fadeX = shiftedArea.getX() + juce::roundToInt(fade * shiftedArea.getWidth());
        if (fadeX < shiftedArea.getRight())
        {
            g.setColour(juce::Colour(0xffe35f6f).withAlpha(0.18f));
            g.fillRect(juce::Rectangle<int>(fadeX, waveformArea.getY(),
                                            shiftedArea.getRight() - fadeX, waveformArea.getHeight()));
        }
        g.setColour(juce::Colour(0xffe35f6f));
        g.drawVerticalLine(fadeX, static_cast<float>(waveformArea.getY() + 2),
                           static_cast<float>(waveformArea.getBottom() - 2));
        juce::Path fadeMarker;
        fadeMarker.addTriangle(static_cast<float>(fadeX - 4), static_cast<float>(waveformArea.getBottom() - 1),
                               static_cast<float>(fadeX + 4), static_cast<float>(waveformArea.getBottom() - 1),
                               static_cast<float>(fadeX), static_cast<float>(waveformArea.getBottom() - 7));
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

void SlotView::updateStartFromMouse(const juce::MouseEvent& event)
{
    const auto area = getShiftedWaveformArea();
    const auto fraction = juce::jlimit(dragTarget == DragTarget::fade ? 0.01f : 0.0f,
        dragTarget == DragTarget::fade ? 1.0f : 0.99f,
        static_cast<float>(event.position.x - area.getX()) / static_cast<float>(juce::jmax(1, area.getWidth())));
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
        const auto area = getShiftedWaveformArea();
        const auto fade = processor.state.getRawParameterValue(
            "slot" + juce::String(slot + 1) + ".fade")->load();
        const auto fadeX = area.getX() + juce::roundToInt(fade * area.getWidth());
        dragTarget = std::abs(event.getPosition().x - fadeX) <= 10
            ? DragTarget::fade : DragTarget::start;
        if (auto* parameter = processor.state.getParameter(
                "slot" + juce::String(slot + 1)
                    + (dragTarget == DragTarget::fade ? ".fade" : ".start"))) parameter->beginChangeGesture();
        updateStartFromMouse(event);
    }
}

void SlotView::mouseDrag(const juce::MouseEvent& event)
{
    if (dragTarget != DragTarget::none) updateStartFromMouse(event);
}

void SlotView::mouseUp(const juce::MouseEvent&)
{
    if (dragTarget == DragTarget::none) return;
    if (auto* parameter = processor.state.getParameter(
            "slot" + juce::String(slot + 1)
                + (dragTarget == DragTarget::fade ? ".fade" : ".start"))) parameter->endChangeGesture();
    dragTarget = DragTarget::none;
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
    for (size_t i = 0; i < slots.size(); ++i)
    {
        slots[i] = std::make_unique<SlotView>(processor, static_cast<int>(i));
        addAndMakeVisible(*slots[i]);
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
        addAndMakeVisible(randomLabels[i]); addAndMakeVisible(random[i]);
        randomLinks[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.state, "random." + ids[i], random[i]);
    }
    randomTitle.setText("RANDOM AMOUNT", juce::dontSendNotification);
    randomTitle.setFont(juce::FontOptions(12.0f, juce::Font::bold));
    randomTitle.setColour(juce::Label::textColourId, juce::Colour(0xff858c98));
    randomTitle.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(randomTitle);
    addAndMakeVisible(dice);
    addAndMakeVisible(back);
    addAndMakeVisible(samples);
    addAndMakeVisible(params);
    about.setColour(juce::HyperlinkButton::textColourId, juce::Colour(0xff9299a5));
    about.onClick = [this]
    {
        juce::CallOutBox::launchAsynchronously(std::make_unique<AboutPanel>(),
                                               about.getScreenBounds(), nullptr);
    };
    addAndMakeVisible(about);
    voicesLabel.setText("VOICES", juce::dontSendNotification);
    voicesLabel.setFont(juce::FontOptions(10.5f, juce::Font::bold));
    voicesLabel.setColour(juce::Label::textColourId, juce::Colour(0xff9299a5));
    voicesLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(voicesLabel);
    for (int i = 1; i <= 16; ++i) voices.addItem(juce::String(i), i);
    addAndMakeVisible(voices);
    voicesLink = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        processor.state, "global.voices", voices);
    burst.setColour(juce::ToggleButton::textColourId, juce::Colour(0xffdfe3ea));
    burst.setColour(juce::ToggleButton::tickColourId, juce::Colour(accent));
    burst.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colour(0xff4a505c));
    addAndMakeVisible(burst);
    burstLink = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processor.state, "global.burst", burst);
    pte.setColour(juce::ToggleButton::textColourId, juce::Colour(0xffdfe3ea));
    pte.setColour(juce::ToggleButton::tickColourId, juce::Colour(accent));
    pte.setTooltip("Randomize parameters on every MIDI note-on; samples stay unchanged");
    addAndMakeVisible(pte);
    pteLink = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processor.state, "global.pte", pte);
    burstRateLabel.setText("RATE", juce::dontSendNotification);
    burstRateLabel.setFont(juce::FontOptions(9.5f, juce::Font::bold));
    burstRateLabel.setColour(juce::Label::textColourId, juce::Colour(0xff9299a5));
    burstRateLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(burstRateLabel);
    burstRate.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    burstRate.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 58, 16);
    burstRate.setTextValueSuffix(" Hz");
    addAndMakeVisible(burstRate);
    burstRateLink = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.state, "global.burstRate", burstRate);
    dice.onClick = [this] { dice.roll(); processor.dice(); };
    back.onClick = [this] { processor.back(); };
    samples.onClick = [this] { processor.dice(true, false); };
    params.onClick = [this] { processor.dice(false, true); };
    setSize(820, 875);
    startTimerHz(60);
}

SampleDicerAudioProcessorEditor::~SampleDicerAudioProcessorEditor() { setLookAndFeel(nullptr); }

void SampleDicerAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff14171c));
    g.setColour(juce::Colour(accent));
    g.setFont(juce::FontOptions(25.0f, juce::Font::bold));
    g.drawText("SAMPLE DICER", 20, 10, 300, 36, juce::Justification::centredLeft);
}

void SampleDicerAudioProcessorEditor::resized()
{
    about.setBounds(getWidth() - 76, 12, 55, 24);
    auto area = getLocalBounds().reduced(20);
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
