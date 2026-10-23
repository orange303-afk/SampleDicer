#pragma once

#include <JuceHeader.h>
#include <sqlite3.h>
#include <vector>

namespace sampledicer
{

struct RootFolderRecord
{
    int id = 0;
    juce::String path;
    juce::String name;
    int64_t lastScanned = 0;
    int totalFiles = 0;
    int totalFolders = 0;
};

struct FolderRecord
{
    int id = 0;
    int rootId = 0;
    juce::String path;
    juce::String parentPath;
    juce::String name;
    int fileCount = 0;
    int subfolderCount = 0;
};

struct FileRecord
{
    int id = 0;
    int rootId = 0;
    juce::String folderPath;
    juce::String filePath;
    juce::String name;
    juce::String ext;
    int64_t size = 0;
    int64_t modTime = 0;
};

class SampleDatabase
{
public:
    static SampleDatabase& getInstance();

    SampleDatabase();
    ~SampleDatabase();

    bool open();
    void close();
    bool isOpen() const { return db != nullptr; }

    // Root folder operations
    int addConnectedRoot(const juce::File& folder);
    bool removeConnectedRoot(int rootId);
    std::vector<RootFolderRecord> getConnectedRoots();
    bool getRootById(int rootId, RootFolderRecord& outRoot);

    // Folder & file checks
    bool hasFolder(const juce::File& folder);
    bool isFolderInsideConnectedRoot(const juce::File& folder);
    int findRootIdForFolder(const juce::File& folder);

    // Fast browsing queries (< 1 ms)
    std::vector<FolderRecord> getSubfolders(const juce::String& parentPath);
    std::vector<FileRecord> getFilesInFolder(const juce::String& folderPath);
    std::vector<FileRecord> getAllFilesInFolderRecursiveRecords(const juce::String& folderPath);
    juce::Array<juce::File> getAllFilesInFolderRecursive(const juce::File& folder);
    juce::Array<juce::File> getFilesForPad(const juce::File& folder, bool searchSubfolders, bool searchNearFolders);
    std::vector<FileRecord> searchFiles(const juce::String& query, int maxResults = 500);

    // Scanner helper methods
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();

    void clearRootData(int rootId);
    int insertFolder(int rootId, const juce::String& path, const juce::String& parentPath, const juce::String& name, int fileCount, int subfolderCount);
    bool insertFile(int rootId, const juce::String& folderPath, const juce::String& filePath, const juce::String& name, const juce::String& ext, int64_t size, int64_t modTime);
    void updateRootStats(int rootId, int totalFiles, int totalFolders, int64_t lastScanned);

    bool exportDatabase(const juce::File& destinationFile);
    juce::File getDatabaseFile() const;

    static bool isSupportedAudioFile(const juce::File& file);
    static bool isSupportedAudioExtension(const juce::String& ext);

private:
    bool createTables();

    sqlite3* db = nullptr;
    mutable juce::CriticalSection lock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleDatabase)
};

} // namespace sampledicer
