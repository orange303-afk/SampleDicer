#include "SampleDatabase.h"

namespace sampledicer
{

SampleDatabase& SampleDatabase::getInstance()
{
    static SampleDatabase instance;
    return instance;
}

SampleDatabase::SampleDatabase()
{
    open();
}

SampleDatabase::~SampleDatabase()
{
    close();
}

juce::File SampleDatabase::getDatabaseFile() const
{
    auto orangeAudioDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("Orange Audio");
    auto sharedDb = orangeAudioDir.getChildFile("sample_library.db");
    if (sharedDb.existsAsFile())
        return sharedDb;

    auto loopDicerDb = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("Loop Dicer")
        .getChildFile("sample_library.db");
    if (loopDicerDb.existsAsFile())
        return loopDicerDb;

    auto drumDicerDb = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("Drum Dicer")
        .getChildFile("sample_library.db");
    if (drumDicerDb.existsAsFile())
        return drumDicerDb;

    return sharedDb;
}

bool SampleDatabase::open()
{
    const juce::ScopedLock sl(lock);
    if (db != nullptr)
        return true;

    auto dbFile = getDatabaseFile();
    dbFile.getParentDirectory().createDirectory();

    int rc = sqlite3_open_v2(dbFile.getFullPathName().toUTF8(), &db,
                             SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX,
                             nullptr);
    if (rc != SQLITE_OK)
    {
        db = nullptr;
        return false;
    }

    sqlite3_exec(db, "PRAGMA journal_mode = WAL;", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "PRAGMA synchronous = NORMAL;", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);

    return createTables();
}

void SampleDatabase::close()
{
    const juce::ScopedLock sl(lock);
    if (db != nullptr)
    {
        sqlite3_close_v2(db);
        db = nullptr;
    }
}

bool SampleDatabase::createTables()
{
    const char* schema = R"(
        CREATE TABLE IF NOT EXISTS connected_roots (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            path TEXT UNIQUE NOT NULL,
            name TEXT NOT NULL,
            last_scanned INTEGER DEFAULT 0,
            total_files INTEGER DEFAULT 0,
            total_folders INTEGER DEFAULT 0
        );

        CREATE TABLE IF NOT EXISTS folders (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            root_id INTEGER NOT NULL,
            path TEXT UNIQUE NOT NULL,
            parent_path TEXT NOT NULL,
            name TEXT NOT NULL,
            file_count INTEGER DEFAULT 0,
            subfolder_count INTEGER DEFAULT 0,
            FOREIGN KEY(root_id) REFERENCES connected_roots(id) ON DELETE CASCADE
        );

        CREATE TABLE IF NOT EXISTS files (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            root_id INTEGER NOT NULL,
            folder_path TEXT NOT NULL,
            file_path TEXT UNIQUE NOT NULL,
            name TEXT NOT NULL,
            ext TEXT NOT NULL,
            size INTEGER DEFAULT 0,
            mod_time INTEGER DEFAULT 0,
            FOREIGN KEY(root_id) REFERENCES connected_roots(id) ON DELETE CASCADE
        );

        CREATE INDEX IF NOT EXISTS idx_folders_parent ON folders(parent_path);
        CREATE INDEX IF NOT EXISTS idx_folders_root ON folders(root_id);
        CREATE INDEX IF NOT EXISTS idx_folders_path ON folders(path);
        CREATE INDEX IF NOT EXISTS idx_files_folder ON files(folder_path);
        CREATE INDEX IF NOT EXISTS idx_files_root ON files(root_id);
        CREATE INDEX IF NOT EXISTS idx_files_name ON files(name);
    )";

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, schema, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK)
    {
        if (errMsg != nullptr)
            sqlite3_free(errMsg);
        return false;
    }
    return true;
}

int SampleDatabase::addConnectedRoot(const juce::File& folder)
{
    if (!folder.isDirectory())
        return -1;

    const juce::ScopedLock sl(lock);
    if (db == nullptr && !open())
        return -1;

    const auto fullPath = folder.getFullPathName();
    const auto name = folder.getFileName();

    // Check if already exists
    sqlite3_stmt* checkStmt = nullptr;
    if (sqlite3_prepare_v2(db, "SELECT id FROM connected_roots WHERE path = ? LIMIT 1;", -1, &checkStmt, nullptr) == SQLITE_OK)
    {
        sqlite3_bind_text(checkStmt, 1, fullPath.toUTF8(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(checkStmt) == SQLITE_ROW)
        {
            int existingId = sqlite3_column_int(checkStmt, 0);
            sqlite3_finalize(checkStmt);
            return existingId;
        }
        sqlite3_finalize(checkStmt);
    }

    sqlite3_stmt* insertStmt = nullptr;
    if (sqlite3_prepare_v2(db, "INSERT INTO connected_roots (path, name) VALUES (?, ?);", -1, &insertStmt, nullptr) == SQLITE_OK)
    {
        sqlite3_bind_text(insertStmt, 1, fullPath.toUTF8(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(insertStmt, 2, name.toUTF8(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(insertStmt) == SQLITE_DONE)
        {
            int newId = static_cast<int>(sqlite3_last_insert_rowid(db));
            sqlite3_finalize(insertStmt);
            return newId;
        }
        sqlite3_finalize(insertStmt);
    }

    return -1;
}

bool SampleDatabase::removeConnectedRoot(int rootId)
{
    const juce::ScopedLock sl(lock);
    if (db == nullptr)
        return false;

    clearRootData(rootId);

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, "DELETE FROM connected_roots WHERE id = ?;", -1, &stmt, nullptr) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, rootId);
        bool success = (sqlite3_step(stmt) == SQLITE_DONE);
        sqlite3_finalize(stmt);
        return success;
    }
    return false;
}

std::vector<RootFolderRecord> SampleDatabase::getConnectedRoots()
{
    std::vector<RootFolderRecord> results;
    const juce::ScopedLock sl(lock);
    if (db == nullptr && !open())
        return results;

    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT id, path, name, last_scanned, total_files, total_folders FROM connected_roots ORDER BY name COLLATE NOCASE;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK)
    {
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            RootFolderRecord rec;
            rec.id = sqlite3_column_int(stmt, 0);
            rec.path = juce::String::fromUTF8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
            rec.name = juce::String::fromUTF8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
            rec.lastScanned = sqlite3_column_int64(stmt, 3);
            rec.totalFiles = sqlite3_column_int(stmt, 4);
            rec.totalFolders = sqlite3_column_int(stmt, 5);
            results.push_back(rec);
        }
        sqlite3_finalize(stmt);
    }
    return results;
}

bool SampleDatabase::getRootById(int rootId, RootFolderRecord& outRoot)
{
    const juce::ScopedLock sl(lock);
    if (db == nullptr)
        return false;

    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT id, path, name, last_scanned, total_files, total_folders FROM connected_roots WHERE id = ? LIMIT 1;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, rootId);
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            outRoot.id = sqlite3_column_int(stmt, 0);
            outRoot.path = juce::String::fromUTF8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
            outRoot.name = juce::String::fromUTF8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
            outRoot.lastScanned = sqlite3_column_int64(stmt, 3);
            outRoot.totalFiles = sqlite3_column_int(stmt, 4);
            outRoot.totalFolders = sqlite3_column_int(stmt, 5);
            sqlite3_finalize(stmt);
            return true;
        }
        sqlite3_finalize(stmt);
    }
    return false;
}

static juce::String cleanFolderPath(juce::String path)
{
    while ((path.endsWithChar('/') || path.endsWithChar('\\')) && path.length() > 1)
        path = path.dropLastCharacters(1);
    return path;
}

bool SampleDatabase::hasFolder(const juce::File& folder)
{
    const juce::ScopedLock sl(lock);
    if (db == nullptr)
        return false;

    const auto p = cleanFolderPath(folder.getFullPathName());
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, "SELECT 1 FROM folders WHERE path = ? LIMIT 1;", -1, &stmt, nullptr) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, p.toUTF8(), -1, SQLITE_TRANSIENT);
        bool found = (sqlite3_step(stmt) == SQLITE_ROW);
        sqlite3_finalize(stmt);
        if (found)
            return true;
    }

    if (sqlite3_prepare_v2(db, "SELECT 1 FROM connected_roots WHERE path = ? LIMIT 1;", -1, &stmt, nullptr) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, p.toUTF8(), -1, SQLITE_TRANSIENT);
        bool found = (sqlite3_step(stmt) == SQLITE_ROW);
        sqlite3_finalize(stmt);
        return found;
    }

    return false;
}

bool SampleDatabase::isFolderInsideConnectedRoot(const juce::File& folder)
{
    const auto fullPath = cleanFolderPath(folder.getFullPathName());
    auto roots = getConnectedRoots();
    for (const auto& r : roots)
    {
        const auto rPath = cleanFolderPath(r.path);
        if (fullPath == rPath || fullPath.startsWith(rPath + "/"))
            return true;
    }
    return false;
}

int SampleDatabase::findRootIdForFolder(const juce::File& folder)
{
    const auto fullPath = cleanFolderPath(folder.getFullPathName());
    auto roots = getConnectedRoots();
    for (const auto& r : roots)
    {
        const auto rPath = cleanFolderPath(r.path);
        if (fullPath == rPath || fullPath.startsWith(rPath + "/"))
            return r.id;
    }
    return -1;
}

std::vector<FolderRecord> SampleDatabase::getSubfolders(const juce::String& parentPath)
{
    std::vector<FolderRecord> results;
    const juce::ScopedLock sl(lock);
    if (db == nullptr)
        return results;

    const auto cleanP = cleanFolderPath(parentPath);
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT id, root_id, path, parent_path, name, file_count, subfolder_count FROM folders WHERE parent_path = ? ORDER BY name COLLATE NOCASE;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, cleanP.toUTF8(), -1, SQLITE_TRANSIENT);
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            FolderRecord rec;
            rec.id = sqlite3_column_int(stmt, 0);
            rec.rootId = sqlite3_column_int(stmt, 1);
            rec.path = juce::String::fromUTF8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
            rec.parentPath = juce::String::fromUTF8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)));
            rec.name = juce::String::fromUTF8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)));
            rec.fileCount = sqlite3_column_int(stmt, 5);
            rec.subfolderCount = sqlite3_column_int(stmt, 6);
            results.push_back(rec);
        }
        sqlite3_finalize(stmt);
    }
    return results;
}

std::vector<FileRecord> SampleDatabase::getFilesInFolder(const juce::String& folderPath)
{
    std::vector<FileRecord> results;
    const juce::ScopedLock sl(lock);
    if (db == nullptr)
        return results;

    const auto cleanP = cleanFolderPath(folderPath);
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT id, root_id, folder_path, file_path, name, ext, size, mod_time FROM files WHERE folder_path = ? ORDER BY name COLLATE NOCASE;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, cleanP.toUTF8(), -1, SQLITE_TRANSIENT);
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            FileRecord rec;
            rec.id = sqlite3_column_int(stmt, 0);
            rec.rootId = sqlite3_column_int(stmt, 1);
            rec.folderPath = juce::String::fromUTF8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
            rec.filePath = juce::String::fromUTF8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)));
            rec.name = juce::String::fromUTF8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)));
            rec.ext = juce::String::fromUTF8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)));
            rec.size = sqlite3_column_int64(stmt, 6);
            rec.modTime = sqlite3_column_int64(stmt, 7);
            results.push_back(rec);
        }
        sqlite3_finalize(stmt);
    }
    return results;
}

std::vector<FileRecord> SampleDatabase::getAllFilesInFolderRecursiveRecords(const juce::String& folderPath)
{
    std::vector<FileRecord> results;
    const juce::ScopedLock sl(lock);
    if (db == nullptr || folderPath.isEmpty())
        return results;

    const auto cleanP = cleanFolderPath(folderPath);
    const auto subPattern = cleanP + "/%";

    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT id, root_id, folder_path, file_path, name, ext, size, mod_time FROM files WHERE folder_path = ? OR folder_path LIKE ? ORDER BY name COLLATE NOCASE;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, cleanP.toUTF8(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, subPattern.toUTF8(), -1, SQLITE_TRANSIENT);
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            FileRecord rec;
            rec.id = sqlite3_column_int(stmt, 0);
            rec.rootId = sqlite3_column_int(stmt, 1);
            rec.folderPath = juce::String::fromUTF8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
            rec.filePath = juce::String::fromUTF8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)));
            rec.name = juce::String::fromUTF8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)));
            rec.ext = juce::String::fromUTF8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)));
            rec.size = sqlite3_column_int64(stmt, 6);
            rec.modTime = sqlite3_column_int64(stmt, 7);
            results.push_back(rec);
        }
        sqlite3_finalize(stmt);
    }
    return results;
}

juce::Array<juce::File> SampleDatabase::getAllFilesInFolderRecursive(const juce::File& folder)
{
    juce::Array<juce::File> results;
    const juce::ScopedLock sl(lock);
    if (db == nullptr)
        return results;

    const auto cleanP = cleanFolderPath(folder.getFullPathName());
    const auto subPattern = cleanP + "/%";

    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT file_path FROM files WHERE folder_path = ? OR folder_path LIKE ? ORDER BY name COLLATE NOCASE;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, cleanP.toUTF8(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, subPattern.toUTF8(), -1, SQLITE_TRANSIENT);
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            juce::String fp = juce::String::fromUTF8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
            results.add(juce::File(fp));
        }
        sqlite3_finalize(stmt);
    }
    return results;
}

juce::Array<juce::File> SampleDatabase::getFilesForPad(const juce::File& folder, bool searchSubfolders, bool searchNearFolders)
{
    if (searchSubfolders)
    {
        auto list = getAllFilesInFolderRecursive(folder);
        if (searchNearFolders)
        {
            auto parentDir = folder.getParentDirectory();
            auto siblingFolders = getSubfolders(parentDir.getFullPathName());
            for (const auto& sf : siblingFolders)
            {
                if (sf.path != folder.getFullPathName())
                {
                    auto sibFiles = getAllFilesInFolderRecursive(juce::File(sf.path));
                    list.addArray(sibFiles);
                }
            }
        }
        return list;
    }

    auto fileRecs = getFilesInFolder(folder.getFullPathName());
    juce::Array<juce::File> list;
    list.ensureStorageAllocated(static_cast<int>(fileRecs.size()));
    for (const auto& fr : fileRecs)
        list.add(juce::File(fr.filePath));

    if (searchNearFolders)
    {
        auto parentDir = folder.getParentDirectory();
        auto siblingFolders = getSubfolders(parentDir.getFullPathName());
        for (const auto& sf : siblingFolders)
        {
            if (sf.path != folder.getFullPathName())
            {
                auto sibRecs = getFilesInFolder(sf.path);
                for (const auto& sr : sibRecs)
                    list.add(juce::File(sr.filePath));
            }
        }
    }
    return list;
}

std::vector<FileRecord> SampleDatabase::searchFiles(const juce::String& query, int maxResults)
{
    std::vector<FileRecord> results;
    const juce::ScopedLock sl(lock);
    if (db == nullptr || query.trim().isEmpty())
        return results;

    const auto searchPattern = "%" + query.trim() + "%";
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT id, root_id, folder_path, file_path, name, ext, size, mod_time FROM files WHERE name LIKE ? ORDER BY name COLLATE NOCASE LIMIT ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, searchPattern.toUTF8(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, maxResults);
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            FileRecord rec;
            rec.id = sqlite3_column_int(stmt, 0);
            rec.rootId = sqlite3_column_int(stmt, 1);
            rec.folderPath = juce::String::fromUTF8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
            rec.filePath = juce::String::fromUTF8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)));
            rec.name = juce::String::fromUTF8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)));
            rec.ext = juce::String::fromUTF8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)));
            rec.size = sqlite3_column_int64(stmt, 6);
            rec.modTime = sqlite3_column_int64(stmt, 7);
            results.push_back(rec);
        }
        sqlite3_finalize(stmt);
    }
    return results;
}

bool SampleDatabase::beginTransaction()
{
    const juce::ScopedLock sl(lock);
    if (db == nullptr)
        return false;
    return (sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr) == SQLITE_OK);
}

bool SampleDatabase::commitTransaction()
{
    const juce::ScopedLock sl(lock);
    if (db == nullptr)
        return false;
    return (sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr) == SQLITE_OK);
}

bool SampleDatabase::rollbackTransaction()
{
    const juce::ScopedLock sl(lock);
    if (db == nullptr)
        return false;
    return (sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr) == SQLITE_OK);
}

void SampleDatabase::clearRootData(int rootId)
{
    const juce::ScopedLock sl(lock);
    if (db == nullptr)
        return;

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, "DELETE FROM files WHERE root_id = ?;", -1, &stmt, nullptr) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, rootId);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    if (sqlite3_prepare_v2(db, "DELETE FROM folders WHERE root_id = ?;", -1, &stmt, nullptr) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, rootId);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}

int SampleDatabase::insertFolder(int rootId, const juce::String& path, const juce::String& parentPath, const juce::String& name, int fileCount, int subfolderCount)
{
    const juce::ScopedLock sl(lock);
    if (db == nullptr)
        return -1;

    sqlite3_stmt* stmt = nullptr;
    const char* sql = "INSERT OR REPLACE INTO folders (root_id, path, parent_path, name, file_count, subfolder_count) VALUES (?, ?, ?, ?, ?, ?);";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, rootId);
        sqlite3_bind_text(stmt, 2, path.toUTF8(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, parentPath.toUTF8(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, name.toUTF8(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 5, fileCount);
        sqlite3_bind_int(stmt, 6, subfolderCount);
        if (sqlite3_step(stmt) == SQLITE_DONE)
        {
            int newId = static_cast<int>(sqlite3_last_insert_rowid(db));
            sqlite3_finalize(stmt);
            return newId;
        }
        sqlite3_finalize(stmt);
    }
    return -1;
}

bool SampleDatabase::insertFile(int rootId, const juce::String& folderPath, const juce::String& filePath, const juce::String& name, const juce::String& ext, int64_t size, int64_t modTime)
{
    const juce::ScopedLock sl(lock);
    if (db == nullptr)
        return false;

    sqlite3_stmt* stmt = nullptr;
    const char* sql = "INSERT OR REPLACE INTO files (root_id, folder_path, file_path, name, ext, size, mod_time) VALUES (?, ?, ?, ?, ?, ?, ?);";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, rootId);
        sqlite3_bind_text(stmt, 2, folderPath.toUTF8(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, filePath.toUTF8(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, name.toUTF8(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, ext.toUTF8(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 6, size);
        sqlite3_bind_int64(stmt, 7, modTime);
        bool success = (sqlite3_step(stmt) == SQLITE_DONE);
        sqlite3_finalize(stmt);
        return success;
    }
    return false;
}

void SampleDatabase::updateRootStats(int rootId, int totalFiles, int totalFolders, int64_t lastScanned)
{
    const juce::ScopedLock sl(lock);
    if (db == nullptr)
        return;

    sqlite3_stmt* stmt = nullptr;
    const char* sql = "UPDATE connected_roots SET total_files = ?, total_folders = ?, last_scanned = ? WHERE id = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, totalFiles);
        sqlite3_bind_int(stmt, 2, totalFolders);
        sqlite3_bind_int64(stmt, 3, lastScanned);
        sqlite3_bind_int(stmt, 4, rootId);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}

bool SampleDatabase::isSupportedAudioExtension(const juce::String& ext)
{
    const auto lower = ext.toLowerCase();
    return lower == ".wav" || lower == ".aif" || lower == ".aiff" ||
           lower == ".mp3" || lower == ".flac" || lower == ".ogg";
}

bool SampleDatabase::isSupportedAudioFile(const juce::File& file)
{
    return isSupportedAudioExtension(file.getFileExtension());
}

bool SampleDatabase::exportDatabase(const juce::File& destinationFile)
{
    const juce::ScopedLock sl(lock);
    if (db == nullptr)
        return false;

    // Checkpoint any pending WAL transactions to ensure the database file is fresh
    sqlite3_wal_checkpoint_v2(db, nullptr, SQLITE_CHECKPOINT_PASSIVE, nullptr, nullptr);

    destinationFile.deleteFile();
    destinationFile.getParentDirectory().createDirectory();

    sqlite3* destDb = nullptr;
    int rc = sqlite3_open(destinationFile.getFullPathName().toUTF8(), &destDb);
    if (rc != SQLITE_OK)
    {
        if (destDb != nullptr)
            sqlite3_close(destDb);
        return false;
    }

    sqlite3_backup* backup = sqlite3_backup_init(destDb, "main", db, "main");
    if (backup == nullptr)
    {
        sqlite3_close(destDb);
        return false;
    }

    rc = sqlite3_backup_step(backup, -1);
    sqlite3_backup_finish(backup);
    sqlite3_close(destDb);

    return (rc == SQLITE_DONE);
}

} // namespace sampledicer
