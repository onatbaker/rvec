#include <iostream>
#include <cassert>
#include "rvec/rope_vector.hpp"

int main() {
    /*
    using rvec::rope_vector;

    rope_vector<int> rv;

    // testing the empty
    assert(rv.empty());
    assert(rv.size() == 0);

    // tseting push_back
    rv.push_back(10);
    rv.push_back(20);
    rv.push_back(30);

    assert(!rv.empty());
    assert(rv.size() == 3);
    assert(rv[0] == 10);
    assert(rv[1] == 20);
    assert(rv[2] == 30);
    assert(rv.front() == 10);
    assert(rv.back() == 30);
    assert(rv.at(1) == 20);

    // test insert
    rv.insert(1, 15); // 10, 15, 20, 30
    assert(rv.size() == 4);
    assert(rv[1] == 15);
    assert(rv[2] == 20);

    // test erase
    rv.erase(2); // 10, 15, 30
    assert(rv.size() == 3);
    assert(rv[2] == 30);

    // test resize larger
    rv.resize(5);
    assert(rv.size() == 5);
    assert(rv[3] == 0); // def init
    assert(rv[4] == 0);

    // test reszie smaller
    rv.resize(2);
    assert(rv.size() == 2);
    assert(rv[0] == 10);
    assert(rv[1] == 15);

    // test clear func
    rv.clear();
    assert(rv.size() == 0);
    assert(rv.empty());

    std::cout << "All sanity checks passed." << std::endl;
    */

    rvec::rope_vector<int> rv;

    // add some data
    for (int i = 1; i <= 5; ++i)
    {
        rv.push_back(i * 10); // 10, 20, 30, 40, 50
    }

    std::cout << "Mutable iteration using iterator:" << std::endl;
    for (auto it = rv.begin(); it != rv.end(); ++it)
    {
        std::cout << *it << " ";
        *it += 1; // testing write access
    }
    std::cout << std::endl;

    std::cout << "Const iteration using const_iterator:" << std::endl;
    const rvec::rope_vector<int>& const_ref = rv;

    for (auto it = const_ref.begin(); it != const_ref.end(); ++it)
    {
        std::cout << *it << " "; // cant mutate
        // *it += 1; // this triggers a compiler error!!!!
    }
    std::cout << std::endl;

    std::cout << "Range-based for loop on const ref:" << std::endl;
    for (const auto& val : const_ref)
    {
        std::cout << val << " ";
    }
    std::cout << std::endl;
    return 0;
}
