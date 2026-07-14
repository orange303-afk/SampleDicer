#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

struct DicerTheme
{
    DicerTheme() = default;
    DicerTheme(juce::String themeName, juce::uint32 backgroundColour, juce::uint32 panelColour,
               juce::uint32 waveformBackgroundColour, juce::uint32 accentColour,
               juce::uint32 waveformColour, juce::uint32 primaryTextColour,
               juce::uint32 secondaryTextColour, juce::uint32 mutedTextColour,
               juce::uint32 controlTrackColour, juce::uint32 buttonColour,
               juce::uint32 fadeColour, juce::uint32 playheadColour)
        : name(std::move(themeName)), background(backgroundColour), panel(panelColour),
          waveformBackground(waveformBackgroundColour), accent(accentColour), waveform(waveformColour),
          primaryText(primaryTextColour), secondaryText(secondaryTextColour), mutedText(mutedTextColour),
          controlTrack(controlTrackColour), button(buttonColour), fade(fadeColour), playhead(playheadColour) {}
    juce::String name;
    juce::Colour background, panel, waveformBackground, accent, waveform;
    juce::Colour primaryText, secondaryText, mutedText, controlTrack, button;
    juce::Colour fade, playhead;
};

class DicerLookAndFeel : public juce::LookAndFeel_V4
{
public:
    DicerLookAndFeel();
    void setTheme(const DicerTheme&);
    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPos, float startAngle, float endAngle,
                          juce::Slider&) override;
    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&,
                              bool highlighted, bool down) override;
private:
    DicerTheme theme;
};

class DisplaySlider : public juce::Slider
{
public:
    void setDisplayScale(double newScale) { displayScale = newScale; }
    void setShiftFineSteps(double coarse, double fine) { coarseStep = coarse; fineStep = fine; }
    void setAdaptiveDecimalDisplay(bool enabled) { adaptiveDecimalDisplay = enabled; }
    juce::String getTextFromValue(double value) override;
    double getValueFromText(const juce::String& text) override;
    double snapValue(double attemptedValue, DragMode dragMode) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override;
    void mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails&) override;
private:
    double displayScale = 1.0;
    double coarseStep = 0.0, fineStep = 0.0;
    bool fineStepActive = false;
    bool adaptiveDecimalDisplay = false;
};

class DiceButton : public juce::Button
{
public:
    DiceButton();
    void roll();
    void setTheme(const DicerTheme& newTheme) { theme = newTheme; repaint(); }
    void paintButton(juce::Graphics&, bool highlighted, bool down) override;
private:
    int face = 1;
    juce::Random random;
    DicerTheme theme;
};

class MiniDiceButton : public juce::Button
{
public:
    MiniDiceButton() : juce::Button("Slot DICE") {}
    void roll();
    void setTheme(const DicerTheme& newTheme) { theme = newTheme; repaint(); }
    void paintButton(juce::Graphics&, bool highlighted, bool down) override;
private:
    int face = 3;
    juce::Random random;
    DicerTheme theme;
};

class ClearSlotButton : public juce::Button
{
public:
    ClearSlotButton() : juce::Button("Clear Slot") {}
    void setTheme(const DicerTheme& newTheme) { theme = newTheme; repaint(); }
    void paintButton(juce::Graphics&, bool highlighted, bool down) override;
private:
    DicerTheme theme;
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
    void setTheme(const DicerTheme&);
    void setGlitchVisuals(bool enabled, uint32_t seed);
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override;
    void mouseMove(const juce::MouseEvent&) override;
    void mouseExit(const juce::MouseEvent&) override;
    void mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails&) override;

private:
    enum class DragTarget { none, start, fade, range };
    void changeListenerCallback(juce::ChangeBroadcaster*) override { repaint(); }
    juce::Rectangle<int> getShiftedWaveformArea() const;
    DragTarget markerAt(juce::Point<int>) const;
    void updateMarkerHover(juce::Point<int>);
    void updateStartFromMouse(const juce::MouseEvent&);
    void updateRangeFromMouse(const juce::MouseEvent&);
    SampleDicerAudioProcessor& processor;
    int slot;
    juce::Label title, file;
    juce::TextButton previous { "<" }, next { ">" };
    MiniDiceButton slotDice;
    ClearSlotButton clearSlot;
    juce::ToggleButton slotLock { "LOCK" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> lockLink;
    std::array<DisplaySlider, 5> knobs;
    std::array<juce::Label, 5> labels;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 5> links;
    juce::AudioFormatManager thumbnailFormats;
    juce::AudioThumbnailCache thumbnailCache { 8 };
    juce::AudioThumbnail thumbnail { 512, thumbnailFormats, thumbnailCache };
    juce::File displayedFile;
    juce::String displayedSampleName;
    uint32_t displayedGlitchSeed = 0;
    bool displayedWithGlitch = false;
    juce::Rectangle<int> waveformArea;
    DragTarget dragTarget = DragTarget::none;
    DragTarget hoverTarget = DragTarget::none;
    float rangeDragMouseFraction = 0.0f;
    float rangeDragStart = 0.0f;
    float rangeDragFade = 1.0f;
    bool glitchVisuals = false;
    uint32_t glitchSeed = 0;
    DicerTheme theme;
};

class SampleDicerAudioProcessorEditor : public juce::AudioProcessorEditor,
                                        private juce::Timer,
                                        private juce::URL::DownloadTaskListener
{
public:
    explicit SampleDicerAudioProcessorEditor(SampleDicerAudioProcessor&);
    ~SampleDicerAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void finished(juce::URL::DownloadTask*, bool success) override;
    void beginUpdateCheck();
    void showUpdateAvailable(const juce::String& version, const juce::URL& releaseUrl);
    void showOptions();
    void quickExport();
    void chooseExportDirectory(std::function<void(bool)> completion);
    void exportToConfiguredDirectory();
    void applyScalePreset(int index, bool save = true);
    void applyTheme(int index, bool save = true);
    void setGlitchVisualsEnabled(bool enabled);
    void triggerGlitchVisuals(uint32_t generation);
    void applyGlitchThemeFrame();
    void setInterfaceTextGlitch(bool enabled, uint32_t seed);
    void updateComponentColours();
    SampleDicerAudioProcessor& processor;
    DicerLookAndFeel lookAndFeel;
    juce::Component content;
    std::array<std::unique_ptr<SlotView>, 4> slots;
    DiceButton dice;
    juce::TextButton back { "<< BACK" }, samples { "SAMPLES" }, params { "PARAMS" };
    juce::HyperlinkButton options { "Options", juce::URL() };
    juce::HyperlinkButton about { "About", juce::URL() };
    juce::TextButton quickExportButton { "QUICK EXPORT" };
    juce::Label masterLabel;
    DisplaySlider masterVolume;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> masterVolumeLink;
    juce::Label randomTitle;
    juce::Label voicesLabel, burstRateLabel;
    juce::ComboBox voices;
    juce::ToggleButton burst { "BURST" };
    juce::ToggleButton pte { "PTE" };
    juce::ToggleButton key { "KEY" };
    juce::ToggleButton roundRobin { "RR" };
    DisplaySlider burstRate;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> voicesLink;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> burstLink;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> pteLink;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> keyLink;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> roundRobinLink;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> burstRateLink;
    std::array<DisplaySlider, 4> random;
    std::array<juce::Label, 4> randomLabels;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 4> randomLinks;
    juce::File updateResponseFile;
    std::unique_ptr<juce::URL::DownloadTask> updateDownload;
    std::unique_ptr<juce::FileChooser> exportDirectoryChooser;
    DicerTheme normalTheme;
    DicerTheme currentTheme;
    int scalePresetIndex = 1;
    int themeIndex = 0;
    juce::Random visualRandom;
    juce::String glitchTitleText { "SAMPLE DICER" };
    uint32_t lastGlitchGeneration = 0;
    uint32_t glitchVisualSeed = 0;
    int glitchFlashFrames = 0;
    int exportStatusFrames = 0;
    bool glitchVisualActive = false;
    bool glitchModeWasEnabled = false;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleDicerAudioProcessorEditor)
};
