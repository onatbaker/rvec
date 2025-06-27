#pragma once

#include <vector>
#include <memory>
#include <cassert>

namespace rvec
{

    template <typename T, std::size_t ChunkSize = 256>
    class rope_vector
    {
    public:
        using value_type = T;
        using size_type = std::size_t;

        ~rope_vector()
        {
            for (T* chunk : chunks)
            {
                free_chunk(chunk);
            }
        }

        size_type memory_used() const
        {
            // returns total memory used by all chunks in bytes
            return chunks.size() * ChunkSize * sizeof(T);
        }

        double fragmentation() const
        {
            // returns 1.0 = completely unused; 0.0 = fully packed
            if (chunks.empty())
            {
                return 0.0;
            }

            size_type used_slots = total_size;
            size_type total_slots = chunks.size() * ChunkSize;

            return 1.0 - static_cast<double>(used_slots) / total_slots;
        }

    private:
        std::vector<T*> chunks;
        size_type total_size = 0;
        size_type start_index = 0; // for logical indexing
        size_type front_chunk_index = 0;

        static constexpr size_type chunk_index(size_type i)
        {
            return i / ChunkSize;
        }

        static constexpr size_type within_chunk_index(size_type i)
        {
            return i % ChunkSize;
        }

        void ensure_capacity_for(size_type i)
        {
            while (i >= chunks.size() * ChunkSize)
            {
                // chunks.emplace_back(std::make_unique<T[]>(ChunkSize));
                chunks.emplace_back(allocate_chunk()); // decouples allocation logic from smart pointer strategy
            }
        }

        void grow_front()
        {
            chunks.insert(chunks.begin(), allocate_chunk());
            start_index += ChunkSize;
            ++front_chunk_index;
        }

        T* allocate_chunk()
        {
            // std::cout << "[allocating chunk]" << std::endl;
            return new T[ChunkSize];
        }

        void free_chunk(T* chunk)
        {
            // std::cout << "[freeing chunk]" << std::endl;
            delete[] chunk;
        }

    public:
        rope_vector() = default;

        // move constructor
        rope_vector(rope_vector&& other) noexcept
            : chunks(std::move(other.chunks)),
            total_size(other.total_size),
            start_index(other.start_index),
            front_chunk_index(other.front_chunk_index)
        {
            other.total_size = 0;
            other.start_index = 0;
            other.front_chunk_index = 0;
        }

        // move assignment
        rope_vector& operator=(rope_vector&& other) noexcept
        {
            if (this != &other)
            {
                chunks = std::move(other.chunks);
                total_size = other.total_size;
                start_index = other.start_index;
                front_chunk_index = other.front_chunk_index;
                other.total_size = 0;
                other.start_index = 0;
                other.front_chunk_index = 0;
            }
            return *this;
        }

        size_type size() const noexcept
        {
            return total_size;
        }

        bool empty() const noexcept
        {
            return total_size == 0;
        }

        T& operator[](size_type i)
        {
            assert(i < total_size);
            size_type real_index = start_index + i;
            return chunks[front_chunk_index + chunk_index(real_index)][within_chunk_index(real_index)];
        }

        const T& operator[](size_type i) const
        {
            assert(i < total_size);
            size_type real_index = start_index + i;
            return chunks[front_chunk_index + chunk_index(real_index)][within_chunk_index(real_index)];
        }

        T& at(size_type i)
        {
            assert(i < total_size && "rvec::at() index out of range");
            return (*this)[i];
        }

        const T& at(size_type i) const
        {
            assert(i < total_size && "rvec::at() index out of range");
            return (*this)[i];
        }

        T& front()
        {
            assert(!empty() && "rvec::front() called on empty vector");
            return (*this)[0];
        }

        const T& front() const
        {
            assert(!empty() && "rvec::front() called on empty vector");
            return (*this)[0];
        }

        T& back()
        {
            assert(!empty() && "rvec::back() called on empty vector");
            return (*this)[total_size - 1];
        }

        const T& back() const
        {
            assert(!empty() && "rvec::back() called on empty vector");
            return (*this)[total_size - 1];
        }

        // TODO: we reset the logical size but don't free the memory. handle l8r
        void clear()
        {
            for (auto chunk : chunks)
            {
                free_chunk(chunk);
            }
            chunks.clear();
            total_size = 0;
            start_index = 0;
            front_chunk_index = 0;
        }

        void resize(size_type new_size)
        {
            if (new_size < total_size)
            {
                total_size = new_size;
            }
            else
            {
                ensure_capacity_for(start_index + new_size - 1);
                for (size_type i = total_size; i < new_size; ++i)
                {
                    chunks[chunk_index(i)][within_chunk_index(i)] = T{};
                }
                total_size = new_size;
            }
        }

        // ensures enough chunks to hold at least n elements
        void reserve(size_type n)
        {
            if (n > capacity())
            {
                ensure_capacity_for(start_index + n - 1);
            }
        }

        // returns how many elements can be stored without growing
        size_type capacity() const noexcept
        {
            return (chunks.size() - front_chunk_index) * ChunkSize - start_index;
        }

        // deallocates unused chunks beyond current size
        void shrink_to_fit()
        {
            size_type required_chunks = chunk_index(start_index + total_size) + (within_chunk_index(start_index + total_size) ? 1 : 0);
            while (chunks.size() > required_chunks)
            {
                // free_chunk(chunks.back().release());
                free_chunk(chunks.back()); // no more unique_ptr
                chunks.pop_back();
            }
        }

        void push_back(const T& value)
        {
            ensure_capacity_for(start_index + total_size);
            (*this)[total_size++] = value;
        }

        void push_back(T&& value)
        {
            ensure_capacity_for(start_index + total_size);
            (*this)[total_size++] = std::move(value);
        }

        template <typename... Args>
        void emplace_back(Args&&... args)
        {
            ensure_capacity_for(start_index + total_size);
            new (&(*this)[total_size++]) T(std::forward<Args>(args)...);
        }

        // TODO: insert optimization - choose the cheapest direction to shift existing elements...
        // ...improves performance for mid/front insertions by avoiding unnecessary moves
        void insert(size_type pos, T&& value)
        {
            assert(pos <= total_size);
            ensure_capacity_for(start_index + total_size);

            if (pos == 0)
            {
                if (start_index == 0)
                {
                    grow_front();
                }
                --start_index;
                ++total_size;
                (*this)[0] = std::move(value);
            }
            else if (pos == total_size)
            {
                push_back(std::move(value));
            }
            else if (pos < total_size / 2)
            {
                if (start_index == 0)
                {
                    grow_front();
                }
                --start_index;
                ++total_size;
                for (size_type i = 0; i < pos; ++i)
                {
                    (*this)[i] = std::move((*this)[i + 1]);
                }
                (*this)[pos] = std::move(value);
            }
            else
            {
                for (size_type i = total_size; i > pos; --i)
                {
                    (*this)[i] = std::move((*this)[i - 1]);
                }
                (*this)[pos] = std::move(value);
                ++total_size;
            }
        }

        void erase(size_type pos)
        {
            assert(pos < total_size && "erase position out of bounds");

            for (size_type i = pos; i < total_size - 1; ++i)
            {
                (*this)[i] = std::move((*this)[i + 1]);
            }

            --total_size;
        }

        void erase_front()
        {
            assert(!empty());

            ++start_index;
            --total_size;

            if (start_index >= ChunkSize)
            {
                free_chunk(chunks[front_chunk_index]);
                ++front_chunk_index;
                start_index -= ChunkSize;
            }
        }

        bool operator==(const rope_vector& other) const
        {
            if (total_size != other.total_size)
            {
                return false;
            }

            for (size_type i = 0; i < total_size; ++i)
            {
                if ((*this)[i] != other[i])
                {
                    return false;
                }
            }

            return true;
        }

        bool operator!=(const rope_vector& other) const
        {
            return !(*this == other);
        }

        void swap(rope_vector& other) noexcept
        {
            chunks.swap(other.chunks);
            std::swap(total_size, other.total_size);
            std::swap(start_index, other.start_index);
            std::swap(front_chunk_index, other.front_chunk_index);
        }


        class iterator
        {
        public:
            using iterator_category = std::random_access_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = T*;
            using reference = T&;

        private:
            rope_vector* parent = nullptr;
            size_type index = 0;

        public:
            iterator() = default;

            iterator(rope_vector* rv, size_type i)
                : parent(rv), index(i)
            {
            }

            reference operator*() const
            {
                return (*parent)[index];
            }

            pointer operator->() const
            {
                return &(*parent)[index];
            }

            iterator& operator++()
            {
                ++index;
                return *this;
            }

            iterator operator++(int)
            {
                iterator tmp = *this;
                ++(*this);
                return tmp;
            }

            iterator& operator--()
            {
                --index;
                return *this;
            }

            iterator operator--(int)
            {
                iterator tmp = *this;
                --(*this);
                return tmp;
            }

            iterator& operator+=(difference_type n)
            {
                index += n;
                return *this;
            }

            iterator& operator-=(difference_type n)
            {
                index -= n;
                return *this;
            }

            iterator operator+(difference_type n) const
            {
                return iterator(parent, index + n);
            }

            iterator operator-(difference_type n) const
            {
                return iterator(parent, index - n);
            }

            difference_type operator-(const iterator& other) const
            {
                return static_cast<difference_type>(index) - static_cast<difference_type>(other.index);
            }

            bool operator==(const iterator& other) const
            {
                return index == other.index;
            }

            bool operator!=(const iterator& other) const
            {
                return index != other.index;
            }

            bool operator<(const iterator& other) const
            {
                return index < other.index;
            }

            bool operator>(const iterator& other) const
            {
                return index > other.index;
            }

            bool operator<=(const iterator& other) const
            {
                return index <= other.index;
            }

            bool operator>=(const iterator& other) const
            {
                return index >= other.index;
            }
        };

        iterator begin()
        {
            return iterator(this, 0);
        }

        iterator end()
        {
            return iterator(this, total_size);
        }

        class const_iterator
        {
        public:
            using iterator_category = std::random_access_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = const T*;
            using reference = const T&;

        private:
            const rope_vector* parent = nullptr;
            size_type index = 0;

        public:
            const_iterator() = default;

            const_iterator(const rope_vector* rv, size_type i)
                : parent(rv), index(i)
            {
            }

            reference operator*() const
            {
                return (*parent)[index];
            }

            pointer operator->() const
            {
                return &(*parent)[index];
            }

            const_iterator& operator++()
            {
                ++index;
                return *this;
            }

            const_iterator operator++(int)
            {
                const_iterator tmp = *this;
                ++(*this);
                return tmp;
            }

            const_iterator& operator--()
            {
                --index;
                return *this;
            }

            const_iterator operator--(int)
            {
                const_iterator tmp = *this;
                --(*this);
                return tmp;
            }

            const_iterator& operator+=(difference_type n)
            {
                index += n;
                return *this;
            }

            const_iterator& operator-=(difference_type n)
            {
                index -= n;
                return *this;
            }

            const_iterator operator+(difference_type n) const
            {
                return const_iterator(parent, index + n);
            }

            const_iterator operator-(difference_type n) const
            {
                return const_iterator(parent, index - n);
            }

            difference_type operator-(const const_iterator& other) const
            {
                return static_cast<difference_type>(index) - static_cast<difference_type>(other.index);
            }

            reference operator[](difference_type n) const
            {
                return (*parent)[index + n];
            }

            bool operator==(const const_iterator& other) const
            {
                return parent == other.parent && index == other.index;
            }

            bool operator!=(const const_iterator& other) const
            {
                return !(*this == other);
            }

            bool operator<(const const_iterator& other) const
            {
                return index < other.index;
            }

            bool operator>(const const_iterator& other) const
            {
                return index > other.index;
            }

            bool operator<=(const const_iterator& other) const
            {
                return index <= other.index;
            }

            bool operator>=(const const_iterator& other) const
            {
                return index >= other.index;
            }
        };

        const_iterator cbegin() const noexcept
        {
            return const_iterator(this, 0);
        }

        const_iterator cend() const noexcept
        {
            return const_iterator(this, total_size);
        }

        const_iterator begin() const noexcept
        {
            return cbegin();
        }

        const_iterator end() const noexcept
        {
            return cend();
        }

        class reverse_iterator
        {
        public:
            using iterator_category = std::random_access_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = T*;
            using reference = T&;

        private:
            rope_vector* parent = nullptr;
            size_type index = 0;

        public:
            reverse_iterator() = default;

            reverse_iterator(rope_vector* rv, size_type i)
                : parent(rv), index(i)
            {
            }

            reference operator*() const
            {
                return (*parent)[index - 1];
            }

            pointer operator->() const
            {
                return &(*parent)[index - 1];
            }

            reverse_iterator& operator++()
            {
                --index;
                return *this;
            }

            reverse_iterator operator++(int)
            {
                reverse_iterator tmp = *this;
                --(*this);
                return tmp;
            }

            reverse_iterator& operator--()
            {
                ++index;
                return *this;
            }

            reverse_iterator operator--(int)
            {
                reverse_iterator tmp = *this;
                ++(*this);
                return tmp;
            }

            reverse_iterator& operator+=(difference_type n)
            {
                index -= n;
                return *this;
            }

            reverse_iterator& operator-=(difference_type n)
            {
                index += n;
                return *this;
            }

            reverse_iterator operator+(difference_type n) const
            {
                return reverse_iterator(parent, index - n);
            }

            reverse_iterator operator-(difference_type n) const
            {
                return reverse_iterator(parent, index + n);
            }

            difference_type operator-(const reverse_iterator& other) const
            {
                return static_cast<difference_type>(other.index) - static_cast<difference_type>(index);
            }

            bool operator==(const reverse_iterator& other) const
            {
                return index == other.index && parent == other.parent;
            }

            bool operator!=(const reverse_iterator& other) const
            {
                return !(*this == other);
            }

            bool operator<(const reverse_iterator& other) const
            {
                return index > other.index;
            }

            bool operator>(const reverse_iterator& other) const
            {
                return index < other.index;
            }

            bool operator<=(const reverse_iterator& other) const
            {
                return index >= other.index;
            }

            bool operator>=(const reverse_iterator& other) const
            {
                return index <= other.index;
            }
        };

        reverse_iterator rbegin()
        {
            return reverse_iterator(this, total_size);
        }

        reverse_iterator rend()
        {
            return reverse_iterator(this, 0);
        }

        class const_reverse_iterator
        {
        public:
            using iterator_category = std::random_access_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = const T*;
            using reference = const T&;

        private:
            const rope_vector* parent = nullptr;
            size_type index = 0;

        public:
            const_reverse_iterator() = default;

            const_reverse_iterator(const rope_vector* rv, size_type i)
                : parent(rv), index(i)
            {
            }

            reference operator*() const
            {
                return (*parent)[index - 1];
            }

            pointer operator->() const
            {
                return &(*parent)[index - 1];
            }

            const_reverse_iterator& operator++()
            {
                --index;
                return *this;
            }

            const_reverse_iterator operator++(int)
            {
                const_reverse_iterator tmp = *this;
                --(*this);
                return tmp;
            }

            const_reverse_iterator& operator--()
            {
                ++index;
                return *this;
            }

            const_reverse_iterator operator--(int)
            {
                const_reverse_iterator tmp = *this;
                ++(*this);
                return tmp;
            }

            const_reverse_iterator& operator+=(difference_type n)
            {
                index -= n;
                return *this;
            }

            const_reverse_iterator& operator-=(difference_type n)
            {
                index += n;
                return *this;
            }

            const_reverse_iterator operator+(difference_type n) const
            {
                return const_reverse_iterator(parent, index - n);
            }

            const_reverse_iterator operator-(difference_type n) const
            {
                return const_reverse_iterator(parent, index + n);
            }

            difference_type operator-(const const_reverse_iterator& other) const
            {
                return static_cast<difference_type>(other.index) - static_cast<difference_type>(index);
            }

            bool operator==(const const_reverse_iterator& other) const
            {
                return index == other.index && parent == other.parent;
            }

            bool operator!=(const const_reverse_iterator& other) const
            {
                return !(*this == other);
            }

            bool operator<(const const_reverse_iterator& other) const
            {
                return index > other.index;
            }

            bool operator>(const const_reverse_iterator& other) const
            {
                return index < other.index;
            }

            bool operator<=(const const_reverse_iterator& other) const
            {
                return index >= other.index;
            }

            bool operator>=(const const_reverse_iterator& other) const
            {
                return index <= other.index;
            }
        };

        const_reverse_iterator rbegin() const
        {
            return const_reverse_iterator(this, total_size);
        }

        const_reverse_iterator rend() const
        {
            return const_reverse_iterator(this, 0);
        }

        const_reverse_iterator crbegin() const
        {
            return const_reverse_iterator(this, total_size);
        }

        const_reverse_iterator crend() const
        {
            return const_reverse_iterator(this, 0);
        }
    };

    template <typename T, std::size_t ChunkSize>
    void swap(rope_vector<T, ChunkSize>& a, rope_vector<T, ChunkSize>& b) noexcept
    {
        a.swap(b);
    }
} // namespace rvec
