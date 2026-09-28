#include <iostream>
#include <filesystem>
#include <stdexcept>
#include <fstream>

#include "FileIndexer.h"

namespace fs = std::filesystem;

const std::string INDEX_FILE = "data/index.dat";

int main() {
    FileIndexer indexer;

    if (fs::exists(INDEX_FILE)) {
        try {
            indexer.loadIndex(INDEX_FILE);
            std::cout << "\nSaved index loaded successfully.\n";
            std::cout << "Indexed folders: " << indexer.getIndexedRoots().size() << '\n';
            std::cout << "Indexed files: "<<indexer.getFiles().size() << '\n';
        }
        catch (const std::exception& e) {
            std::cout << "\nCould not load saved index.\n";
            std::cout << "Reason: "<< e.what() << '\n';
            std::cout << "Starting with an empty index.\n";
        }
    }

    while (indexer.getIndexedRoots().empty()) {
        std::cout << "\n================================\n";
        std::cout << "       C++ FILE INDEXER\n";
        std::cout << "================================\n";

        std::cout << "\nNo indexed folders found.\n\n";

        std::cout << "1. Add folder\n";
        std::cout << "2. Exit\n";

        std::cout << "\nChoose an option: ";

        int choice;
        std::cin >> choice;
        std::cin.ignore();

        if (choice==2) return 0;

        if (choice == 1) {
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

            try {
                indexer.scan(directoryPath);
                indexer.saveIndex(INDEX_FILE);

                std::cout << "\nFolder indexed successfully.\n";
                std::cout << "Indexed " << indexer.getFiles().size() << " files\n";
                std::cout <<"Index Saved\n";
            }
            catch (const fs::filesystem_error& e) {
                std::cout << "Error while scanning folder: " << e.what() << '\n';
            }
        }else std::cout << "\nInvalid option.\n";
    }


    ValidationResult validation = indexer.validateIndex();

    std::cout << "\nIndex validation:\n";
    std::cout << "Valid files: "<< validation.validFiles << '\n';

    std::cout << "Modified files: "<< validation.modifiedFileIndexes.size() << '\n';

    std::cout << "Missing files: "<< validation.missingFileIndexes.size() << '\n';

    std::cout << "New files: " << validation.newFiles.size() << '\n';

    std::cout << "" << validation.newFiles.size() << " new files detected\n";

    std::cout << "Would you like to update the indexes (y/n)?: ";
    char updatechoice;
    std::cin >> updatechoice;
    std::cin.ignore();

    if (updatechoice == 'y' || updatechoice == 'Y') {
        indexer.updateIndex();
        indexer.saveIndex(INDEX_FILE);
    }

    while (true) {

        std::cout << "\n================================\n";
        std::cout << "       C++ FILE INDEXER\n";
        std::cout << "================================\n";

        std::cout << "Indexed "
                  << indexer.getIndexedRoots().size()
                  << " Folders\n";

        std::cout << "Indexed "
                  << indexer.getFiles().size()
                  << " Files\n\n";

        std::cout << "1. Add folder to index\n";
        std::cout << "2. Search by filename\n";
        std::cout << "3. Search by path\n";
        std::cout << "4. Search by extension\n";
        std::cout << "5. Search by size\n";
        std::cout << "6. Show indexed folders\n";
        std::cout << "7. Exit\n";

        std::cout << "\nChoose an option: ";
        int choice;
        std::cin >> choice;

        std::cin.ignore();

        if (choice == 7) return 0;

        if (choice == 1) {
            std::string directoryPath;

            std::cout << "Enter directory path: ";
            std::getline(std::cin,directoryPath);

            if (!fs::exists(directoryPath)) {
                std::cout << "Directory does not exist.\n";
                continue;
            }

            if (!fs::is_directory(directoryPath)) {
                std::cout << "that path is not a directory.\n";
                continue;
            }

            try {
                std::size_t oldFileCount = indexer.getFiles().size();

                indexer.scan(directoryPath);

                std::size_t newFileCount = indexer.getFiles().size();

                if (newFileCount == oldFileCount) std::cout << "\nFolder was already indexed.\n";
                else {
                    std::cout << "\nFolder indexed successfully";
                    std::cout << "Added " << newFileCount - oldFileCount << '\n';
                }
                std::cout << "Total " << newFileCount << " files indexed.\n";

                indexer.saveIndex(INDEX_FILE);
                std::cout << "Index saved\n";
            }
            catch (const fs::filesystem_error& e) {
                std::cout << "Error while scanning folder: " << e.what() << '\n';
            }
        }

        if (choice == 2) {
            std::string query;
            std::cout << "Search filename: ";
            std::getline(std::cin,query);
            auto results = indexer.searchByNameSubstring(query);
            indexer.displayResults(results);
        }

        else if (choice == 3) {
            std::string query;
            std::cout << "Search path: ";
            std::getline(std::cin,query);
            std::vector<std::size_t> results = indexer.searchByPath(query);

            indexer.displayResults(results);
        }

        else if (choice == 4) {
            std::string query;
            std::cout << "Enter Exntension: ";
            std::getline(std::cin,query);
            std::vector<std::size_t> results = indexer.searchByExtension(query);

            indexer.displayResults(results);
        }

       else if (choice == 5) {

            std::string query;

            std::cout << "Enter size constraint: ";
            std::getline(std::cin,query);

            try {
                std::vector<std::size_t> results = indexer.searchBySizeQuery(query);

                indexer.displayResults(results);
            }
            catch (const std::invalid_argument& e) {
                std::cout << "Invalid query: " << e.what() << '\n';
            }
        }

        else if (choice == 6) {
            const auto& roots = indexer.getIndexedRoots();
            std::cout << "\nIndexed folders:\n\n";
            for (std::size_t i =0; i<roots.size(); i++) {
                std::cout << i+1 << ") " << roots[i] << '\n';
            }
        }
    }
}