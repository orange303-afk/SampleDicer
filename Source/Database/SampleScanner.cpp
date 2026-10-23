#include "SampleScanner.h"

namespace sampledicer
{

SampleScanner& SampleScanner::getInstance()
{
    static SampleScanner instance;
    return instance;
}

SampleScanner::SampleScanner()
    : juce::Thread("LoopDicerSampleScanner")
{
}

SampleScanner::~SampleScanner()
{
    cancelScan();
}

void SampleScanner::addListener(Listener* l)
{
    listeners.add(l);
}

void SampleScanner::removeListener(Listener* l)
{
    listeners.remove(l);
}

void SampleScanner::startScan(int rootId)
{
    {
        const juce::ScopedLock sl(queueLock);
        if (rootId == -1)
        {
            auto roots = SampleDatabase::getInstance().getConnectedRoots();
            for (const auto& r : roots)
                rootQueue.push(r.id);
        }
        else
        {
            rootQueue.push(rootId);
        }
    }

    if (!isThreadRunning())
    {
        scanning.store(true);
        startThread(juce::Thread::Priority::normal);
    }
}

void SampleScanner::cancelScan()
{
    signalThreadShouldExit();
    {
        const juce::ScopedLock sl(queueLock);
        while (!rootQueue.empty())
            rootQueue.pop();
    }
    stopThread(3000);
    scanning.store(false);
}

juce::String SampleScanner::getCurrentFolder() const
{
    const juce::ScopedLock sl(pathLock);
    return currentDirName;
}

void SampleScanner::notifyScanStarted(int rootId, const juce::String& name)
{
    juce::MessageManager::callAsync([this, rootId, name]()
    {
        listeners.call([rootId, name](Listener& l) { l.scanStarted(rootId, name); });
    });
}

void SampleScanner::notifyScanProgress(int rootId, int files, float prog, const juce::String& dir)
{
    juce::MessageManager::callAsync([this, rootId, files, prog, dir]()
    {
        listeners.call([rootId, files, prog, dir](Listener& l) { l.scanProgress(rootId, files, prog, dir); });
    });
}

void SampleScanner::notifyScanFinished(int rootId, int totalFiles, int totalFolders, bool cancelled)
{
    juce::MessageManager::callAsync([this, rootId, totalFiles, totalFolders, cancelled]()
    {
        listeners.call([rootId, totalFiles, totalFolders, cancelled](Listener& l) { l.scanFinished(rootId, totalFiles, totalFolders, cancelled); });
    });
}

void SampleScanner::notifyAllScansFinished()
{
    juce::MessageManager::callAsync([this]()
    {
        listeners.call([](Listener& l) { l.allScansFinished(); });
    });
}

void SampleScanner::run()
{
    while (!threadShouldExit())
    {
        int rootId = -1;
        {
            const juce::ScopedLock sl(queueLock);
            if (rootQueue.empty())
                break;
            rootId = rootQueue.front();
            rootQueue.pop();
        }

        RootFolderRecord root;
        if (!SampleDatabase::getInstance().getRootById(rootId, root))
            continue;

        juce::File rootDir(root.path);
        if (!rootDir.isDirectory())
            continue;

        notifyScanStarted(rootId, root.name);

        SampleDatabase::getInstance().clearRootData(rootId);
        SampleDatabase::getInstance().beginTransaction();

        std::vector<juce::File> dirsToScan;
        dirsToScan.push_back(rootDir);

        int totalFilesCount = 0;
        int totalFoldersCount = 0;
        int batchCounter = 0;
        int64_t lastNotifyTime = juce::Time::currentTimeMillis();

        bool wasCancelled = false;

        while (!dirsToScan.empty() && !threadShouldExit())
        {
            juce::File curDir = dirsToScan.back();
            dirsToScan.pop_back();

            ++totalFoldersCount;

            {
                const juce::ScopedLock sl(pathLock);
                currentDirName = curDir.getFileName();
            }

            juce::Array<juce::File> subDirs;
            juce::Array<juce::File> audioFiles;

            curDir.findChildFiles(subDirs, juce::File::findDirectories, false);
            curDir.findChildFiles(audioFiles, juce::File::findFiles, false, "*.wav;*.aif;*.aiff;*.mp3;*.flac;*.ogg");

            // Insert folder
            juce::String parentPath = curDir == rootDir ? rootDir.getParentDirectory().getFullPathName()
                                                        : curDir.getParentDirectory().getFullPathName();
            SampleDatabase::getInstance().insertFolder(rootId,
                                                      curDir.getFullPathName(),
                                                      parentPath,
                                                      curDir.getFileName(),
                                                      audioFiles.size(),
                                                      subDirs.size());

            // Insert files
            for (const auto& f : audioFiles)
            {
                if (threadShouldExit())
                {
                    wasCancelled = true;
                    break;
                }

                SampleDatabase::getInstance().insertFile(rootId,
                                                         curDir.getFullPathName(),
                                                         f.getFullPathName(),
                                                         f.getFileName(),
                                                         f.getFileExtension(),
                                                         f.getSize(),
                                                         f.getLastModificationTime().toMilliseconds());
                ++totalFilesCount;
                ++batchCounter;

                if (batchCounter >= 1000)
                {
                    SampleDatabase::getInstance().commitTransaction();
                    SampleDatabase::getInstance().beginTransaction();
                    batchCounter = 0;
                }
            }

            if (wasCancelled)
                break;

            // Push subdirectories
            for (const auto& sd : subDirs)
                dirsToScan.push_back(sd);

            filesFound.store(totalFilesCount);

            const int64_t now = juce::Time::currentTimeMillis();
            if (now - lastNotifyTime >= 100)
            {
                lastNotifyTime = now;
                notifyScanProgress(rootId, totalFilesCount, 0.0f, curDir.getFileName());
            }
        }

        if (threadShouldExit() || wasCancelled)
        {
            SampleDatabase::getInstance().rollbackTransaction();
            notifyScanFinished(rootId, totalFilesCount, totalFoldersCount, true);
            break;
        }
        else
        {
            SampleDatabase::getInstance().commitTransaction();
            SampleDatabase::getInstance().updateRootStats(rootId, totalFilesCount, totalFoldersCount, juce::Time::currentTimeMillis());
            notifyScanFinished(rootId, totalFilesCount, totalFoldersCount, false);
        }
    }

    scanning.store(false);
    notifyAllScansFinished();
}

} // namespace sampledicer
