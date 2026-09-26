#ifndef FILE_INDEXER_H
#define FILE_INDEXER_H

#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>

struct FileInfo {
    std::string name;
    std::string path;
    std::string extension;
    std::uintmax_t size;
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

enum class SearchField {
    NAME,
    PATH
};

class FileIndexer {
private:
    std::vector<FileInfo> files;
    std::unordered_map<std::string, std::vector<std::size_t>> extensionIndex;
    std::unordered_map<std::string, std::vector<std::size_t>> nameIndex;
    std::unordered_map<std::string,std::vector<std::size_t>> ngramIndex;

    std::vector<std::string> generateNGrams(const std::string& text) const;
    std::string lowerCase(const std::string& s) const;
    std::uintmax_t parseSize(const std::string& input) const;
    SizeQuery parseSizeQuery(const std::string& input) const;
    std::vector<FileInfo> searchByField(const std::string& query, SearchField field) const;
    
public:
    void scan(const std::string& directoryPath);

    const std::vector<FileInfo>& getFiles() const;

    std::vector<FileInfo> searchByNameExact(const std::string& query) const;

    std::vector<FileInfo> searchByNameSubstring(const std::string& query) const;

    std::vector<FileInfo> searchByPath(const std::string& query) const;

    std::vector<FileInfo> searchByExtension(const std::string& extension) const;

    std::vector<FileInfo> searchBySize(std::uintmax_t size, SizeOperator operation) const;

    std::vector<FileInfo> searchBySizeQuery(const std::string& query) const;
};

#endif