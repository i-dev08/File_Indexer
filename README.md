# C++ File Indexer

A command-line file indexing and search tool built in **C++17** using `std::filesystem`.

The project recursively scans directories, builds metadata indexes for fast searching, persists the index to disk, and supports incremental updates when files are added, modified, or deleted.

## Features

* **Recursive directory scanning**

  * Scans selected directories and their subdirectories.
  * Supports indexing multiple directories.

* **File metadata indexing**

  * File name
  * Normalized file name
  * Full path
  * File extension
  * File size
  * Last modified timestamp
  * Indexed root directory

* **Indexed search**

  * Case-insensitive filename substring search
  * Path search
  * Extension search
  * Size-based filtering

* **Trigram substring indexing**

  * Uses a trigram index to efficiently locate candidate files for filename substring searches.
  * Uses compact `uint32_t` representations for trigrams.
  * Starts searches with the smallest available posting list.
  * Intersects sorted posting lists using a two-pointer algorithm.
  * Verifies candidate results against the actual filename.
  * Queries shorter than three characters use a linear-search fallback.

* **Persistent index**

  * Saves the metadata index to disk.
  * Loads the index when the program starts.
  * Avoids rebuilding the entire index after every restart.
  * Derived search indexes are rebuilt from persisted metadata.

* **Incremental indexing**

  * Detects newly created files.
  * Detects deleted files.
  * Detects modified files.
  * Updates only the affected entries instead of rebuilding the entire index.

* **Memory-conscious data structures**

  * Uses compact integer trigram keys instead of storing trigrams as strings.
  * Avoids maintaining a separate exact-name index because substring search already supports exact-name queries.
  * Uses swap-and-pop removal to keep the primary file vector compact.
  * Returns file indexes from internal search operations rather than unnecessarily copying complete `FileInfo` objects.

* **Interactive command-line interface**

* **CMake + Ninja build system**

---

## Project Structure

```text
cpp-file-indexer/

├── include/
│   └── FileIndexer.h
├── src/
│   ├── FileIndexer.cpp
│   ├── main.cpp
│   └── benchmark.cpp
├── tests/
├── data/
├── CMakeLists.txt
├── .gitignore
└── README.md
```

The `data/` directory is used for locally generated index data and benchmark data and is not tracked by Git.

## Requirements

* C++17 compatible compiler
* CMake
* Ninja

The project was developed and tested using **GCC with C++17** on Windows/MSYS2.

## Building

Clone the repository:

```bash
git clone <repository-url>

cd cpp-file-indexer
```

Configure the project with CMake and Ninja:

```bash
cmake -S . -B build -G Ninja
```

Build:

```bash
cmake --build build
```

## Running

On Windows/MSYS2:

```bash
./build/file_indexer.exe
```

On first launch, the program allows a directory to be added to the index.

Once an index has been created, it is saved to disk and loaded automatically on subsequent launches.

---

# Search Operations

The index supports several search operations.

## Search by filename

Filename searches are case-insensitive and support substring matching.

```text
Search filename: report
```

Example result:

```text
Name: report.cpp

Path: sample_data/project/report.cpp

Extension: .cpp

Size: 1842
```

Searching for the complete filename also produces an exact-name match through the substring search system.

## Search by path

Searches indexed file paths using the path-search functionality.

```text
Search path: project/src
```

## Search by extension

Both of these inputs are supported:

```text
cpp
```

or:

```text
.cpp
```

## Search by size

Supports size comparisons such as:

```text
> 5000
< 10000
>= 5000
<= 10000
= 5000
```

Size queries can also use supported units where applicable.

---

# Index Architecture

The project maintains a primary collection of `FileInfo` objects:

```cpp
struct FileInfo {

    std::string name;

    std::string normalizedName;

    std::string path;

    std::string extension;

    std::uintmax_t size;

    std::string root;

    std::filesystem::file_time_type lastModified;
};
```

The `files` vector acts as the source of truth.

Secondary indexes store positions into this vector:

```text
files
  │
  ├── extensionIndex
  ├── pathIndex
  └── ngramIndex
```

This keeps the metadata in one primary structure while allowing specialized indexes to accelerate different types of queries.

## Normalized Filename

Each file stores a normalized, lowercase version of its filename.

This avoids repeatedly allocating and lowercasing filenames during search operations.

```text
Original filename
       ↓
   normalize
       ↓
normalizedName
       ↓
search verification
```

The additional memory required for this cache was measured against the benchmark dataset and retained because it provided a measurable search-performance improvement.

## Extension Index

Maps normalized extensions to file indexes.

```text
extension → [file indexes]
```

This allows extension searches to directly access the relevant files.

## Path Index

Maps a full file path directly to its file index.

```text
path → file index
```

This provides constant-average-time exact path lookups and is used for efficient filesystem validation and incremental index updates.

## Trigram Index

Filename substring searches use three-character sequences, or **trigrams**.

For example:

```text
report.cpp
```

can produce trigrams such as:

```text
rep
epo
por
ort
rt.
t.c
.cpp
```

Internally, each trigram is represented as a compact `uint32_t` value.

The index maps each trigram to a sorted list of candidate file indexes:

```text
trigram → [file indexes]
```

## Search Optimization

For a query containing multiple trigrams, the search process is:

```text
Query
  ↓
Generate trigrams
  ↓
Find posting lists
  ↓
Choose smallest posting list
  ↓
Intersect sorted posting lists
  ↓
Verify actual substring
  ↓
Return file indexes
```

Starting with the smallest posting list reduces the number of candidates that need to be processed.

Posting lists are naturally sorted because file indexes are appended as files are indexed. This allows intersections to be performed using a two-pointer technique rather than constructing temporary hash sets.

Queries shorter than three characters fall back to a linear search.

---

# Persistent Index

The index can be serialized to disk and loaded again without rescanning the indexed directories.

The persisted data contains:

* Indexed roots
* File metadata
* File sizes
* Last modified timestamps
* File paths
* File extensions

Derived search indexes are rebuilt when the index is loaded rather than being stored separately.

This keeps the persisted representation focused on the source-of-truth metadata.

---

# Incremental Indexing

The index can be validated against the current filesystem state.

During validation, files are classified as:

```text
Valid
Modified
Missing
New
```

`updateIndex()` then applies only the required changes:

```text
Modified file
    ↓
Update metadata

Missing file
    ↓
Remove from files + indexes

New file
    ↓
Create FileInfo + add to indexes
```

Deleted files are handled using **swap-and-pop** so that the main file vector remains compact.

When a file is removed from the middle of the vector, the final element is moved into its position and the affected indexes are updated accordingly.

---

# Multi-Root Indexing

Multiple directories can be indexed by the same `FileIndexer` instance.

Each file stores the root from which it was indexed, allowing incremental updates to associate newly discovered files with the correct indexed root.

The persisted index also stores all indexed roots.

---

# Benchmarking

The project includes a separate benchmark executable for measuring filename search performance and memory usage.

The benchmark generates a dataset containing **50,000 files** with a deliberately skewed filename distribution:

```text
report       30,000 files
project      12,500 files
algorithm     5,000 files
client        2,500 files
```

The benchmark performs **1,000 searches per query**.

A representative run of the current implementation produced:

```text
Memory before indexing: 5 MB
Memory after indexing:  37 MB
Memory increase:        31 MB

Query: report
Average: 14987.9 us
Matches: 30000

Query: project
Average: 8287.35 us
Matches: 12500

Query: algorithm
Average: 4187.76 us
Matches: 5000

Query: xyz987
Average: 1.282 us
Matches: 0
```

These results correspond to approximately:

```text
report       ~15.0 ms
project       ~8.3 ms
algorithm     ~4.2 ms
xyz987        ~1.3 µs
```

The benchmark results are workload- and machine-dependent. Runtime can vary because of operating-system scheduling, CPU state, filesystem caching, and other system conditions.

The benchmark is intended primarily for comparing implementation changes under the same workload rather than as an absolute performance guarantee.

---

# Optimization Decisions

Several optimization approaches were benchmarked during development.

## Retained

* Smallest posting list selection
* Sorted-vector posting-list intersection
* Compact `uint32_t` trigram representation
* Cached normalized filenames
* Index-based search results instead of copying complete `FileInfo` objects
* Hash-based path indexing

## Rejected after benchmarking

* Sorting all query posting lists before intersection
* Manual character-by-character case-insensitive substring verification
* Maintaining a separate exact-name index
* Replacing the exact path index with a hierarchical folder structure

The folder hierarchy was specifically tested as an alternative to the path index. Although it could represent the directory structure naturally, benchmark results showed that exact path lookup was substantially faster with the hash-based `pathIndex`.

The project therefore favors **measured optimizations rather than optimization based solely on intuition**.

---

# Technologies

* **C++17**
* **STL**
* **`std::filesystem`**
* **`std::unordered_map`**
* **`std::vector`**
* **CMake**
* **Ninja**
* **GCC**

---

# Future Improvements

Potential future improvements include:

* Combining multiple metadata filters
* Sorting and ranking search results
* Duplicate file detection using hashing
* Unit and integration tests
* More robust command-line input validation
* Additional filesystem error handling
* Further memory profiling and data-structure optimization

---

# Author

**Ishika R Dev**
