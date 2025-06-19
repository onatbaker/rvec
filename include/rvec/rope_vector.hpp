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

            push_back(T{}); // make space

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
    };

} // namespace rvec
