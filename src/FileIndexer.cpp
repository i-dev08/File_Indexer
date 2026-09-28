#include "FileIndexer.h"

#include <filesystem>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>
#include <unordered_set>
#include <fstream>
#include <limits>

namespace {
    void writeString(std::ofstream& out, const std::string& value) {
        std::uint64_t length = value.size();

        out.write(reinterpret_cast<const char*>(&length),sizeof(length));
        out.write(value.data(),static_cast<std::streamsize>(length));
    }

    std::int64_t serializeTime(const std::filesystem::file_time_type& time) {
        return time.time_since_epoch().count();
    }

    std::filesystem::file_time_type deserializeTime(std::int64_t value) {
        return std::filesystem::file_time_type(std::filesystem::file_time_type::duration(value));
    }

    std::string readString(std::ifstream& in) {
        std:;uint64_t length;

        in.read(reinterpret_cast<char*>(&length),sizeof(length));

        if(!in) throw std::runtime_error("Unexpected end of index file.");

        std::string value(length,'\0');

        in.read(value.data(),static_cast<std::streamsize>(length));

        if (!in) throw std::runtime_error("Unexpected end of index file.");

        return value;
    }
}

namespace fs = std::filesystem;

void FileIndexer::rebuildIndexes() {
    extensionIndex.clear();
    ngramIndex.clear();
    pathIndex.clear();

    for (std::size_t fileIndex = 0; fileIndex < files.size(); fileIndex++) {
        const auto& file = files[fileIndex];
        pathIndex[file.path] = fileIndex;

        extensionIndex[lowerCase(file.extension)].push_back(fileIndex);

        std::vector<std::uint32_t> grams = generateNGrams(file.normalizedName);

        for (const auto& gram : grams) ngramIndex[gram].push_back(fileIndex);
    }
}

void FileIndexer::addFile(const std::filesystem::directory_entry& entry, const std::string& root) {
    FileInfo file;

    file.name = entry.path().filename().string();
    file.normalizedName = lowerCase(file.name);
    file.path = entry.path().string();
    file.extension = entry.path().extension().string();
    file.size = fs::file_size(entry);
    file.root = root;
    file.lastModified = fs::last_write_time(entry);
    std:;size_t fileIndex = files.size();
    files.push_back(file);
    addIndexesForFile(file,fileIndex);
}

void FileIndexer::addFile(const fs::path& filePath, const std::string& root) {
    fs::directory_entry entry(filePath);
    addFile(entry,root);
}

void FileIndexer::addIndexesForFile(const FileInfo& file, std::size_t fileIndex) {
    pathIndex[file.path] = fileIndex;
    extensionIndex[lowerCase(file.extension)].push_back(fileIndex);
    std::vector<std::uint32_t> grams = generateNGrams(file.normalizedName);

    for (const auto& gram : grams) {
        ngramIndex[gram].push_back(fileIndex);
    }
}

void FileIndexer::removeFile(std::size_t fileIndex) {
    if (fileIndex >= files.size()) return;

    std::size_t lastIndex = files.size() - 1;
    removeFileFromIndexes(files[fileIndex],fileIndex);

    if (fileIndex == lastIndex) {
        files.pop_back();
        return;
    }

    removeFileFromIndexes(files[lastIndex],lastIndex);

    FileInfo movedFile = files[lastIndex];

    files.pop_back();

    files[fileIndex] = movedFile;

    addIndexesForFile(files[fileIndex],fileIndex);
}

void FileIndexer::removeIndexFromPostingList(std::vector<std::size_t>& postingList, std::size_t fileIndex) {
    auto it = std::find(postingList.begin(), postingList.end(),fileIndex);

    if (it != postingList.end()) postingList.erase(it);
}

void FileIndexer::removeFileFromIndexes(const FileInfo& file, std::size_t fileIndex) {
    pathIndex.erase(file.path);

    std::string normalizedExtension(lowerCase(file.extension));
    auto extensionIt = extensionIndex.find(normalizedExtension);

    if (extensionIt != extensionIndex.end()) {
        removeIndexFromPostingList(extensionIt->second,fileIndex);
        if (extensionIt->second.empty()) extensionIndex.erase(extensionIt);
    }

    std::vector<std::uint32_t> grams = generateNGrams(file.normalizedName);

    for (const auto& gram : grams) {
        auto gramIt = ngramIndex.find(gram);
        if (gramIt != ngramIndex.end()) {
            removeIndexFromPostingList(gramIt->second,fileIndex);
            if (gramIt->second.empty()) ngramIndex.erase(gramIt);
        }
    }
}

std::vector<std::size_t> FileIndexer::intersectPostingLists(const std::vector<std::size_t>& first, const std::vector<std::size_t>& second) const {
    std::vector<std::size_t> results;

    std::size_t i = 0;
    std::size_t j = 0;

    while (i < first.size() && j < second.size()) {
        if (first[i] < second[j]) i++;
        else if (first[i] > second[j]) j++;
        else {
            results.push_back(first[i]);
            i++;
            j++;
        }
    }
    return results;
}

std::vector<std::uint32_t> FileIndexer::generateNGrams(const std::string& query) const {

    std::vector<std::uint32_t> grams;

    if (query.length() < 3) {
        return grams;
    }

    for (std::size_t i = 0;
         i <= query.length() - 3;
         ++i) {

        std::uint32_t gram =
            (static_cast<std::uint32_t>(
                static_cast<unsigned char>(query[i])
            ) << 16)
            |
            (static_cast<std::uint32_t>(
                static_cast<unsigned char>(query[i + 1])
            ) << 8)
            |
            static_cast<std::uint32_t>(
                static_cast<unsigned char>(query[i + 2])
            );

        grams.push_back(gram);
    }

    return grams;
}

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

std::uintmax_t FileIndexer::parseSize(const std::string& input) const {
    std::stringstream ss(input);

    double number;
    std::string unit;

    ss >> number >> unit;

    unit = lowerCase(unit);

    if (unit == "b") return static_cast<std::uintmax_t>(number);
    if (unit == "kb") return static_cast<std::uintmax_t>(number*1024);
    if (unit == "mb") return static_cast<std::uintmax_t>(number*1024*1024);
    if (unit == "gb") return static_cast<std::uintmax_t>(number*1024*1024*1024);

    throw std::invalid_argument("Unknown size unit");
}

SizeQuery FileIndexer::parseSizeQuery(const std::string& input) const {

    SizeQuery query;

    std::size_t operatorLength = 0;

    if (input.size() >= 2 && input.substr(0, 2) == ">=") {
        query.operation = SizeOperator::GREATER_EQUAL;
        operatorLength = 2;
    }
    else if (input.size() >= 2 && input.substr(0, 2) == "<=") {
        query.operation = SizeOperator::LESS_EQUAL;
        operatorLength = 2;
    }
    else if (!input.empty() && input[0] == '>') {
        query.operation = SizeOperator::GREATER;
        operatorLength = 1;
    }
    else if (!input.empty() && input[0] == '<') {
        query.operation = SizeOperator::LESS;
        operatorLength = 1;
    }
    else if (!input.empty() && input[0] == '=') {
        query.operation = SizeOperator::EQUAL;
        operatorLength = 1;
    }
    else {
        throw std::invalid_argument(
            "Invalid size operator"
        );
    }

    std::string sizePart = input.substr(operatorLength);

    query.size = parseSize(sizePart);

    return query;
}

std::vector<FileInfo> FileIndexer::searchByField(const std::string& query, SearchField field) const {
    std::vector<FileInfo> results;

    std::string lowerQuery = lowerCase(query);

    for (const auto& file : files) {
        std::string value;

        switch (field) {
            case SearchField::NAME:
                value = lowerCase(file.name);
                break;

            case SearchField::PATH:
                value = lowerCase(file.path);
                break;
        }

        if (value.find(lowerQuery) != std::string::npos) results.push_back(file);
    }

    return results;
}




ValidationResult FileIndexer::validateIndex() const {

    ValidationResult result;

    for (std::size_t i =0; i< files.size(); i++) {

        const auto& file = files[i];
        fs::path filePath(file.path);

        if (!fs::exists(filePath)) {
            result.missingFileIndexes.push_back(i);
            continue;
        }

        try {
            std::uintmax_t currentSize = fs::file_size(filePath);

            auto currentModified = fs::last_write_time(filePath);

            if (currentSize == file.size && currentModified == file.lastModified) result.validFiles++;
            else result.modifiedFileIndexes.push_back(i);
        }
        catch (const fs::filesystem_error&) { 
            result.missingFileIndexes.push_back(i);
        }
    }

    for (const auto& root : indexedRoots) {
        if (!fs::exists(root) || !fs::is_directory(root)) continue;

        for (const auto& entry : fs::recursive_directory_iterator(root)) {
            if (!fs::is_regular_file(entry)) continue;

            std::string path = entry.path().string();

            if (pathIndex.find(path) == pathIndex.end()) result.newFiles.push_back({path,root});
        }
    }

    return result;
}

void FileIndexer::updateFileMetaData(std::size_t fileIndex) {
    if (fileIndex >= files.size()) return;
    std::filesystem::path filePath = files[fileIndex].path;
    files[fileIndex].size = fs::file_size(filePath);
    files[fileIndex].lastModified = fs::last_write_time(filePath);
}

void FileIndexer::updateIndex() {
    ValidationResult validation = validateIndex();

    for (std::size_t fileIndex : validation.modifiedFileIndexes) updateFileMetaData(fileIndex);

    std::sort(validation.missingFileIndexes.rbegin(),validation.missingFileIndexes.rend());

    for (std::size_t fileIndex : validation.missingFileIndexes) removeFile(fileIndex);

    for (const auto& newFile : validation.newFiles) {
        addFile(std::filesystem::path(newFile.path),newFile.root);
    }
}

void FileIndexer::scan(const std::string& directoryPath) {

    if (std::find(indexedRoots.begin(), indexedRoots.end(), directoryPath) != indexedRoots.end()) return;

    indexedRoots.push_back(directoryPath);

    for (const auto& entry : fs::recursive_directory_iterator(directoryPath)) {

        if (fs::is_regular_file(entry)) {
            addFile(entry,directoryPath);
        }
    }
}

void FileIndexer::saveIndex(const std::string& filePath) const {
    std::ofstream out(filePath,std::ios::binary);

    if (!out) throw std::runtime_error("Could not open index file for writing.");

    const char magic[] = "CFIDX";

    out.write(magic,sizeof(magic));

    std::uint32_t version = 1;

    out.write(reinterpret_cast<const char*>(&version),sizeof(version));

    std::uint64_t rootCount = indexedRoots.size();

    out.write(reinterpret_cast<const char*>(&rootCount),sizeof(rootCount));

    for (const auto& root : indexedRoots) writeString(out,root);

    std::uint64_t fileCount = files.size();

    out.write(reinterpret_cast<const char*>(&fileCount),sizeof(fileCount));

    for (const auto& file : files) {
        writeString(out,file.name);
        writeString(out,file.normalizedName);
        writeString(out,file.path);
        writeString(out,file.extension);
        out.write(reinterpret_cast<const char*>(&file.size),sizeof(file.size));
        std::int64_t lastModified = serializeTime(file.lastModified);
        out.write(reinterpret_cast<const char*>(&lastModified),sizeof(lastModified));
        writeString(out,file.root);
    }
    
    if(!out) throw std::runtime_error("Failed while writing index file.");
}

void FileIndexer::loadIndex(const std::string& filePath) {
    std::ifstream in(filePath,std::ios::binary);

    if (!in) throw std::runtime_error("Could not open index file.");

    char magic[6] = {};

    in.read(magic,sizeof(magic));

    if (!in || std::string(magic) != "CFIDX") throw std::runtime_error("Invalid index file.");

    std::uint32_t version;

    in.read(reinterpret_cast<char*>(&version),sizeof(version));

    if (!in) throw std::runtime_error("Invalid index file.");

    if (version != 1) throw std::runtime_error("Unsupported index file version");

    std::uint64_t rootCount;

    in.read(reinterpret_cast<char*>(&rootCount),sizeof(rootCount));

    if (!in) throw std::runtime_error("Invalid index file.");
    
    indexedRoots.clear();

    for (std::uint64_t i =0;i<rootCount; i++) indexedRoots.push_back(readString(in));

    std::uint64_t fileCount;

    in.read(reinterpret_cast<char*>(&fileCount),sizeof(fileCount));

    if(!in) throw std::runtime_error("Invalid index file.");

    files.clear();

    for (std::uint64_t i =0;i<fileCount;i++) {
        FileInfo file;

        file.name = readString(in);
        file.normalizedName = readString(in);
        file.path = readString(in);
        file.extension = readString(in);
        in.read(reinterpret_cast<char*>(&file.size),sizeof(file.size));
        if (!in) throw std::runtime_error("Invalid index file.");
        std::int64_t lastModified;
        in.read(reinterpret_cast<char*>(&lastModified),sizeof(lastModified));
        if (!in) throw std::runtime_error("Invalid index file.");
        file.lastModified = deserializeTime(lastModified);
        file.root = readString(in);
        files.push_back(file);
    }
    rebuildIndexes();
}

const std::vector<FileInfo>& FileIndexer::getFiles() const {
    return files;
}

const std::vector<std::string>& FileIndexer::getIndexedRoots() const {
    return indexedRoots;
}

std::vector<FileInfo> FileIndexer::searchByNameSubstring(const std::string& query) const {

    if (query.length() < 3) {
        return searchByField(query,SearchField::NAME);
    }

    std::string lowerQuery = lowerCase(query);

    std::vector<std::uint32_t> grams = generateNGrams(lowerQuery);

    std::size_t smallestGramIndex = 0;
    std::size_t smallestPostingSize = std::numeric_limits<std::size_t>::max();

    for (std::size_t i =0; i < grams.size(); i++) {
        auto it = ngramIndex.find(grams[i]);

        if (it == ngramIndex.end()) return {};

        if (it->second.size() < smallestPostingSize) {
            smallestPostingSize = it->second.size();
            smallestGramIndex = i;
        }
    }

    std::vector<std::size_t> candidates = ngramIndex.at(grams[smallestGramIndex]);

    for (std::size_t i =0; i< grams.size(); i++) {
        if (i == smallestGramIndex) continue;

        const auto& postingList = ngramIndex.at(grams[i]);

        candidates = intersectPostingLists(candidates,postingList);

        if (candidates.empty()) return {};
    }

    std::vector<FileInfo> results;

    for (std::size_t index : candidates) {
        std::string filename = files[index].normalizedName;
        if (filename.find(lowerQuery) != std::string::npos) results.push_back(files[index]);
    }
    return results;
}

std::vector<FileInfo> FileIndexer::searchByPath(const std::string& query) const {
    return searchByField(query,SearchField::PATH);
}

std::vector<FileInfo> FileIndexer::searchByExtension(const std::string& query) const {
    std::vector<FileInfo> results;

    std::string lowerExtension = lowerCase(query);

    if (!lowerExtension.empty() && lowerExtension[0] != '.') {
        lowerExtension = "." + lowerExtension;
    }

    auto it = extensionIndex.find(lowerExtension);

    if (it == extensionIndex.end()) {
        return results;
    }

    for (std::size_t index : it->second) {
        results.push_back(files[index]);
    }

    return results;
}

std::vector<FileInfo> FileIndexer::searchBySize(std::uintmax_t size,SizeOperator operation) const {
    std::vector<FileInfo> results;

    for (const auto&file : files) {
        bool matches = false;

        switch(operation) {
            case SizeOperator::GREATER:
                matches = file.size > size;
                break;
            
            case SizeOperator::LESS:
                matches = file.size < size;
                break;

            case SizeOperator::GREATER_EQUAL:
                matches = file.size >= size;
                break;

            case SizeOperator::LESS_EQUAL:
                matches = file.size <= size;
                break;

            case SizeOperator::EQUAL:
                matches = file.size == size;
                break;
        }
        if (matches) {
            results.push_back(file);
        }
    }
    return results;
}

std::vector<FileInfo> FileIndexer::searchBySizeQuery(const std::string& query) const {
    SizeQuery parsedQuery = parseSizeQuery(query);

    return searchBySize(parsedQuery.size, parsedQuery.operation);
}