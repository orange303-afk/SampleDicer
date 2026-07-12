#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class DicerLookAndFeel : public juce::LookAndFeel_V4
{
public:
    DicerLookAndFeel();
    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPos, float startAngle, float endAngle,
                          juce::Slider&) override;
    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&,
                              bool highlighted, bool down) override;
};

class DisplaySlider : public juce::Slider
{
public:
    void setDisplayScale(double newScale) { displayScale = newScale; }
    juce::String getTextFromValue(double value) override;
    double getValueFromText(const juce::String& text) override;
private:
    double displayScale = 1.0;
};

class DiceButton : public juce::Button
{
public:
    DiceButton();
    void roll();
    void paintButton(juce::Graphics&, bool highlighted, bool down) override;
private:
    int face = 1;
    juce::Random random;
};

class SlotView : public juce::Component,
                 public juce::FileDragAndDropTarget,
                 private juce::ChangeListener
{
public:
    SlotView(SampleDicerAudioProcessor&, int);
    ~SlotView() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    bool isInterestedInFileDrag(const juce::StringArray&) override { return true; }
    void filesDropped(const juce::StringArray&, int, int) override;
    void refresh();
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override;

private:
    void changeListenerCallback(juce::ChangeBroadcaster*) override { repaint(); }
    juce::Rectangle<int> getShiftedWaveformArea() const;
    void updateStartFromMouse(const juce::MouseEvent&);
    SampleDicerAudioProcessor& processor;
    int slot;
    juce::Label title, file;
    juce::TextButton previous { "<" }, next { ">" };
    juce::ToggleButton slotLock { "LOCK" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> lockLink;
    std::array<DisplaySlider, 5> knobs;
    std::array<juce::Label, 5> labels;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 5> links;
    juce::AudioFormatManager thumbnailFormats;
    juce::AudioThumbnailCache thumbnailCache { 8 };
    juce::AudioThumbnail thumbnail { 512, thumbnailFormats, thumbnailCache };
    juce::File displayedFile;
    juce::Rectangle<int> waveformArea;
    enum class DragTarget { none, start, fade };
    DragTarget dragTarget = DragTarget::none;
};

class SampleDicerAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit SampleDicerAudioProcessorEditor(SampleDicerAudioProcessor&);
    ~SampleDicerAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    SampleDicerAudioProcessor& processor;
    DicerLookAndFeel lookAndFeel;
    std::array<std::unique_ptr<SlotView>, 4> slots;
    DiceButton dice;
    juce::TextButton back { "<< BACK" }, samples { "SAMPLES" }, params { "PARAMS" };
    juce::HyperlinkButton about { "about", juce::URL() };
    juce::Label randomTitle;
    juce::Label voicesLabel, burstRateLabel;
    juce::ComboBox voices;
    juce::ToggleButton burst { "BURST" };
    juce::ToggleButton pte { "PTE" };
    DisplaySlider burstRate;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> voicesLink;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> burstLink;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> pteLink;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> burstRateLink;
    std::array<DisplaySlider, 4> random;
    std::array<juce::Label, 4> randomLabels;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 4> randomLinks;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleDicerAudioProcessorEditor)
};
