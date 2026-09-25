#include <iostream>
#include <filesystem>

#include "FileIndexer.h"

namespace fs = std::filesystem;

void displayResults(const std::vector<FileInfo>& results) {
    std::cout << "\nResults: " << results.size() << "\n\n";

    for (const auto& file : results) {
        std::cout << "Name: " << file.name << '\n';
        std::cout << "Path: " << file.path << '\n';
        std::cout << "Extension: " << file.extension << '\n';
        std::cout << "Size: " << file.size << '\n';
        std::cout << "-------------------------------------------\n";
    }
}

int main() {
    std::string directoryPath;

    std::cout << "Enter directory path: ";
    std::getline(std::cin,directoryPath);

    if (!fs::exists(directoryPath)) {
        std::cout << "Directory does not exist.\n";
        return 1;
    }

    if (!fs::is_directory(directoryPath)) {
        std::cout << "That path is not a directory.\n";
        return 1;
    }

    FileIndexer indexer;

    indexer.scan(directoryPath);

    const auto& files = indexer.getFiles();

    std::cout << "\nIndexed " << files.size() << " files.\n\n";

    while (true) {
        std::cout << "\n==============================\n";
        std::cout << "       C++ FILE INDEXER\n";
        std::cout << "==============================\n";

        std::cout << "1. Search by filename\n";
        std::cout << "2. Search by extension\n";
        std::cout << "3. Search by size\n";
        std::cout << "4. Exit\n";

        std::cout << "\nChoose an option: ";
        int choice;
        std::cin >> choice;

        std::cin.ignore();

        if (choice == 4) return 0;

        if (choice == 1) {
            std::string query;
            std::cout << "Search filename: ";
            std::getline(std::cin,query);
            std::vector<FileInfo> results = indexer.search(query);

            displayResults(results);
        }

        else if (choice == 2) {
            std::string query;
            std::cout << "Enter Exntension: ";
            std::getline(std::cin,query);
            std::vector<FileInfo> results = indexer.searchByExtension(query);

            displayResults(results);
        }

        else if (choice == 3) {
            std::uintmax_t query;
            std::cout << "ENter minimun size in bytes: ";
            std::cin >> query;

            std::cin.ignore();

            std::vector<FileInfo> results = indexer.searchByMinSize(query);

            displayResults(results);
        }
    }
}