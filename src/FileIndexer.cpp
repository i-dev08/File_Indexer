#include "FileIndexer.h"

#include <filesystem>
#include <algorithm>
#include <cctype>

namespace fs = std::filesystem;

std::string FileIndexer::lowerCase(const std::string& s) const {
    std::string res = s;
    std::transform(
        res.begin(),
        res.end(),
        res.begin(),
        [](unsigned char c) {
            return std::tolower(c);
        }
    );
    return res;
}

void FileIndexer::scan(const std::string& directoryPath) {

    files.clear();

    for (const auto& entry : fs::recursive_directory_iterator(directoryPath)) {

        if (fs::is_regular_file(entry)) {

            FileInfo file;

            file.name = entry.path().filename().string();
            file.path = entry.path().string();
            file.extension = entry.path().extension().string();
            file.size = fs::file_size(entry);
            file.searchableName = lowerCase(file.name);
            file.searchableExtension = lowerCase(file.extension);

            files.push_back(file);
        }
    }
}

const std::vector<FileInfo>& FileIndexer::getFiles() const {
    return files;
}

std::vector<FileInfo> FileIndexer::search(const std::string& query) const {
    std::vector<FileInfo> results;

    std::string lowerQuery = lowerCase(query);

    for (const auto& file : files) {

        if (file.searchableName.find(lowerQuery) != std::string::npos) {
            results.push_back(file);
        }
    }

    return results;
}

std::vector<FileInfo> FileIndexer::searchByExtension(const std::string& query) const {
    std::vector<FileInfo> results;

    std::string lowerExtension = lowerCase(query);

    if (!lowerExtension.empty() && lowerExtension[0] != '.') {
        lowerExtension = "." + lowerExtension;
    }

    for (const auto& file : files) {
        if (file.searchableExtension == lowerExtension) {
            results.push_back(file);
        }
    }
    return results;
}

std::vector<FileInfo> FileIndexer::searchByMinSize(std::uintmax_t minSize) const {
    std::vector<FileInfo> results;

    for (const auto& file : files) {
        if (file.size >= minSize) {
            results.push_back(file);
        }
    }
    return results;
}