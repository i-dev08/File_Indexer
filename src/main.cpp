#include <iostream>
#include <filesystem>
#include <stdexcept>

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
        std::cout << "2. Search by path\n";
        std::cout << "3. Search by extension\n";
        std::cout << "4. Search by size\n";
        std::cout << "5. Exit\n";

        std::cout << "\nChoose an option: ";
        int choice;
        std::cin >> choice;

        std::cin.ignore();

        if (choice == 5) return 0;

        if (choice == 1) {
            std::string query;
            std::cout << "Search filename: ";
            std::getline(std::cin,query);

            auto exactResults = indexer.searchByNameExact(query);

            if (!exactResults.empty()) {
                displayResults(exactResults);

                char answer;

                std::cout << "\nSearch for filenames containing \"" << query << "\" as a sunstring? (y/n): ";
                std::cin >> answer;

                if (answer == 'y' || answer == 'Y') {
                    auto substringResults = indexer.searchByNameSubstring(query);
                    displayResults(substringResults);
                }
            }
            else {
                std::cout << "\nNo exact filename match found";
                auto substringResults = indexer.searchByNameSubstring(query);
                displayResults(substringResults);
            }
        }

        else if (choice == 2) {
            std::string query;
            std::cout << "Search path: ";
            std::getline(std::cin,query);
            std::vector<FileInfo> results = indexer.searchByPath(query);

            displayResults(results);
        }

        else if (choice == 3) {
            std::string query;
            std::cout << "Enter Exntension: ";
            std::getline(std::cin,query);
            std::vector<FileInfo> results = indexer.searchByExtension(query);

            displayResults(results);
        }

       else if (choice == 4) {

            std::string query;

            std::cout << "Enter size in bytes: ";
            std::getline(std::cin,query);

            try {
                std::vector<FileInfo> results = indexer.searchBySizeQuery(query);

                displayResults(results);
            }
            catch (const std::invalid_argument& e) {
                std::cout << "Invalid query: " << e.what() << '\n';
            }
        }
    }
}