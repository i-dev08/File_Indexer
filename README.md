# C++ File Indexer

A command-line file indexing and search tool built in **C++17** using `std::filesystem`.

The project recursively scans directories, builds metadata indexes for fast searching, persists the index to disk, and supports incremental updates when files are added, modified, or deleted.

## Features

* **Recursive directory scanning**

  * Scans selected directories and their subdirectories.
  * Supports indexing multiple directories.

* **File metadata indexing**

  * File name
  * Full path
  * File extension
  * File size
  * Last modified timestamp
  * Indexed root directory

* **Fast indexed search**

  * Exact filename search
  * Case-insensitive filename search
  * Filename substring search
  * Path search
  * Extension search
  * Size-based filtering

* **Trigram substring indexing**

  * Uses a trigram index to efficiently locate candidate files for substring searches.
  * Short queries fall back to linear search.
  * Candidate results are verified against the actual filename.

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

* **Interactive command-line interface**

* **CMake + Ninja build system**

## Project Structure

```text
cpp-file-indexer/

├── include/
│   └── FileIndexer.h
├── src/
│   ├── FileIndexer.cpp
│   └── main.cpp
├── tests/
├── data/
├── CMakeLists.txt
├── .gitignore
└── README.md
```

The `data/` directory is used for locally generated index data and is not tracked by Git.

## Requirements

* C++17 compatible compiler
* CMake
* Ninja

The project was developed and tested using **GCC with C++17**.

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

## Search Operations

The index supports several search operations:

### Search by filename

Supports both exact and substring-based filename searches.

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

Filename searches are case-insensitive.

### Search by path

Searches indexed file paths without case sensitivity.

```text
Search path: project/src
```

### Search by extension

Both of these inputs are supported:

```text
cpp
```

or:

```text
.cpp
```

### Search by size

Supports size comparisons such as:

```text
> 5000
< 10000
>= 5000
<= 10000
= 5000
```

Size queries can also use supported units where applicable.

## Index Architecture

The project maintains a primary collection of `FileInfo` objects:

```cpp
struct FileInfo {
    std::string name;
    std::string path;
    std::string extension;
    std::uintmax_t size;
    std::string root;
    std::filesystem::file_time_type lastModified;
};
```

The `files` vector acts as the source of truth.

Several secondary indexes store positions into this vector:

```text
files
  │
  ├── nameIndex
  ├── extensionIndex
  ├── pathIndex
  └── ngramIndex
```

### Name Index

Maps normalized filenames to file indexes for fast exact filename lookup.

```text
filename → [file indexes]
```

### Extension Index

Maps normalized extensions to file indexes.

```text
extension → [file indexes]
```

### Path Index

Maps a full file path directly to its file index.

```text
path → file index
```

### Trigram Index

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

The index maps each trigram to candidate file indexes. Candidate lists are intersected and the resulting files are verified using the actual substring search.

Queries shorter than three characters use a linear-search fallback.

## Persistent Index

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

## Incremental Indexing

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

## Multi-Root Indexing

Multiple directories can be indexed by the same `FileIndexer` instance.

Each file stores the root from which it was indexed, allowing incremental updates to associate newly discovered files with the correct indexed root.

The persisted index also stores all indexed roots.

## Technologies

* **C++17**
* **STL**
* **`std::filesystem`**
* **`std::unordered_map`**
* **`std::unordered_set`**
* **CMake**
* **Ninja**

## Future Improvements

Potential future improvements include:

* Searching inside file contents
* Combining multiple metadata filters
* Sorting and ranking search results
* Duplicate file detection using hashing
* Unit and integration tests
* Benchmarking with large file collections
* Measuring query latency and memory usage
* More robust command-line input validation
* Additional filesystem error handling

## Author

**Ishika R Dev**
