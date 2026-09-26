# C++ File Indexer

A command-line file indexing and search tool built in **C++17** using `std::filesystem`.

The project recursively scans a directory, builds in-memory indexes for file metadata, and provides multiple ways to search the indexed files.

## Features

* **Recursive directory scanning**

  * Scans the selected directory and its subdirectories.
* **File metadata indexing**

  * File name
  * Full path
  * File extension
  * File size
* **Case-insensitive filename search**

  * Exact filename lookup using a hash-based index.
  * Substring filename search using a trigram (N-gram) index.
  * Automatically falls back to substring search when no exact filename match is found.
* **Case-insensitive path search**
* **Extension-based search**

  * Supports input such as `cpp` or `.cpp`.
* **File-size filtering**

  * Supports `>`, `<`, `>=`, `<=`, and `=` comparisons.
  * Supports units such as `B`, `KB`, `MB`, and `GB`.
* **Interactive command-line interface**

## Project Structure

```text
cpp-file-indexer/
├── include/
│   └── FileIndexer.h
├── src/
│   ├── FileIndexer.cpp
│   └── main.cpp
├── tests/
├── sample_data/
├── CMakeLists.txt
├── .gitignore
└── README.md
```

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

The program will ask for a directory to scan:

```text
Enter directory path: sample_data

Indexed 12 files.
```

You can then choose from the available search operations:

```text
==============================
       C++ FILE INDEXER
==============================

1. Search by filename
2. Search by path
3. Search by extension
4. Search by size
5. Exit
```

## Example

### Search by filename

The filename search first checks for an exact filename match.

```text
Search filename: report.cpp

Results: 1

Name: report.cpp
Path: sample_data/project/report.cpp
Extension: .cpp
Size: 1842
```

If an exact match is found, the user can optionally perform a substring search.

If no exact match is found, substring search is performed automatically.

For example:

```text
Search filename: report

No exact filename match found.
Searching for filenames containing "report"...
```

The substring search uses a **trigram index** to identify candidate files before verifying the actual filename match.

### Search by extension

Both of these inputs are supported:

```text
cpp
```

or

```text
.cpp
```

### Search by path

Path searches are case-insensitive and match paths containing the supplied query.

### Search by size

Size queries support comparison operators:

```text
> 10 MB
< 500 KB
>= 1 GB
<= 5 MB
= 1024 B
```

The parser also accepts decimal values such as:

```text
1.5 MB
```

## Technical Details

The project uses C++17's `std::filesystem` library for directory traversal and file metadata.

Each indexed file is represented by a `FileInfo` structure:

```cpp
struct FileInfo {
    std::string name;
    std::string path;
    std::string extension;
    std::uintmax_t size;
};
```

The project maintains several in-memory indexes.

### Filename index

Exact filename searches use an `std::unordered_map`:

```text
filename → file indices
```

This allows exact filename lookups without scanning every indexed file.

### Extension index

Extensions are also indexed using an `std::unordered_map`:

```text
extension → file indices
```

### N-gram index

Substring filename searches use a **trigram index**.

A filename such as:

```text
report.cpp
```

is divided into three-character sequences:

```text
rep
epo
por
ort
rt.
t.c
.cp
cpp
```

The index stores mappings from each trigram to the files containing it.

When a substring query is made, the query is divided into trigrams and their candidate file sets are intersected. The remaining candidates are then verified using the actual substring search.

This reduces the number of files that need to be examined during substring searches compared with scanning every filename.

### Source of truth

The `files` vector stores the actual `FileInfo` records.

The indexes store references to those records using their vector positions rather than duplicating the complete file metadata.

## Current Architecture

```text
             FILESYSTEM
                 │
                 ▼
          DIRECTORY SCANNER
                 │
                 ▼
             FileInfo
                 │
        ┌────────┼────────┐
        ▼        ▼        ▼
   nameIndex  extension  ngramIndex
        │        │        │
        └────────┼────────┘
                 ▼
            SEARCH ENGINE
                 │
                 ▼
              RESULTS
```

## Future Improvements

Planned improvements include:

* Persistent index storage
* Incremental indexing for new, modified, and deleted files
* File sorting and result ranking
* Duplicate file detection
* File content indexing and search
* Performance benchmarking on large datasets
* More robust filesystem error handling
* Improved command-line input validation
* Unit testing

## Technologies

* **C++17**
* **STL**
* **`std::filesystem`**
* **`std::unordered_map`**
* **`std::unordered_set`**
* **CMake**
* **Ninja**

## Author

**Ishika R Dev**
