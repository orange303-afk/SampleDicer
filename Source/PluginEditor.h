#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/SampleBrowserModalComponent.h"

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

class SlotBrowseButton : public juce::Button
{
public:
    SlotBrowseButton() : juce::Button("Load Sample") {}
    void setTheme(const DicerTheme& newTheme) { theme = newTheme; repaint(); }
    void paintButton(juce::Graphics& g, bool highlighted, bool down) override
    {
        auto background = theme.button;
        if (highlighted) background = background.brighter(0.12f);
        if (down) background = background.darker(0.12f);
        g.setColour(background);
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 4.0f);
        g.setColour(highlighted ? theme.accent : juce::Colour(0xff3a4254));
        g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), 4.0f, 1.0f);

        auto iconBounds = getLocalBounds().toFloat().withSizeKeepingCentre(13.0f, 10.0f);
        const float x = iconBounds.getX();
        const float y = iconBounds.getY();
        const float w = iconBounds.getWidth();
        const float h = iconBounds.getHeight();

        juce::Path folder;
        folder.startNewSubPath(x, y + 2.2f);
        folder.lineTo(x, y + h);
        folder.lineTo(x + w, y + h);
        folder.lineTo(x + w, y + 2.0f);
        folder.lineTo(x + w * 0.55f, y + 2.0f);
        folder.lineTo(x + w * 0.40f, y);
        folder.lineTo(x + 1.0f, y);
        folder.lineTo(x, y + 1.0f);
        folder.closeSubPath();

        g.setColour(highlighted ? theme.accent.brighter(0.2f) : theme.accent);
        g.strokePath(folder, juce::PathStrokeType(1.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
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
    SlotBrowseButton slotBrowse;
    ClearSlotButton clearSlot;
    juce::ToggleButton slotLock { "LOCK" };
    juce::ToggleButton slotMute { "MUTE" };
    juce::ToggleButton slotSolo { "SOLO" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> lockLink;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> muteLink;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> soloLink;
    std::array<DisplaySlider, 5> knobs;
    std::array<juce::Label, 5> labels;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 5> links;
    juce::AudioFormatManager thumbnailFormats;
    juce::AudioThumbnailCache thumbnailCache { 8 };
    juce::AudioThumbnail thumbnail { 512, thumbnailFormats, thumbnailCache };
    juce::File displayedFile;
    juce::String displayedSampleName;
    juce::String displayedPadTitle;
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
    void openSampleBrowserForSlot(int slot);
    bool keyPressed(const juce::KeyPress& keyPress) override;

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
    juce::Label masterAdsrTitle;
    std::array<DisplaySlider, 4> masterAdsr;
    std::array<juce::Label, 4> masterAdsrLabels;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 4> masterAdsrLinks;
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
    std::array<DisplaySlider, 5> random;
    std::array<juce::Label, 5> randomLabels;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 5> randomLinks;
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
    sampledicer::SampleBrowserModalComponent sampleBrowserModal;
    int currentBrowserSlot = 0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleDicerAudioProcessorEditor)
};
