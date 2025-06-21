#include <iostream>
#include <cassert>
#include "rvec/rope_vector.hpp"

int main() {
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

    return 0;
}
