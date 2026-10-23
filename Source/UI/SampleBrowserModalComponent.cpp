#include "SampleBrowserModalComponent.h"

namespace sampledicer
{

SampleBrowserModalComponent::SampleBrowserModalComponent()
    : rootsModel(*this),
      recentsModel(*this),
      foldersModel(*this),
      filesModel(*this)
{
    initCommon();
}

void SampleBrowserModalComponent::initCommon()
{
    setAlwaysOnTop(true);
    setWantsKeyboardFocus(true);

    SampleScanner::getInstance().addListener(this);

    // Title
    titleLabel.setText("SAMPLE & FOLDER BROWSER", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, customAccent);
    addAndMakeVisible(titleLabel);

    // Close button
    closeButton.setTooltip("Close browser (Esc)");
    closeButton.setLookAndFeel(&lcdLnf);
    closeButton.onClick = [this]
    {
        stopPreview();
        setVisible(false);
    };
    addAndMakeVisible(closeButton);

    // Search box
    searchBox.setTextToShowWhenEmpty("Search samples across all libraries...", juce::Colour(0xff6e7687));
    searchBox.setFont(juce::FontOptions(12.0f, juce::Font::plain));
    searchBox.setColour(juce::TextEditor::backgroundColourId, customPanel);
    searchBox.setColour(juce::TextEditor::textColourId, juce::Colour(0xffffffff));
    searchBox.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xff2d3345));
    searchBox.setColour(juce::TextEditor::focusedOutlineColourId, customAccent);
    searchBox.onTextChange = [this]()
    {
        performSearch(searchBox.getText());
    };
    addAndMakeVisible(searchBox);

    // Scanner status & Cancel button
    scannerStatusLabel.setFont(juce::FontOptions(11.0f, juce::Font::italic));
    scannerStatusLabel.setColour(juce::Label::textColourId, juce::Colour(0xffffa726));
    scannerStatusLabel.setText("Database Ready", juce::dontSendNotification);
    addAndMakeVisible(scannerStatusLabel);

    cancelScanButton.setLookAndFeel(&lcdLnf);
    cancelScanButton.setTooltip("Stop background scanning");
    cancelScanButton.onClick = [this]
    {
        SampleScanner::getInstance().cancelScan();
        scannerStatusLabel.setText("Scanning cancelled", juce::dontSendNotification);
        cancelScanButton.setVisible(false);
    };
    addChildComponent(cancelScanButton);

    // Left Panel: Roots & Recents
    rootsHeaderLabel.setText("CONNECTED LIBRARIES", juce::dontSendNotification);
    rootsHeaderLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    rootsHeaderLabel.setColour(juce::Label::textColourId, juce::Colour(0xffa0aec0));
    addAndMakeVisible(rootsHeaderLabel);

    rootsListBox.setModel(&rootsModel);
    rootsListBox.setColour(juce::ListBox::backgroundColourId, customPanel);
    rootsListBox.setColour(juce::ListBox::outlineColourId, juce::Colour(0xff2d3345));
    rootsListBox.setRowHeight(28);
    addAndMakeVisible(rootsListBox);

    addRootButton.setLookAndFeel(&lcdLnf);
    addRootButton.setTooltip("Attach a sample library or folder to database");
    addRootButton.onClick = [this]
    {
        activeFileChooser = std::make_shared<juce::FileChooser>("Choose Sample Library Folder to Connect",
            juce::File::getSpecialLocation(juce::File::userMusicDirectory));

        activeFileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories,
            [this](const juce::FileChooser& fc)
            {
                auto result = fc.getResult();
                if (result.isDirectory())
                {
                    int rootId = SampleDatabase::getInstance().addConnectedRoot(result);
                    if (rootId > 0)
                    {
                        SampleScanner::getInstance().startScan(rootId);
                        refreshRoots();
                        navigateToFolder(result.getFullPathName());
                    }
                }
            });
    };
    addAndMakeVisible(addRootButton);

    rescanAllButton.setLookAndFeel(&lcdLnf);
    rescanAllButton.setTooltip("Scan all connected libraries for new samples");
    rescanAllButton.onClick = []
    {
        SampleScanner::getInstance().startScan(-1);
    };
    addAndMakeVisible(rescanAllButton);

    recentsHeaderLabel.setText("RECENT FOLDERS", juce::dontSendNotification);
    recentsHeaderLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    recentsHeaderLabel.setColour(juce::Label::textColourId, juce::Colour(0xffa0aec0));
    addAndMakeVisible(recentsHeaderLabel);

    recentsListBox.setModel(&recentsModel);
    recentsListBox.setColour(juce::ListBox::backgroundColourId, customPanel);
    recentsListBox.setColour(juce::ListBox::outlineColourId, juce::Colour(0xff2d3345));
    recentsListBox.setRowHeight(24);
    addAndMakeVisible(recentsListBox);

    exportDbButton.setLookAndFeel(&lcdLnf);
    exportDbButton.setTooltip("Export SQLite sample library database backup (.db)");
    exportDbButton.onClick = [this] { exportDatabaseToFile(); };
    addAndMakeVisible(exportDbButton);

    // Center Panel: Folders
    foldersHeaderLabel.setText("FOLDERS", juce::dontSendNotification);
    foldersHeaderLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    foldersHeaderLabel.setColour(juce::Label::textColourId, juce::Colour(0xffa0aec0));
    addAndMakeVisible(foldersHeaderLabel);

    upButton.setLookAndFeel(&lcdLnf);
    upButton.setTooltip("Navigate to parent folder (Backspace)");
    upButton.onClick = [this] { navigateToParent(); };
    addAndMakeVisible(upButton);

    breadcrumbLabel.setFont(juce::FontOptions(10.0f, juce::Font::plain));
    breadcrumbLabel.setColour(juce::Label::textColourId, customAccent);
    breadcrumbLabel.setMinimumHorizontalScale(0.85f);
    breadcrumbLabel.setText("/", juce::dontSendNotification);
    addAndMakeVisible(breadcrumbLabel);

    foldersListBox.setModel(&foldersModel);
    foldersListBox.setColour(juce::ListBox::backgroundColourId, customPanel);
    foldersListBox.setColour(juce::ListBox::outlineColourId, juce::Colour(0xff2d3345));
    foldersListBox.setRowHeight(24);
    addAndMakeVisible(foldersListBox);

    assignFolderButton.setLookAndFeel(&lcdLnf);
    assignFolderButton.setTooltip("Set browsed folder as current slot folder");
    assignFolderButton.onClick = [this] { assignCurrentFolder(); };
    addAndMakeVisible(assignFolderButton);

    // Right Panel: Files & Audition
    filesHeaderLabel.setText("SAMPLES", juce::dontSendNotification);
    filesHeaderLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    filesHeaderLabel.setColour(juce::Label::textColourId, juce::Colour(0xffa0aec0));
    addAndMakeVisible(filesHeaderLabel);

    filesListBox.setModel(&filesModel);
    filesListBox.setColour(juce::ListBox::backgroundColourId, customPanel);
    filesListBox.setColour(juce::ListBox::outlineColourId, juce::Colour(0xff2d3345));
    filesListBox.setRowHeight(24);
    addAndMakeVisible(filesListBox);

    playButton.setLookAndFeel(&lcdLnf);
    playButton.setTooltip("Audition sample (Spacebar)");
    playButton.onClick = [this]
    {
        bool isPlaying = false;
        if (isAuditionPlaying)
            isPlaying = isAuditionPlaying();

        if (isPlaying)
            stopPreview();
        else
            previewSelectedSample();
    };
    addAndMakeVisible(playButton);

    autoplayToggle.setLookAndFeel(&lcdLnf);
    autoplayToggle.setTooltip("Automatically play sample on selection");
    autoplayToggle.setToggleState(true, juce::dontSendNotification);
    addAndMakeVisible(autoplayToggle);

    auditionVolLabel.setText("VOL", juce::dontSendNotification);
    auditionVolLabel.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    auditionVolLabel.setColour(juce::Label::textColourId, juce::Colour(0xff718096));
    auditionVolLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(auditionVolLabel);

    auditionVolumeSlider.setSliderStyle(juce::Slider::LinearBar);
    auditionVolumeSlider.setRange(0.0, 100.0, 1.0);
    auditionVolumeSlider.setValue(80.0, juce::dontSendNotification);
    auditionVolumeSlider.setTextValueSuffix("%");
    auditionVolumeSlider.setColour(juce::Slider::trackColourId, customAccent);
    auditionVolumeSlider.setColour(juce::Slider::thumbColourId, customAccent.darker(0.2f));
    auditionVolumeSlider.setColour(juce::Slider::backgroundColourId, customPanel);
    auditionVolumeSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff2d3345));
    auditionVolumeSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xffffffff));
    auditionVolumeSlider.setTooltip("Audition volume");
    auditionVolumeSlider.onValueChange = [this]
    {
        const float vol = getAuditionVolume();
        if (onSetAuditionGain)
            onSetAuditionGain(vol);
    };
    addAndMakeVisible(auditionVolumeSlider);

    loadButton.setLookAndFeel(&lcdLnf);
    loadButton.setTooltip("Load selected sample (Enter)");
    loadButton.onClick = [this] { loadSelectedSample(); };
    addAndMakeVisible(loadButton);
}

SampleBrowserModalComponent::~SampleBrowserModalComponent()
{
    SampleScanner::getInstance().removeListener(this);
    closeButton.setLookAndFeel(nullptr);
    addRootButton.setLookAndFeel(nullptr);
    rescanAllButton.setLookAndFeel(nullptr);
    exportDbButton.setLookAndFeel(nullptr);
    upButton.setLookAndFeel(nullptr);
    assignFolderButton.setLookAndFeel(nullptr);
    playButton.setLookAndFeel(nullptr);
    autoplayToggle.setLookAndFeel(nullptr);
    loadButton.setLookAndFeel(nullptr);
    cancelScanButton.setLookAndFeel(nullptr);
    auditionVolumeSlider.setLookAndFeel(nullptr);
}

void SampleBrowserModalComponent::setColors(juce::Colour accent, juce::Colour bg, juce::Colour panel, juce::Colour button)
{
    customAccent = accent;
    customBackground = bg;
    customPanel = panel;

    lcdLnf.accentColour = accent;
    lcdLnf.buttonColour = button;
    lcdLnf.buttonHighlightColour = button.brighter(0.12f);
    lcdLnf.textColour = accent.withAlpha(0.85f);
    lcdLnf.textHighlightColour = juce::Colours::white;

    titleLabel.setColour(juce::Label::textColourId, accent);
    breadcrumbLabel.setColour(juce::Label::textColourId, accent);
    searchBox.setColour(juce::TextEditor::focusedOutlineColourId, accent);
    searchBox.setColour(juce::TextEditor::backgroundColourId, panel);
    rootsListBox.setColour(juce::ListBox::backgroundColourId, panel);
    recentsListBox.setColour(juce::ListBox::backgroundColourId, panel);
    foldersListBox.setColour(juce::ListBox::backgroundColourId, panel);
    filesListBox.setColour(juce::ListBox::backgroundColourId, panel);
    auditionVolumeSlider.setColour(juce::Slider::trackColourId, accent);
    auditionVolumeSlider.setColour(juce::Slider::thumbColourId, accent.darker(0.2f));
    auditionVolumeSlider.setColour(juce::Slider::backgroundColourId, panel);

    repaint();
}

void SampleBrowserModalComponent::visibilityChanged()
{
    if (auto* p = getParentComponent())
        p->repaint();
}

void SampleBrowserModalComponent::openBrowser(const juce::File& initialFolder, const juce::File& initialFile)
{
    refreshRoots();
    refreshRecentFolders();

    if (initialFolder.isDirectory())
    {
        navigateToFolder(initialFolder.getFullPathName());
    }
    else if (initialFile.existsAsFile() && initialFile.getParentDirectory().isDirectory())
    {
        navigateToFolder(initialFile.getParentDirectory().getFullPathName());
    }
    else if (!rootItems.empty())
    {
        navigateToFolder(rootItems[0].path);
    }
    else
    {
        currentBrowsedPath = "";
        breadcrumbLabel.setText("/", juce::dontSendNotification);
        breadcrumbLabel.setTooltip("/");
        upButton.setEnabled(false);
        refreshFolders("");
        refreshFiles("");
    }

    if (initialFile.existsAsFile())
    {
        selectedFilePath = initialFile.getFullPathName();
        for (int i = 0; i < static_cast<int>(fileItems.size()); ++i)
        {
            if (fileItems[static_cast<size_t>(i)].filePath == selectedFilePath)
            {
                filesListBox.selectRow(i);
                filesListBox.scrollToEnsureRowIsOnscreen(i);
                break;
            }
        }
    }

    setVisible(true);
    toFront(true);
    grabKeyboardFocus();
}

void SampleBrowserModalComponent::setLoadButtonText(const juce::String& text)
{
    loadButton.setButtonText(text);
}

void SampleBrowserModalComponent::setAssignFolderButtonText(const juce::String& text)
{
    assignFolderButton.setButtonText(text);
}

void SampleBrowserModalComponent::setAuditionVolume(float vol)
{
    auditionVolumeSlider.setValue(static_cast<double>(juce::jlimit(0.0f, 1.0f, vol)) * 100.0, juce::dontSendNotification);
}

float SampleBrowserModalComponent::getAuditionVolume() const
{
    return static_cast<float>(auditionVolumeSlider.getValue() / 100.0);
}

void SampleBrowserModalComponent::refreshRoots()
{
    auto roots = SampleDatabase::getInstance().getConnectedRoots();
    rootItems.clear();
    for (const auto& r : roots)
    {
        RootItem item;
        item.id = r.id;
        item.path = r.path;
        item.name = r.name;
        item.totalFiles = r.totalFiles;
        rootItems.push_back(item);
    }
    rootsListBox.updateContent();
    rootsListBox.repaint();
}

void SampleBrowserModalComponent::refreshRecentFolders()
{
    if (getRecentFoldersCallback)
        recentFoldersList = getRecentFoldersCallback();
    else
        recentFoldersList.clear();

    recentsListBox.updateContent();
    recentsListBox.repaint();
}

void SampleBrowserModalComponent::refreshFolders(const juce::String& folderPath)
{
    folderItems.clear();
    if (folderPath.isNotEmpty())
    {
        auto subs = SampleDatabase::getInstance().getSubfolders(folderPath);
        if (!subs.empty())
        {
            for (const auto& s : subs)
            {
                FolderItem item;
                item.path = s.path;
                item.name = s.name;
                item.fileCount = s.fileCount;
                item.subfolderCount = s.subfolderCount;
                folderItems.push_back(item);
            }
        }
        else
        {
            // Disk fallback if folder is unindexed or not yet in database
            juce::File f(folderPath);
            if (f.isDirectory())
            {
                juce::Array<juce::File> diskDirs;
                f.findChildFiles(diskDirs, juce::File::findDirectories, false);
                for (const auto& d : diskDirs)
                {
                    FolderItem item;
                    item.path = d.getFullPathName();
                    item.name = d.getFileName();
                    item.fileCount = 0;
                    item.subfolderCount = 0;
                    folderItems.push_back(item);
                }
            }
        }
    }
    foldersListBox.updateContent();
    foldersListBox.repaint();
}

void SampleBrowserModalComponent::refreshFiles(const juce::String& folderPath)
{
    fileItems.clear();
    currentDisplayedFolderPath = folderPath;
    if (folderPath.isNotEmpty())
    {
        // Recursive retrieval: fetch all audio files in this folder and all subfolders
        auto fls = SampleDatabase::getInstance().getAllFilesInFolderRecursiveRecords(folderPath);
        if (!fls.empty())
        {
            for (const auto& f : fls)
            {
                FileItem item;
                item.filePath = f.filePath;
                item.folderPath = f.folderPath;
                item.name = f.name;
                item.ext = f.ext;
                item.size = f.size;
                fileItems.push_back(item);
            }
        }
        else
        {
            // Recursive disk fallback if folder is unindexed
            juce::File f(folderPath);
            if (f.isDirectory())
            {
                juce::Array<juce::File> diskFiles;
                f.findChildFiles(diskFiles, juce::File::findFiles, true);
                for (const auto& df : diskFiles)
                {
                    if (SampleDatabase::isSupportedAudioFile(df))
                    {
                        FileItem item;
                        item.filePath = df.getFullPathName();
                        item.folderPath = df.getParentDirectory().getFullPathName();
                        item.name = df.getFileName();
                        item.ext = df.getFileExtension();
                        item.size = df.getSize();
                        fileItems.push_back(item);
                    }
                }
            }
        }
    }
    filesListBox.updateContent();
    filesListBox.repaint();

    juce::String headerText = "SAMPLES";
    if (folderPath.isNotEmpty())
    {
        juce::File f(folderPath);
        juce::String fName = f.getFileName();
        if (fName.isNotEmpty())
            headerText += ": " + fName;
    }
    filesHeaderLabel.setText(headerText + " (" + juce::String(fileItems.size()) + ")", juce::dontSendNotification);
}

void SampleBrowserModalComponent::performSearch(const juce::String& query)
{
    const auto trimmed = query.trim();
    if (trimmed.isEmpty())
    {
        isSearchingMode = false;
        refreshFiles(currentBrowsedPath);
        return;
    }

    isSearchingMode = true;
    auto results = SampleDatabase::getInstance().searchFiles(trimmed, 300);
    fileItems.clear();
    for (const auto& f : results)
    {
        FileItem item;
        item.filePath = f.filePath;
        item.folderPath = f.folderPath;
        item.name = f.name;
        item.ext = f.ext;
        item.size = f.size;
        fileItems.push_back(item);
    }
    filesListBox.updateContent();
    filesListBox.repaint();
    filesHeaderLabel.setText("SEARCH: \"" + trimmed + "\" (" + juce::String(fileItems.size()) + ")", juce::dontSendNotification);
}

void SampleBrowserModalComponent::navigateToFolder(const juce::String& folderPath)
{
    currentBrowsedPath = folderPath;
    currentDisplayedFolderPath = folderPath;
    juce::File f(folderPath);
    juce::String displayPath = f.getFileName().isEmpty() ? folderPath : f.getFullPathName();
    breadcrumbLabel.setText(displayPath, juce::dontSendNotification);
    breadcrumbLabel.setTooltip(displayPath);

    upButton.setEnabled(hasParentFolder());

    refreshFolders(folderPath);
    foldersListBox.deselectAllRows();
    refreshFiles(folderPath);
}

void SampleBrowserModalComponent::navigateToParent()
{
    if (currentBrowsedPath.isEmpty())
        return;

    juce::File f(currentBrowsedPath);
    auto parent = f.getParentDirectory();
    if (parent.exists() && parent != f)
    {
        navigateToFolder(parent.getFullPathName());
    }
}

bool SampleBrowserModalComponent::hasParentFolder() const
{
    if (currentBrowsedPath.isEmpty())
        return false;

    juce::File f(currentBrowsedPath);
    auto parent = f.getParentDirectory();
    return parent.exists() && parent != f;
}

void SampleBrowserModalComponent::previewSelectedSample()
{
    if (selectedFilePath.isEmpty())
        return;

    juce::File f(selectedFilePath);
    if (f.existsAsFile())
    {
        const float gain = getAuditionVolume();
        if (onPlayAudition)
            onPlayAudition(f, gain);

        playButton.setButtonText("STOP");
    }
}

void SampleBrowserModalComponent::stopPreview()
{
    if (onStopAudition)
        onStopAudition();

    playButton.setButtonText("PLAY");
}

void SampleBrowserModalComponent::loadSelectedSample()
{
    if (selectedFilePath.isEmpty())
        return;

    juce::File f(selectedFilePath);
    if (f.existsAsFile())
    {
        stopPreview();
        if (onSampleLoaded)
            onSampleLoaded(f);
        setVisible(false);
    }
}

void SampleBrowserModalComponent::assignCurrentFolder()
{
    juce::String folderToAssign = currentBrowsedPath;

    int selectedRow = foldersListBox.getSelectedRow();
    if (selectedRow >= 0)
    {
        const bool hasParent = hasParentFolder();
        if (!hasParent || selectedRow > 0)
        {
            const int itemIdx = hasParent ? (selectedRow - 1) : selectedRow;
            if (itemIdx >= 0 && itemIdx < static_cast<int>(folderItems.size()))
            {
                folderToAssign = folderItems[static_cast<size_t>(itemIdx)].path;
            }
        }
    }

    if (folderToAssign.isEmpty())
        return;

    juce::File f(folderToAssign);
    if (f.isDirectory())
    {
        if (onFolderAssigned)
            onFolderAssigned(f);
    }
}

void SampleBrowserModalComponent::exportDatabaseToFile()
{
    activeFileChooser = std::make_shared<juce::FileChooser>(
        "Export Sample Database",
        juce::File::getSpecialLocation(juce::File::userDesktopDirectory).getChildFile("SampleLibrary_Backup.db"),
        "*.db");

    activeFileChooser->launchAsync(
        juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::warnAboutOverwriting,
        [this](const juce::FileChooser& fc)
        {
            auto result = fc.getResult();
            if (result != juce::File())
            {
                if (result.getFileExtension().isEmpty())
                    result = result.withFileExtension("db");

                bool ok = SampleDatabase::getInstance().exportDatabase(result);
                if (ok)
                {
                    scannerStatusLabel.setText("Database exported: " + result.getFileName(), juce::dontSendNotification);
                }
                else
                {
                    scannerStatusLabel.setText("Export failed!", juce::dontSendNotification);
                }
            }
        });
}

bool SampleBrowserModalComponent::keyPressed(const juce::KeyPress& key)
{
    if (key == juce::KeyPress::escapeKey)
    {
        stopPreview();
        setVisible(false);
        return true;
    }
    if (key == juce::KeyPress::backspaceKey || (key.getModifiers().isCommandDown() && key.isKeyCode(juce::KeyPress::upKey)))
    {
        navigateToParent();
        return true;
    }
    if (key == juce::KeyPress::spaceKey)
    {
        bool isPlaying = false;
        if (isAuditionPlaying)
            isPlaying = isAuditionPlaying();

        if (isPlaying)
            stopPreview();
        else
            previewSelectedSample();
        return true;
    }
    if (key == juce::KeyPress::returnKey)
    {
        if (foldersListBox.hasKeyboardFocus(true))
        {
            int sel = foldersListBox.getSelectedRow();
            if (sel >= 0)
            {
                const bool hasParent = hasParentFolder();
                if (hasParent && sel == 0)
                {
                    navigateToParent();
                    return true;
                }
                const int itemIdx = hasParent ? (sel - 1) : sel;
                if (itemIdx >= 0 && itemIdx < static_cast<int>(folderItems.size()))
                {
                    navigateToFolder(folderItems[static_cast<size_t>(itemIdx)].path);
                    return true;
                }
            }
        }
        else
        {
            loadSelectedSample();
            return true;
        }
    }
    return Component::keyPressed(key);
}

void SampleBrowserModalComponent::mouseDown(const juce::MouseEvent& e)
{
    if (!dialogBounds.contains(e.getPosition()))
    {
        stopPreview();
        setVisible(false);
    }
}

// Scanner callbacks
void SampleBrowserModalComponent::scanStarted(int /*rootId*/, const juce::String& rootName)
{
    scannerStatusLabel.setText("Scanning: " + rootName + "...", juce::dontSendNotification);
    cancelScanButton.setVisible(true);
}

void SampleBrowserModalComponent::scanProgress(int /*rootId*/, int filesFound, float /*progress*/, const juce::String& currentDir)
{
    scannerStatusLabel.setText("Scanning " + currentDir + ": " + juce::String(filesFound) + " samples", juce::dontSendNotification);
    cancelScanButton.setVisible(true);
}

void SampleBrowserModalComponent::scanFinished(int /*rootId*/, int totalFiles, int /*totalFolders*/, bool cancelled)
{
    scannerStatusLabel.setText(cancelled ? "Scan cancelled" : ("Scan done: " + juce::String(totalFiles) + " samples"), juce::dontSendNotification);
    cancelScanButton.setVisible(false);
    refreshRoots();
    if (currentBrowsedPath.isNotEmpty())
    {
        refreshFolders(currentBrowsedPath);
        refreshFiles(currentBrowsedPath);
    }
}

void SampleBrowserModalComponent::allScansFinished()
{
    cancelScanButton.setVisible(false);
    refreshRoots();
}

void SampleBrowserModalComponent::paint(juce::Graphics& g)
{
    // Semi-transparent backdrop
    g.fillAll(juce::Colour(0xee08090e));

    if (dialogBounds.isEmpty())
        return;

    auto dialogF = dialogBounds.toFloat();

    // Dialog chassis
    g.setColour(customBackground);
    g.fillRoundedRectangle(dialogF, 8.0f);

    // Drop shadow
    g.setColour(juce::Colour(0xff000000).withAlpha(0.6f));
    g.fillRoundedRectangle(dialogF.expanded(6.0f), 10.0f);

    // Inner background gradient
    juce::ColourGradient bgGrad(customBackground.brighter(0.08f), dialogF.getCentreX(), dialogF.getY(),
                                customPanel.darker(0.1f), dialogF.getCentreX(), dialogF.getBottom(), false);
    g.setGradientFill(bgGrad);
    g.fillRoundedRectangle(dialogF, 8.0f);

    // Outline
    g.setColour(customAccent.withAlpha(0.5f));
    g.drawRoundedRectangle(dialogF, 8.0f, 1.5f);

    // Header strip area
    auto headerF = dialogF.removeFromTop(38.0f);
    g.setColour(juce::Colour(0x18ffffff));
    g.fillRect(headerF);
    g.setColour(juce::Colour(0xff000000).withAlpha(0.4f));
    g.drawHorizontalLine(static_cast<int>(headerF.getBottom()), dialogF.getX(), dialogF.getRight());
}

void SampleBrowserModalComponent::resized()
{
    const auto area = getLocalBounds();
    const int dialogW = juce::jmin(area.getWidth() - 30, 960);
    const int dialogH = juce::jmin(area.getHeight() - 30, 620);
    dialogBounds = juce::Rectangle<int>((area.getWidth() - dialogW) / 2, (area.getHeight() - dialogH) / 2, dialogW, dialogH);

    auto dArea = dialogBounds;

    // Header layout
    auto header = dArea.removeFromTop(38).reduced(10, 4);
    titleLabel.setBounds(header.removeFromLeft(210));
    closeButton.setBounds(header.removeFromRight(26).reduced(1));

    header.removeFromRight(10);
    if (cancelScanButton.isVisible())
        cancelScanButton.setBounds(header.removeFromRight(50).reduced(2));
    scannerStatusLabel.setBounds(header.removeFromRight(240));

    header.removeFromRight(10);
    searchBox.setBounds(header.removeFromRight(juce::jmin(300, header.getWidth())));

    dArea.reduce(8, 8);

    // 3 Panels layout
    const int leftW = 210;
    const int centerW = 260;

    auto leftArea = dArea.removeFromLeft(leftW);
    dArea.removeFromLeft(6); // separator gap
    auto centerArea = dArea.removeFromLeft(centerW);
    dArea.removeFromLeft(6); // separator gap
    auto rightArea = dArea; // remainder

    // Left Panel Layout: Roots (top 58%) + Recents (bottom 42%)
    const int leftTopH = leftArea.proportionOfHeight(0.58f);
    auto rootsArea = leftArea.removeFromTop(leftTopH);
    rootsHeaderLabel.setBounds(rootsArea.removeFromTop(20));
    auto rootsButtons = rootsArea.removeFromBottom(26);
    addRootButton.setBounds(rootsButtons.removeFromLeft(102).reduced(1));
    rescanAllButton.setBounds(rootsButtons.removeFromLeft(102).reduced(1));
    rootsArea.removeFromBottom(4);
    rootsListBox.setBounds(rootsArea);

    leftArea.removeFromTop(6);
    recentsHeaderLabel.setBounds(leftArea.removeFromTop(20));
    auto exportBar = leftArea.removeFromBottom(26);
    exportDbButton.setBounds(exportBar.reduced(1));
    leftArea.removeFromBottom(4);
    recentsListBox.setBounds(leftArea);

    // Center Panel Layout: Folders
    foldersHeaderLabel.setBounds(centerArea.removeFromTop(20));
    auto breadcrumbRow = centerArea.removeFromTop(26);
    upButton.setBounds(breadcrumbRow.removeFromLeft(80).reduced(1));
    breadcrumbRow.removeFromLeft(4);
    breadcrumbLabel.setBounds(breadcrumbRow);

    auto folderBottomBar = centerArea.removeFromBottom(28);
    assignFolderButton.setBounds(folderBottomBar.reduced(1));
    centerArea.removeFromBottom(4);
    foldersListBox.setBounds(centerArea);

    // Right Panel Layout: Files & Audition
    filesHeaderLabel.setBounds(rightArea.removeFromTop(20));
    auto auditionBar = rightArea.removeFromBottom(28);
    playButton.setBounds(auditionBar.removeFromLeft(54).reduced(1));
    auditionBar.removeFromLeft(4);
    autoplayToggle.setBounds(auditionBar.removeFromLeft(96).reduced(2));
    auditionBar.removeFromLeft(6);
    auditionVolLabel.setBounds(auditionBar.removeFromLeft(28).reduced(1));
    auditionBar.removeFromLeft(2);
    auditionVolumeSlider.setBounds(auditionBar.removeFromLeft(80).reduced(1));
    loadButton.setBounds(auditionBar.removeFromRight(135).reduced(1));
    rightArea.removeFromBottom(4);
    filesListBox.setBounds(rightArea);
}

//==============================================================================
// ListBox Models Implementations
//==============================================================================

// 1. Roots List Model
int SampleBrowserModalComponent::RootsListModel::getNumRows()
{
    return static_cast<int>(owner.rootItems.size());
}

void SampleBrowserModalComponent::RootsListModel::paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    if (row < 0 || row >= static_cast<int>(owner.rootItems.size()))
        return;

    const auto& item = owner.rootItems[static_cast<size_t>(row)];

    if (rowIsSelected)
    {
        g.setColour(owner.customAccent.withAlpha(0.25f));
        g.fillRect(0, 0, width, height);
        g.setColour(owner.customAccent);
        g.fillRect(0, 0, 3, height);
    }
    else if (row % 2 == 1)
    {
        g.setColour(juce::Colour(0x08ffffff));
        g.fillRect(0, 0, width, height);
    }

    g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
    g.setColour(rowIsSelected ? juce::Colour(0xffffffff) : juce::Colour(0xffd0d7de));

    auto textBounds = juce::Rectangle<int>(8, 0, width - 68, height);
    g.drawText(item.name, textBounds, juce::Justification::centredLeft, true);

    // Files count badge
    if (item.totalFiles > 0)
    {
        g.setFont(juce::FontOptions(10.0f, juce::Font::plain));
        g.setColour(juce::Colour(0xff718096));
        juce::String countStr = item.totalFiles >= 1000
            ? juce::String(static_cast<float>(item.totalFiles) / 1000.0f, 1) + "k"
            : juce::String(item.totalFiles);
        g.drawText(countStr, width - 58, 0, 50, height, juce::Justification::centredRight, false);
    }
}

void SampleBrowserModalComponent::RootsListModel::listBoxItemClicked(int row, const juce::MouseEvent& e)
{
    if (row < 0 || row >= static_cast<int>(owner.rootItems.size()))
        return;

    const auto& item = owner.rootItems[static_cast<size_t>(row)];

    if (e.mods.isPopupMenu())
    {
        juce::PopupMenu m;
        m.addItem(1, "Rescan \"" + item.name + "\"");
        m.addItem(2, "Remove from Library");

        m.showMenuAsync(juce::PopupMenu::Options(), [this, item](int result)
        {
            if (result == 1)
            {
                SampleScanner::getInstance().startScan(item.id);
            }
            else if (result == 2)
            {
                SampleDatabase::getInstance().removeConnectedRoot(item.id);
                owner.refreshRoots();
                if (owner.currentBrowsedPath == item.path || owner.currentBrowsedPath.startsWith(item.path + "/"))
                {
                    owner.currentBrowsedPath = "";
                    owner.refreshFolders("");
                    owner.refreshFiles("");
                }
            }
        });
        return;
    }

    owner.navigateToFolder(item.path);
}

// 2. Recents List Model
int SampleBrowserModalComponent::RecentsListModel::getNumRows()
{
    return owner.recentFoldersList.size();
}

void SampleBrowserModalComponent::RecentsListModel::paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    if (row < 0 || row >= owner.recentFoldersList.size())
        return;

    const auto& file = owner.recentFoldersList.getReference(row);

    if (rowIsSelected)
    {
        g.setColour(owner.customAccent.withAlpha(0.2f));
        g.fillRect(0, 0, width, height);
    }

    g.setFont(juce::FontOptions(11.0f, juce::Font::plain));
    g.setColour(rowIsSelected ? juce::Colour(0xffffffff) : juce::Colour(0xffcbd5e0));

    auto name = file.getFileName();
    if (name.isEmpty()) name = file.getFullPathName();
    g.drawText(name, 8, 0, width - 16, height, juce::Justification::centredLeft, true);
}

void SampleBrowserModalComponent::RecentsListModel::listBoxItemClicked(int row, const juce::MouseEvent& /*e*/)
{
    if (row >= 0 && row < owner.recentFoldersList.size())
    {
        const auto& file = owner.recentFoldersList.getReference(row);
        if (file.isDirectory())
            owner.navigateToFolder(file.getFullPathName());
    }
}

// 3. Folders List Model
int SampleBrowserModalComponent::FoldersListModel::getNumRows()
{
    int count = static_cast<int>(owner.folderItems.size());
    if (owner.hasParentFolder())
        count += 1;
    return count;
}

void SampleBrowserModalComponent::FoldersListModel::paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    const bool hasParent = owner.hasParentFolder();
    if (hasParent && row == 0)
    {
        if (rowIsSelected)
        {
            g.setColour(owner.customAccent.withAlpha(0.2f));
            g.fillRect(0, 0, width, height);
        }
        else
        {
            g.setColour(owner.customAccent.withAlpha(0.06f));
            g.fillRect(0, 0, width, height);
        }

        g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        g.setColour(rowIsSelected ? juce::Colour(0xffffffff) : owner.customAccent);
        g.drawText(juce::String::fromUTF8("▲ .. [UP]"), 8, 0, width - 16, height, juce::Justification::centredLeft, true);
        return;
    }

    const int itemIndex = hasParent ? (row - 1) : row;
    if (itemIndex < 0 || itemIndex >= static_cast<int>(owner.folderItems.size()))
        return;

    const auto& item = owner.folderItems[static_cast<size_t>(itemIndex)];

    if (rowIsSelected)
    {
        g.setColour(owner.customAccent.withAlpha(0.2f));
        g.fillRect(0, 0, width, height);
    }
    else if (row % 2 == 1)
    {
        g.setColour(juce::Colour(0x06ffffff));
        g.fillRect(0, 0, width, height);
    }

    g.setFont(juce::FontOptions(11.0f, juce::Font::plain));
    g.setColour(juce::Colour(0xffffa726));
    g.drawText(">", 6, 0, 14, height, juce::Justification::centredLeft, false);

    g.setColour(rowIsSelected ? juce::Colour(0xffffffff) : juce::Colour(0xffe2e8f0));
    g.drawText(item.name, 22, 0, width - 82, height, juce::Justification::centredLeft, true);

    if (item.fileCount > 0)
    {
        g.setFont(juce::FontOptions(10.0f, juce::Font::plain));
        g.setColour(juce::Colour(0xff718096));
        g.drawText(juce::String(item.fileCount), width - 58, 0, 50, height, juce::Justification::centredRight, false);
    }
}

void SampleBrowserModalComponent::FoldersListModel::listBoxItemClicked(int row, const juce::MouseEvent& /*e*/)
{
    const bool hasParent = owner.hasParentFolder();
    if (hasParent && row == 0)
    {
        owner.refreshFiles(owner.currentBrowsedPath);
        return;
    }

    const int itemIndex = hasParent ? (row - 1) : row;
    if (itemIndex >= 0 && itemIndex < static_cast<int>(owner.folderItems.size()))
    {
        const auto& item = owner.folderItems[static_cast<size_t>(itemIndex)];
        owner.refreshFiles(item.path);
    }
}

void SampleBrowserModalComponent::FoldersListModel::selectedRowsChanged(int lastRowSelected)
{
    if (lastRowSelected < 0)
    {
        owner.refreshFiles(owner.currentBrowsedPath);
        return;
    }

    const bool hasParent = owner.hasParentFolder();
    if (hasParent && lastRowSelected == 0)
    {
        owner.refreshFiles(owner.currentBrowsedPath);
        return;
    }

    const int itemIndex = hasParent ? (lastRowSelected - 1) : lastRowSelected;
    if (itemIndex >= 0 && itemIndex < static_cast<int>(owner.folderItems.size()))
    {
        const auto& item = owner.folderItems[static_cast<size_t>(itemIndex)];
        owner.refreshFiles(item.path);
    }
}

void SampleBrowserModalComponent::FoldersListModel::listBoxItemDoubleClicked(int row, const juce::MouseEvent& /*e*/)
{
    const bool hasParent = owner.hasParentFolder();
    if (hasParent && row == 0)
    {
        owner.navigateToParent();
        return;
    }

    const int itemIndex = hasParent ? (row - 1) : row;
    if (itemIndex >= 0 && itemIndex < static_cast<int>(owner.folderItems.size()))
    {
        const auto& item = owner.folderItems[static_cast<size_t>(itemIndex)];
        owner.navigateToFolder(item.path);
    }
}

// 4. Files List Model
int SampleBrowserModalComponent::FilesListModel::getNumRows()
{
    return static_cast<int>(owner.fileItems.size());
}

void SampleBrowserModalComponent::FilesListModel::paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    if (row < 0 || row >= static_cast<int>(owner.fileItems.size()))
        return;

    const auto& item = owner.fileItems[static_cast<size_t>(row)];

    if (rowIsSelected)
    {
        g.setColour(owner.customAccent.withAlpha(0.28f));
        g.fillRect(0, 0, width, height);
        g.setColour(owner.customAccent);
        g.fillRect(0, 0, 3, height);
    }
    else if (row % 2 == 1)
    {
        g.setColour(juce::Colour(0x06ffffff));
        g.fillRect(0, 0, width, height);
    }

    g.setFont(juce::FontOptions(11.0f, juce::Font::plain));
    g.setColour(rowIsSelected ? juce::Colour(0xffffffff) : juce::Colour(0xffedf2f7));

    juce::String relPath;
    const auto basePath = owner.currentDisplayedFolderPath.isNotEmpty()
                            ? owner.currentDisplayedFolderPath
                            : owner.currentBrowsedPath;

    if (owner.isSearchingMode)
    {
        relPath = juce::File(item.folderPath).getFileName();
    }
    else if (item.folderPath.isNotEmpty() && item.folderPath != basePath)
    {
        if (item.folderPath.startsWith(basePath))
        {
            relPath = item.folderPath.substring(basePath.length());
            if (relPath.startsWithChar('/') || relPath.startsWithChar('\\'))
                relPath = relPath.substring(1);
        }
        else
        {
            relPath = juce::File(item.folderPath).getFileName();
        }
    }

    juce::String labelText = relPath.isNotEmpty() ? (relPath + " / " + item.name) : item.name;
    g.drawText(labelText, 10, 0, width - 70, height, juce::Justification::centredLeft, true);

    // Size badge
    if (item.size > 0)
    {
        g.setFont(juce::FontOptions(10.0f, juce::Font::plain));
        g.setColour(juce::Colour(0xff718096));
        juce::String szStr = item.size >= 1048576
            ? juce::String(static_cast<float>(item.size) / 1048576.0f, 1) + "M"
            : juce::String(static_cast<float>(item.size) / 1024.0f, 0) + "K";
        g.drawText(szStr, width - 58, 0, 50, height, juce::Justification::centredRight, false);
    }
}

void SampleBrowserModalComponent::FilesListModel::listBoxItemClicked(int row, const juce::MouseEvent& /*e*/)
{
    if (row >= 0 && row < static_cast<int>(owner.fileItems.size()))
    {
        owner.selectedFilePath = owner.fileItems[static_cast<size_t>(row)].filePath;
        if (owner.autoplayToggle.getToggleState())
        {
            owner.previewSelectedSample();
        }
    }
}

void SampleBrowserModalComponent::FilesListModel::listBoxItemDoubleClicked(int row, const juce::MouseEvent& /*e*/)
{
    if (row >= 0 && row < static_cast<int>(owner.fileItems.size()))
    {
        owner.selectedFilePath = owner.fileItems[static_cast<size_t>(row)].filePath;
        owner.loadSelectedSample();
    }
}

juce::var SampleBrowserModalComponent::FilesListModel::getDragSourceDescription(const juce::SparseSet<int>& selectedRows)
{
    if (selectedRows.isEmpty())
        return {};

    int firstSelected = selectedRows[0];
    if (firstSelected >= 0 && firstSelected < static_cast<int>(owner.fileItems.size()))
    {
        return owner.fileItems[static_cast<size_t>(firstSelected)].filePath;
    }
    return {};
}

} // namespace sampledicer
