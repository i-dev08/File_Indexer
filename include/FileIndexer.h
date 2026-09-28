#ifndef FILE_INDEXER_H
#define FILE_INDEXER_H

#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <filesystem>

struct FileInfo {
    std::string name;
    std::string normalizedName;
    std::string path;
    std::string extension;
    std::uintmax_t size;
    std::string root;
    std::filesystem::file_time_type lastModified;
};

enum class SizeOperator {
    GREATER,
    LESS,
    GREATER_EQUAL,
    LESS_EQUAL,
    EQUAL
};

struct SizeQuery {
    SizeOperator operation;
    std::uintmax_t size;
};

struct NewFile {
    std::string path;
    std::string root;
};

struct ValidationResult {
    std::size_t validFiles = 0;
    std::vector<std::size_t> modifiedFileIndexes;
    std::vector<std::size_t>missingFileIndexes;
    std::vector<NewFile> newFiles;
};

enum class SearchField {
    NAME,
    PATH
};

class FileIndexer {
private:
    std::vector<FileInfo> files;
    std::vector<std::string> indexedRoots;
    std::unordered_map<std::string, std::vector<std::size_t>> extensionIndex;
    std::unordered_map<std::uint32_t,std::vector<std::size_t>> ngramIndex;
    std::unordered_map<std::string,std::size_t> pathIndex;

    void rebuildIndexes();
    void addFile(const std::filesystem::directory_entry& entry, const std::string& root);
    void addFile(const std::filesystem::path& filepath, const std::string& root);
    void addIndexesForFile(const FileInfo& file, std::size_t fileIndex);
    void removeFile(std::size_t fileIndex);
    void removeFileFromIndexes(const FileInfo& file, std::size_t fileIndex);
    void removeIndexFromPostingList(std::vector<std::size_t>& postingList,std::size_t fileIndex);
    std::vector<std::size_t> intersectPostingLists(const std::vector<std::size_t>& first, const std::vector<std::size_t>& second) const;
    std::vector<std::uint32_t> generateNGrams(const std::string& text) const;
    std::string lowerCase(const std::string& s) const;
    std::uintmax_t parseSize(const std::string& input) const;
    SizeQuery parseSizeQuery(const std::string& input) const;
    std::vector<FileInfo> searchByField(const std::string& query, SearchField field) const;
    
public:
    ValidationResult validateIndex() const;
    void updateFileMetaData(std::size_t fileIndex);
    void updateIndex();
    void scan(const std::string& directoryPath);
    void saveIndex(const std::string& filePath) const;
    void loadIndex(const std::string& filePath);
    const std::vector<FileInfo>& getFiles() const;
    const std::vector<std::string>& getIndexedRoots() const;
    std::vector<FileInfo> searchByNameSubstring(const std::string& query) const;
    std::vector<FileInfo> searchByPath(const std::string& query) const;
    std::vector<FileInfo> searchByExtension(const std::string& extension) const;
    std::vector<FileInfo> searchBySize(std::uintmax_t size, SizeOperator operation) const;
    std::vector<FileInfo> searchBySizeQuery(const std::string& query) const;
};

#endif