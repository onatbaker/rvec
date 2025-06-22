#include <iostream>
#include <cassert>
#include "rvec/rope_vector.hpp"

int main() {
    /*
    rvec::rope_vector<int> rv;

    // push_back
    for (int i = 1; i <= 5; ++i)
    {
        rv.push_back(i * 10);
    }

    std::cout << "Initial values with operator[]: ";
    for (std::size_t i = 0; i < rv.size(); ++i)
    {
        std::cout << rv[i] << " ";
    }
    std::cout << std::endl;

    // at(), front(), back()
    std::cout << "at(2): " << rv.at(2) << std::endl;
    std::cout << "front: " << rv.front() << std::endl;
    std::cout << "back: " << rv.back() << std::endl;

    // insert and erase
    rv.insert(2, 999);
    std::cout << "After insert at position 2: ";
    for (auto val : rv)
    {
        std::cout << val << " ";
    }
    std::cout << std::endl;

    rv.erase(2);
    std::cout << "After erase at position 2: ";
    for (auto val : rv)
    {
        std::cout << val << " ";
    }
    std::cout << std::endl;

    // const_iterator
    const rvec::rope_vector<int>& crv = rv;
    std::cout << "Using const_iterator: ";
    for (auto it = crv.cbegin(); it != crv.cend(); ++it)
    {
        std::cout << *it << " ";
    }
    std::cout << std::endl;

    // reverse_iterator
    std::cout << "Using reverse_iterator: ";
    for (auto it = rv.rbegin(); it != rv.rend(); ++it)
    {
        std::cout << *it << " ";
    }
    std::cout << std::endl;

    // const_reverse_iterator
    std::cout << "Using const_reverse_iterator: ";
    for (auto it = crv.crbegin(); it != crv.crend(); ++it)
    {
        std::cout << *it << " ";
    }
    std::cout << std::endl;

    // resize and clear
    rv.resize(3);
    std::cout << "After resize(3): ";
    for (auto val : rv)
    {
        std::cout << val << " ";
    }
    std::cout << std::endl;

    rv.clear();
    std::cout << "After clear(), size: " << rv.size() << ", empty: " << rv.empty() << std::endl;
    */

    /*
    // emplace_back
    rvec::rope_vector<std::pair<int, std::string>> pair_vec;
    pair_vec.emplace_back(1, "one");
    pair_vec.emplace_back(2, "two");

    std::cout << "test: emplace_back" << std::endl;
    for (const auto& p : pair_vec)
    {
        std::cout << "(" << p.first << ", " << p.second << ") ";
    }
    std::cout << std::endl;

    // operator== and operator!=
    rvec::rope_vector<int> a, b, c;
    a.push_back(1); a.push_back(2); a.push_back(3);
    b.push_back(1); b.push_back(2); b.push_back(3);
    c.push_back(1); c.push_back(2); c.push_back(4);

    std::cout << "test: operator== and operator!=" << std::endl;
    std::cout << "a == b: " << (a == b ? "true" : "false") << std::endl;
    std::cout << "a != c: " << (a != c ? "true" : "false") << std::endl;

    // swap() and std::swap
    std::cout << "test: swap and std::swap" << std::endl;
    std::cout << "Before swap: a.back() = " << a.back() << ", c.back() = " << c.back() << std::endl;
    swap(a, c);
    std::cout << "After swap: a.back() = " << a.back() << ", c.back() = " << c.back() << std::endl;

    // move constructor and move assignment
    std::cout << "test: move constructor and move assignment" << std::endl;
    rvec::rope_vector<int> moved = std::move(b);
    std::cout << "After move construction from b, moved.size(): " << moved.size() << std::endl;

    rvec::rope_vector<int> assigned;
    assigned = std::move(moved);
    std::cout << "After move assignment to assigned, assigned.size(): " << assigned.size() << std::endl;
    */
    std::cout << "test: memory_used() and fragmentation() " << std::endl;

    rvec::rope_vector<int> memtest;
    for (int i = 0; i < 600; ++i)
    {
        memtest.emplace_back(i);
    }

    std::cout << "memory used (bytes): " << memtest.memory_used() << std::endl;
    std::cout << "fragmentation: " << memtest.fragmentation() << std::endl << std::endl;

    return 0;
}
