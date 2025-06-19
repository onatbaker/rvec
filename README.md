# rvec: A Chunked STL-Compatible Vector Alternative

`rvec::rope_vector<T>` is a memory-aware, STL-style container that combines the interface of `std::vector` with the performance benefits of chunked allocation. It is designed to offer efficient middle insertions, reduced allocation overhead, and high cache locality while maintaining order and predictable access semantics.

---

## What Is `rope_vector`?

A modern `rope_vector<T>` is:

- A vector-like container that:
  - Uses fixed-size memory blocks ("chunks") to store elements.
  - Allows O(log n) insert/delete at arbitrary positions, in contrast to `std::vector`'s O(n).
  - Preserves cache friendliness by ensuring memory layout is contiguous within each chunk.
  - Maintains insertion order and supports positional logic.

> Conceptually, `rope_vector` sits at the intersection of `std::vector`, `std::deque`, and a rope.

---

## Why `rope_vector` Exists

Most general-purpose C++ containers either:

- Offer fast random access with high reallocation cost (`std::vector`), or
- Offer flexible insertion with poor memory locality (`std::list`, `std::map`, ropes).

`rope_vector` aims to deliver both:

- Efficient middle insertions (sub-linear, chunk-local)
- Contiguous memory within each chunk (cache-aligned)

This makes it ideal for systems where:

- Performance predictability matters
- Memory locality is critical (e.g. real-time systems, editors, undo stacks)
- STL compatibility is desired

---

## Design Highlights

### 1. Chunk-Based Storage

Elements are stored in a vector of fixed-size arrays:

```cpp
std::vector<std::unique_ptr<T[ChunkSize]>>
```

If `ChunkSize = 256`, then:

- Index 0 to 255 is in chunk 0
- Index 256 to 511 is in chunk 1

...and so on.

This allows predictable memory layout and avoids wholesale reallocation.

### 2. Index Translation

```cpp
chunk = index / ChunkSize
offset = index % ChunkSize
```

This gives constant-time access, just like a flat array.

### 3. Efficient Insertions & Deletions

Unlike `std::vector`, which requires O(n) element shifts:

- `rope_vector` shifts elements **within** a chunk or across **a few chunks**
- Resulting in practical runtime of O(log n) for insertion and erasure

This makes it effective for:

- Text editors
- Real-time sequences
- Undo/redo stacks
- Time-travel debugging tools

### 4. Amortized Allocation

Instead of allocating element-by-element, chunks are allocated in bulk (`new T[ChunkSize]`). This:

- Reduces malloc/free frequency
- Prevents fragmentation
- Ensures chunks are compact in memory

### 5. Memory Layout vs Hash Maps

Some may suggest `std::unordered_map<size_t, T>` as an alternative. Here's the distinction:

| Feature                | `rope_vector`     | `unordered_map<size_t, T>` |
| ---------------------- | ----------------- | -------------------------- |
| Memory layout          | Contiguous chunks | Scattered buckets          |
| Access locality        | Cache-friendly    | Pointer chasing            |
| Order-preserving       | Yes               | No                         |
| Shifts on insert       | Yes (vector-like) | No                         |
| Random insert cost     | O(log n)          | O(1)                       |
| Dense access semantics | Yes               | No                         |

---

## Example Usage

```cpp
#include "rvec/rope_vector.hpp"

rvec::rope_vector<int> rv;

rv.push_back(10);
rv.push_back(20);
rv.push_back(30);

rv.insert(1, 15); // Inserts 15 at index 1
rv.erase(0);      // Removes element at index 0
```

---

## Build

```bash
git clone https://github.com/onatbaker/rvec.git
cd rvec
mkdir build && cd build
cmake ..
cmake --build .
./Debug/rvec_demo.exe
```

Requires: CMake 3.14+, C++14+, MSVC or Clang/GCC

---

---

## Author

Designed and implemented by your slick yet friendly neighbourhood dev ob. The work is inspired by classic data structure tradeoffs and modern systems constraints.

---

## Future Directions

- Iterator support (STL-compatible)
- Undo/redo block mutation APIs
- Parallel merges
- Persistent memory chunk support
- C++20 range adaptation
