#include "FileIndexer.h"

#include <filesystem>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>
#include <unordered_set>

namespace fs = std::filesystem;

std::vector<std::string> FileIndexer::generateNGrams(const std::string& query) const {
    std::vector<std::string> grams;

    if (query.length() < 3) return grams;

    for (std::size_t i=0;i<=query.length()-3;i++) {
        grams.push_back(query.substr(i,3));
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




void FileIndexer::scan(const std::string& directoryPath) {

    files.clear();
    extensionIndex.clear();
    nameIndex.clear();
    ngramIndex.clear();

    for (const auto& entry : fs::recursive_directory_iterator(directoryPath)) {

        if (fs::is_regular_file(entry)) {

            FileInfo file;

            file.name = entry.path().filename().string();
            file.path = entry.path().string();
            file.extension = entry.path().extension().string();
            file.size = fs::file_size(entry);

            files.push_back(file);

            std::size_t fileIndex = files.size() - 1;
            std::string normalizedName = lowerCase(file.name);
            extensionIndex[lowerCase(file.extension)].push_back(fileIndex);
            nameIndex[normalizedName].push_back(fileIndex);

            std::vector<std::string> grams = generateNGrams(normalizedName);

            for (const auto& gram : grams) {
                ngramIndex[gram].push_back(fileIndex);
            }

        }
    }
}

std::vector<FileInfo> FileIndexer::searchByNameExact(const std::string& query) const {
    std::vector<FileInfo> results;

    std::string lowerQuery = lowerCase(query);

    auto it = nameIndex.find(lowerQuery);

    if (it == nameIndex.end()) return results;

    for (std::size_t index : it->second) results.push_back(files[index]);

    return results;
}

const std::vector<FileInfo>& FileIndexer::getFiles() const {
    return files;
}

std::vector<FileInfo> FileIndexer::searchByNameSubstring(const std::string& query) const {
    if (query.length() < 3) {
        return searchByField(query, SearchField::NAME);
    }

    std::string lowerQuery = lowerCase(query);

    std::vector<std::string> grams = generateNGrams(lowerQuery);

    std::unordered_set<std::size_t> candidates;

    auto firstIt = ngramIndex.find(grams[0]);

    if (firstIt == ngramIndex.end())return {};

    for (std::size_t index : firstIt->second) candidates.insert(index);

    for (std::size_t i =1; i < grams.size(); i++) {
        auto it = ngramIndex.find(grams[i]);

        if (it == ngramIndex.end()) return {};

        std::unordered_set<std::size_t> currentCandidates;

        for (std::size_t index : it->second) currentCandidates.insert(index);
        for (auto candidateIt = candidates.begin(); candidateIt != candidates.end();) {
            if (currentCandidates.find(*candidateIt) == currentCandidates.end()) candidateIt = candidates.erase(candidateIt);
            else ++candidateIt;
        }
    }

    if (candidates.empty()) return {};

    std::vector<FileInfo> results;

    for (std::size_t index : candidates) {
        std::string filename = lowerCase(files[index].name);

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