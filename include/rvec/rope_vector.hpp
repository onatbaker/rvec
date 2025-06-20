#pragma once

#include <vector>
#include <memory>
#include <cassert>
#include <stdexcept>
#include <iterator>

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
                chunks.emplace_back(std::make_unique<T[]>(ChunkSize));
            }
        }

    public:
        rope_vector() = default;

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

        void insert(size_type pos, const T& value)
        {
            if (pos > total_size)
            {
                throw std::out_of_range("insert position out of bounds");
            }

            push_back(T{});

            for (size_type i = total_size - 1; i > pos; --i)
            {
                (*this)[i] = std::move((*this)[i - 1]);
            }

            (*this)[pos] = value;
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
    };

} // namespace rvec
