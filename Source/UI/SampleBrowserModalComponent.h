#pragma once

#include <JuceHeader.h>
#include "../Database/SampleDatabase.h"
#include "../Database/SampleScanner.h"

namespace sampledicer
{

// Self-contained LCD LookAndFeel for buttons & toggles
class BrowserLcdLookAndFeel : public juce::LookAndFeel_V4
{
public:
    juce::Colour accentColour { 0xff00e5ff };
    juce::Colour buttonColour { 0xff162738 };
    juce::Colour buttonHighlightColour { 0xff1f364d };
    juce::Colour textColour { 0xff88d8ff };
    juce::Colour textHighlightColour { 0xffb8f0ff };
    juce::Colour textActiveColour { 0xff0a1626 };

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour& /*bg*/,
                              bool isHighlighted, bool isDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);
        const float corner = 3.0f;
        const bool active = button.getToggleState() || isDown;

        if (active)
        {
            g.setColour(accentColour);
            g.fillRoundedRectangle(bounds, corner);
        }
        else
        {
            g.setColour(isHighlighted ? buttonHighlightColour : buttonColour);
            g.fillRoundedRectangle(bounds, corner);
            g.setColour(isHighlighted ? accentColour : accentColour.withAlpha(0.28f));
            g.drawRoundedRectangle(bounds, corner, 1.0f);
        }
    }

    void drawButtonText(juce::Graphics& g, juce::TextButton& button,
                        bool isHighlighted, bool isDown) override
    {
        auto bounds = button.getLocalBounds().toFloat();
        const bool active = button.getToggleState() || isDown;
        const juce::String text = button.getButtonText();

        const float fontSize = (text.length() > 8) ? 9.5f : 10.5f;
        g.setFont(juce::FontOptions(fontSize, juce::Font::bold));

        if (active)
            g.setColour(textActiveColour);
        else
            g.setColour(isHighlighted ? textHighlightColour : textColour);

        g.drawText(text, bounds, juce::Justification::centred, true);
    }

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool isHighlighted, bool isDown) override
    {
        drawButtonBackground(g, button, juce::Colour(), isHighlighted, isDown);
        auto bounds = button.getLocalBounds().toFloat();
        const bool active = button.getToggleState() || isDown;
        const juce::String text = button.getButtonText();

        const float fontSize = (text.length() > 8) ? 9.0f : 10.0f;
        g.setFont(juce::FontOptions(fontSize, juce::Font::bold));

        if (active)
            g.setColour(textActiveColour);
        else
            g.setColour(isHighlighted ? textHighlightColour : textColour);

        g.drawText(text, bounds, juce::Justification::centred, true);
    }
};

class SampleBrowserModalComponent : public juce::Component,
                                    public SampleScanner::Listener,
                                    public juce::DragAndDropContainer
{
public:
    SampleBrowserModalComponent();
    ~SampleBrowserModalComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;
    bool keyPressed(const juce::KeyPress& key) override;

    void visibilityChanged() override;

    void openBrowser(const juce::File& initialFolder = {}, const juce::File& initialFile = {});

    void setLoadButtonText(const juce::String& text);
    void setAssignFolderButtonText(const juce::String& text);
    void setAuditionVolume(float vol);
    float getAuditionVolume() const;

    void setColors(juce::Colour accent, juce::Colour bg, juce::Colour panel, juce::Colour button);

    // Callbacks for host
    std::function<void(const juce::File& file)> onSampleLoaded;
    std::function<void(const juce::File& folder)> onFolderAssigned;
    std::function<void(const juce::File& file, float gain)> onPlayAudition;
    std::function<void()> onStopAudition;
    std::function<bool()> isAuditionPlaying;
    std::function<void(float gain)> onSetAuditionGain;
    std::function<juce::Array<juce::File>()> getRecentFoldersCallback;

    // SampleScanner::Listener callbacks
    void scanStarted(int rootId, const juce::String& rootName) override;
    void scanProgress(int rootId, int filesFound, float progress, const juce::String& currentDir) override;
    void scanFinished(int rootId, int totalFiles, int totalFolders, bool cancelled) override;
    void allScansFinished() override;

private:
    void initCommon();
    void refreshRoots();
    void refreshRecentFolders();
    void refreshFolders(const juce::String& folderPath);
    void refreshFiles(const juce::String& folderPath);
    void performSearch(const juce::String& query);

    void navigateToFolder(const juce::String& folderPath);
    void navigateToParent();
    bool hasParentFolder() const;
    void previewSelectedSample();
    void stopPreview();
    void loadSelectedSample();
    void assignCurrentFolder();
    void exportDatabaseToFile();

    juce::Rectangle<int> dialogBounds;

    juce::Colour customAccent { 0xff00e5ff };
    juce::Colour customBackground { 0xff161822 };
    juce::Colour customPanel { 0xff12141c };

    // Header controls
    juce::Label titleLabel;
    juce::TextButton closeButton { "X" };
    juce::TextEditor searchBox;
    juce::Label scannerStatusLabel;
    juce::TextButton cancelScanButton { "STOP" };

    // Left Panel: Roots & Recents
    juce::Label rootsHeaderLabel;
    juce::ListBox rootsListBox;
    juce::TextButton addRootButton { "+ ADD FOLDER" };
    juce::TextButton rescanAllButton { "RESCAN ALL" };

    juce::Label recentsHeaderLabel;
    juce::ListBox recentsListBox;
    juce::TextButton exportDbButton { "EXPORT DATABASE" };

    // Center Panel: Folders
    juce::Label foldersHeaderLabel;
    juce::TextButton upButton { juce::String::fromUTF8("▲ UP") };
    juce::Label breadcrumbLabel;
    juce::ListBox foldersListBox;
    juce::TextButton assignFolderButton { "SET FOLDER" };

    // Right Panel: Files & Audition
    juce::Label filesHeaderLabel;
    juce::ListBox filesListBox;
    juce::TextButton playButton { "PLAY" };
    juce::ToggleButton autoplayToggle { "AUTOPLAY" };
    juce::Label auditionVolLabel;
    juce::Slider auditionVolumeSlider;
    juce::TextButton loadButton { "LOAD" };

    // Data models
    struct RootItem
    {
        int id = 0;
        juce::String path;
        juce::String name;
        int totalFiles = 0;
    };
    std::vector<RootItem> rootItems;

    struct FolderItem
    {
        juce::String path;
        juce::String name;
        int fileCount = 0;
        int subfolderCount = 0;
    };
    std::vector<FolderItem> folderItems;

    struct FileItem
    {
        juce::String filePath;
        juce::String folderPath;
        juce::String name;
        juce::String ext;
        int64_t size = 0;
    };
    std::vector<FileItem> fileItems;

    juce::Array<juce::File> recentFoldersList;

    juce::String currentBrowsedPath;
    juce::String currentDisplayedFolderPath;
    juce::String selectedFilePath;
    bool isSearchingMode = false;

    // Custom ListBox Models
    class RootsListModel : public juce::ListBoxModel
    {
    public:
        explicit RootsListModel(SampleBrowserModalComponent& inOwner) : owner(inOwner) {}
        int getNumRows() override;
        void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
        void listBoxItemClicked(int row, const juce::MouseEvent& e) override;
    private:
        SampleBrowserModalComponent& owner;
    };

    class RecentsListModel : public juce::ListBoxModel
    {
    public:
        explicit RecentsListModel(SampleBrowserModalComponent& inOwner) : owner(inOwner) {}
        int getNumRows() override;
        void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
        void listBoxItemClicked(int row, const juce::MouseEvent& e) override;
    private:
        SampleBrowserModalComponent& owner;
    };

    class FoldersListModel : public juce::ListBoxModel
    {
    public:
        explicit FoldersListModel(SampleBrowserModalComponent& inOwner) : owner(inOwner) {}
        int getNumRows() override;
        void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
        void listBoxItemClicked(int row, const juce::MouseEvent& e) override;
        void listBoxItemDoubleClicked(int row, const juce::MouseEvent& e) override;
        void selectedRowsChanged(int lastRowSelected) override;
    private:
        SampleBrowserModalComponent& owner;
    };

    class FilesListModel : public juce::ListBoxModel
    {
    public:
        explicit FilesListModel(SampleBrowserModalComponent& inOwner) : owner(inOwner) {}
        int getNumRows() override;
        void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
        void listBoxItemClicked(int row, const juce::MouseEvent& e) override;
        void listBoxItemDoubleClicked(int row, const juce::MouseEvent& e) override;
        void selectedRowsChanged(int lastRowSelected) override;
        juce::var getDragSourceDescription(const juce::SparseSet<int>& selectedRows) override;
    private:
        SampleBrowserModalComponent& owner;
    };

    RootsListModel rootsModel;
    RecentsListModel recentsModel;
    FoldersListModel foldersModel;
    FilesListModel filesModel;

    bool isSuppressingAutoplay = false;
    juce::String lastAuditionPath;
    juce::int64 lastAuditionTime = 0;

    BrowserLcdLookAndFeel lcdLnf;
    std::shared_ptr<juce::FileChooser> activeFileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleBrowserModalComponent)
};

} // namespace sampledicer
