# C++ File Indexer

A command-line file indexing and search tool built in **C++17** using `std::filesystem`.

The project recursively scans a directory, stores metadata about discovered files, and provides multiple ways to search the resulting index.

## Features

* **Recursive directory scanning**

  * Scans the selected directory and its subdirectories.
* **File metadata indexing**

  * File name
  * Full path
  * File extension
  * File size
* **Case-insensitive filename search**
* **Extension-based search**

  * Supports input such as `cpp` or `.cpp`
* **Minimum file-size filtering**
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
2. Search by extension
3. Search by size
4. Exit
```

## Example

### Search by filename

```text
Search filename: report

Results: 2

Name: report.cpp
Path: sample_data/project/report.cpp
Extension: .cpp
Size: 1842
```

### Search by extension

Both of these inputs are supported:

```text
cpp
```

or

```text
.cpp
```

### Search by minimum size

```text
Enter minimum size in bytes: 5000
```

The indexer returns files whose size is greater than or equal to the specified value.

## Technical Details

The project uses C++17's `std::filesystem` library for directory traversal and file metadata.

Each indexed file is represented by a `FileInfo` structure containing:

```cpp
struct FileInfo {
    std::string name;
    std::string path;
    std::string extension;
    std::uintmax_t size;
    std::string searchableName;
    std::string searchableExtension;
};
```

Searchable versions of filenames and extensions are stored in lowercase so that searches can be performed without case sensitivity.

## Future Improvements

Planned improvements include:

* File sorting by name, extension, and size
* More robust filesystem error handling
* File statistics and summaries
* Improved command-line input validation
* More advanced search capabilities
* Unit testing
* Performance improvements for larger directories

## Technologies

* **C++17**
* **STL**
* **`std::filesystem`**
* **CMake**
* **Ninja**

## Author

**Ishika R Dev**
