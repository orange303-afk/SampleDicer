#pragma once

#include <JuceHeader.h>
#include "SampleDatabase.h"
#include <atomic>
#include <queue>

namespace sampledicer
{

class SampleScanner : public juce::Thread
{
public:
    class Listener
    {
    public:
        virtual ~Listener() = default;
        virtual void scanStarted(int /*rootId*/, const juce::String& /*rootName*/) {}
        virtual void scanProgress(int /*rootId*/, int /*count*/, float /*progress*/, const juce::String& /*currentDir*/) {}
        virtual void scanFinished(int /*rootId*/, int /*totalFiles*/, int /*totalFolders*/, bool /*cancelled*/) {}
        virtual void allScansFinished() {}
    };

    static SampleScanner& getInstance();

    SampleScanner();
    ~SampleScanner() override;

    void addListener(Listener* l);
    void removeListener(Listener* l);

    void startScan(int rootId = -1);
    void cancelScan();

    bool isScanning() const { return scanning.load(); }
    int getFilesFound() const { return filesFound.load(); }
    juce::String getCurrentFolder() const;
    float getProgress() const { return currentProgress.load(); }

    void run() override;

private:
    void notifyScanStarted(int rootId, const juce::String& name);
    void notifyScanProgress(int rootId, int files, float prog, const juce::String& dir);
    void notifyScanFinished(int rootId, int totalFiles, int totalFolders, bool cancelled);
    void notifyAllScansFinished();

    juce::ListenerList<Listener> listeners;
    std::atomic<bool> scanning { false };
    std::atomic<int> filesFound { 0 };
    std::atomic<float> currentProgress { 0.0f };

    mutable juce::CriticalSection pathLock;
    juce::String currentDirName;

    juce::CriticalSection queueLock;
    std::queue<int> rootQueue;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleScanner)
};

} // namespace sampledicer
