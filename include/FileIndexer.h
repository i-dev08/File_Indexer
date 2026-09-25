#ifndef FILE_INDEXER_H
#define FILE_INDEXER_H

#include <string>
#include <vector>
#include <cstdint>

struct FileInfo {
    std::string name;
    std::string path;
    std::string extension;
    std::uintmax_t size;
    std::string searchableName;
    std::string searchableExtension;
};

class FileIndexer {
private:
    std::vector<FileInfo> files;
    std::string lowerCase(const std::string& s) const;

public:
    void scan(const std::string& directoryPath);

    const std::vector<FileInfo>& getFiles() const;

    std::vector<FileInfo> search(const std::string& query) const;

    std::vector<FileInfo> searchByExtension(const std::string& extension) const;

    std::vector<FileInfo> searchByMinSize(std::uintmax_t minSize) const;
};

#endif