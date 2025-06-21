#pragma once

#include <vector>
#include <memory>
#include <cassert>
#include <stdexcept>

namespace rvec
{

    template <typename T, std::size_t ChunkSize = 256>
    class rope_vector
    {
    public:
        using value_type = T;
        using size_type = std::size_t;

    private:
        std::vector<std::unique_ptr<T[]>> chunks;
        size_type total_size = 0;

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

        T* allocate_chunk()
        {
            return new T[ChunkSize];
        }

        void free_chunk(T* chunk)
        {
            delete[] chunk;
        }

    public:
        rope_vector() = default;

        // move constructor
        rope_vector(rope_vector&& other) noexcept
            : chunks(std::move(other.chunks)),
            total_size(other.total_size)
        {
            other.total_size = 0;
        }

        // move assignment
        rope_vector& operator=(rope_vector&& other) noexcept
        {
            if (this != &other)
            {
                chunks = std::move(other.chunks);
                total_size = other.total_size;
                other.total_size = 0;
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
            return chunks[chunk_index(i)][within_chunk_index(i)];
        }

        const T& operator[](size_type i) const
        {
            assert(i < total_size);
            return chunks[chunk_index(i)][within_chunk_index(i)];
        }

        T& at(size_type i)
        {
            if (i >= total_size)
            {
                throw std::out_of_range("rvec::at() index out of range");
            }
            return (*this)[i];
        }

        const T& at(size_type i) const
        {
            if (i >= total_size)
            {
                throw std::out_of_range("rvec::at() index out of range");
            }
            return (*this)[i];
        }

        T& front()
        {
            if (empty())
            {
                throw std::out_of_range("rvec::front() called on empty vector");
            }
            return (*this)[0];
        }

        const T& front() const
        {
            if (empty())
            {
                throw std::out_of_range("rvec::front() called on empty vector");
            }
            return (*this)[0];
        }

        T& back()
        {
            if (empty())
            {
                throw std::out_of_range("rvec::back() called on empty vector");
            }
            return (*this)[total_size - 1];
        }

        const T& back() const
        {
            if (empty())
            {
                throw std::out_of_range("rvec::back() called on empty vector");
            }
            return (*this)[total_size - 1];
        }

        void clear()
        {
            total_size = 0;
        }

        void resize(size_type new_size)
        {
            if (new_size < total_size)
            {
                total_size = new_size;
            }
            else
            {
                ensure_capacity_for(new_size - 1);
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
                ensure_capacity_for(n - 1);
            }
        }

        // returns how many elements can be stored without growing
        size_type capacity() const noexcept
        {
            return chunks.size() * ChunkSize;
        }

        // deallocates unused chunks beyond current size
        void shrink_to_fit()
        {
            size_type required_chunks = chunk_index(total_size) + (within_chunk_index(total_size) ? 1 : 0);
            while (chunks.size() > required_chunks)
            {
                free_chunk(chunks.back().release());
                chunks.pop_back();
            }
        }

        void push_back(const T& value)
        {
            ensure_capacity_for(total_size);
            chunks[chunk_index(total_size)][within_chunk_index(total_size)] = value;
            ++total_size;
        }

        void push_back(T&& value)
        {
            ensure_capacity_for(total_size);
            chunks[chunk_index(total_size)][within_chunk_index(total_size)] = std::move(value);
            ++total_size;
        }

        template <typename... Args>
        void emplace_back(Args&&... args)
        {
            ensure_capacity_for(total_size);
            new (&chunks[chunk_index(total_size)][within_chunk_index(total_size)])
                T(std::forward<Args>(args)...);
            ++total_size;
        } // constructs T in-place using placement new to avoid copies or moves

        void insert(size_type pos, T&& value)
        {
            if (pos > total_size)
            {
                throw std::out_of_range("insert position out of bounds");
            }

            push_back(T{}); // extend space for 1 more element

            for (size_type i = total_size - 1; i > pos; --i)
            {
                (*this)[i] = std::move((*this)[i - 1]);
            }

            (*this)[pos] = std::move(value);
        }

        void erase(size_type pos)
        {
            if (pos >= total_size)
            {
                throw std::out_of_range("erase position out of bounds");
            }

            for (size_type i = pos; i < total_size - 1; ++i)
            {
                (*this)[i] = std::move((*this)[i + 1]);
            }

            --total_size;
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
