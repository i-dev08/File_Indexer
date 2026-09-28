#include "FileIndexer.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <chrono>
#include <vector>
#include <windows.h>
#include <psapi.h>

namespace fs = std::filesystem;
using Clock = std::chrono::steady_clock;

void generateBenchmarkDataset(const fs::path& benchmarkDirectory, int fileCount) {
    if (fs::exists(benchmarkDirectory)) {
        fs::remove_all(benchmarkDirectory);
    }

    const std::vector<fs::path> folders = {
        "Documents/Reports/2024",
        "Documents/Reports/2025",
        "Documents/Reports/2026",

        "Documents/Projects/Game",
        "Documents/Projects/Website",
        "Documents/Projects/Mobile",

        "Documents/Notes",

        "Downloads/Software",
        "Downloads/Archives",
        "Downloads/Misc",

        "Pictures/Personal",
        "Pictures/College",
        "Pictures/Projects",

        "College/DSA",
        "College/DBMS",
        "College/OS",

        "College/Projects",

        "College/Projects/Game"
    };

    for (const auto& folder : folders) {
        fs::create_directories(benchmarkDirectory/folder);
    }

    std::cout << "Generating " << fileCount << " benchmark files...\n";

    for (int i=0; i< fileCount; i++) {
        std::string filename;
        std::string extension;
        if (i<30000) {
            filename = "report_"+std::to_string(i)+".txt";
        }else if (i<42500) {
            filename = "project_"+std::to_string(i)+".cpp";
        }else if (i<47500) {
            filename = "algorithm_"+std::to_string(i)+".md";
        }else {
            filename = "client_"+std::to_string(i)+"_data.txt";
        }

        const fs::path& folder = folders[i % folders.size()];

        fs::path filePath = benchmarkDirectory / folder / filename;
        std::ofstream file(filePath);
        file << "benchmark test file\n";
    }
    std::cout << "dataset generated\n";
}

FileIndexer buildBenchmarkIndex(const fs::path& benchmarkDirectory) {
    FileIndexer indexer;
    std::cout << "indexing dataset...\n";
    indexer.scan(benchmarkDirectory.string());
    std::cout << "Files indexed\n";
    return indexer;
}

void runBenchmark(const FileIndexer& indexer, const std::vector<std::string>& queries, int iterations) {
    for (const auto& query : queries) indexer.searchByNameSubstring(query);

    std::cout << "\nRunning benchmark...\n";
    std::cout << "iterations per query: " << iterations << "\n\n";

    for (const auto& query : queries) {
        std::size_t totalMatches = 0;
        auto start = Clock::now();

        for (int i=0;i<iterations;i++) {
            auto results = indexer.searchByNameSubstring(query);
            totalMatches +=results.size();
        }

        auto end = Clock::now();

        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end-start);
        double average = static_cast<double>(elapsed.count()) / iterations;

        std::cout << "Query: "<< query<< '\n';
        std::cout << "Average: " << average << " us\n";
        std::cout << "Matches " << totalMatches /iterations << "\n\n";
    }
}

std::size_t getMemoryUsage() {
    PROCESS_MEMORY_COUNTERS memInfo;

    if (GetProcessMemoryInfo(GetCurrentProcess(),&memInfo,sizeof(memInfo))) return memInfo.WorkingSetSize;
    
    return 0;
}
int main() {
    const fs::path benchmarkDirectory = "data/benchmark_data";
    const int fileCount = 50000;
    const int iterations = 1000;

    const std::vector<std::string> queries = {
        "report",
        "project",
        "algorithm",
        "xyz987"
    };
    if (!fs::exists(benchmarkDirectory)) {
        generateBenchmarkDataset(benchmarkDirectory, fileCount);
    }
    else std::cout << "Using existing benchmark datasets\n";
    std::size_t memoryBefore = getMemoryUsage();

    FileIndexer indexer = buildBenchmarkIndex(benchmarkDirectory);

    std::size_t memoryAfter = getMemoryUsage();
    std::cout << "Memory before indexing: " << memoryBefore / (1024 * 1024) << " MB\n";

    std::cout << "Memory after indexing:  " << memoryAfter / (1024 * 1024) << " MB\n";

    std::cout << "Memory increase:        " << (memoryAfter - memoryBefore) / (1024 * 1024) << " MB\n";   


    runBenchmark(indexer,queries,iterations);
    return 0;
}