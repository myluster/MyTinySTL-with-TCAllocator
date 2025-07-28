#pragma once
#include <iostream>
#include <cassert>
#include <string>
#include <unordered_map> // For comparison and std::hash

#include "../include/vector.h"
#include "../include/unordered_map.h" // Your unordered_map header

namespace mystl {
    namespace test {
        namespace unordered_map_test {

            void unordered_map_test() {
                std::cout << "[===============================================================]\n";
                std::cout << "[-------------- Run container test : unordered_map -------------]\n";
                std::cout << "[--------------------------- API test --------------------------]\n";

                // 1. Constructors
                {
                    std::cout << "\n*** 1. Testing constructors ***\n";
                    mystl::unordered_map<int, std::string> um1;
                    assert(um1.empty() && um1.size() == 0);

                    mystl::unordered_map<int, std::string> um2(100);
                    assert(um2.bucket_count() >= 100);

                    mystl::vector<mystl::pair<const int, std::string>> v_pairs = {
                        {1, "one"}, {2, "two"}, {3, "three"}
                    };
                    mystl::unordered_map<int, std::string> um3(v_pairs.begin(), v_pairs.end());
                    assert(um3.size() == 3);
                    assert(um3[1] == "one");

                    mystl::unordered_map<int, std::string> um4({ {4, "four"}, {5, "five"} });
                    assert(um4.size() == 2);
                    assert(um4[4] == "four");

                    mystl::unordered_map<int, std::string> um5(um4);
                    assert(um5.size() == 2 && um5[5] == "five");

                    mystl::unordered_map<int, std::string> um6(std::move(um5));
                    assert(um6.size() == 2 && um6[5] == "five");
                    assert(um5.empty());
                    std::cout << "Constructors PASSED\n";
                }

                // 2. Assignments
                {
                    std::cout << "\n*** 2. Testing assignments ***\n";
                    mystl::unordered_map<int, std::string> src{ {1, "a"}, {2, "b"} };
                    mystl::unordered_map<int, std::string> um1;

                    um1 = src;
                    assert(um1.size() == 2 && um1[1] == "a");

                    mystl::unordered_map<int, std::string> um2;
                    um2 = std::move(src);
                    assert(um2.size() == 2 && um2[2] == "b");
                    assert(src.empty());

                    um1 = { {10, "ten"}, {20, "twenty"} };
                    assert(um1.size() == 2 && um1[10] == "ten");
                    std::cout << "Assignments PASSED\n";
                }

                // 3. Element access
                {
                    std::cout << "\n*** 3. Testing element access ***\n";
                    mystl::unordered_map<int, std::string> um{ {1, "apple"}, {2, "banana"} };

                    assert(um[1] == "apple");
                    um[1] = "apricot";
                    assert(um[1] == "apricot");

                    assert(um[3] == ""); // Inserts default
                    assert(um.size() == 3);

                    assert(um.at(2) == "banana");
                    try {
                        um.at(99);
                        assert(false);
                    }
                    catch (const std::out_of_range&) {
                        assert(true);
                    }
                    std::cout << "Element access PASSED\n";
                }

                // 4. Modifiers
                {
                    std::cout << "\n*** 4. Testing modifiers ***\n";
                    mystl::unordered_map<int, std::string> um;

                    auto p1 = um.emplace(2, "two");
                    assert(p1.second && p1.first->first == 2);

                    auto p2 = um.emplace(2, "another two");
                    assert(!p2.second && p2.first->second == "two");

                    auto p3 = um.insert({ 1, "one" });
                    assert(p3.second);

                    size_t erased_count = um.erase(1);
                    assert(erased_count == 1 && um.find(1) == um.end());

                    um.clear();
                    assert(um.empty());
                    std::cout << "Modifiers PASSED\n";
                }

                // 5. Lookup Operations
                {
                    std::cout << "\n*** 5. Testing lookup operations ***\n";
                    mystl::unordered_map<int, int> um{ {10,1}, {20,2}, {30,3} };

                    assert(um.find(20)->second == 2);
                    assert(um.find(99) == um.end());
                    assert(um.count(30) == 1);
                    assert(um.count(99) == 0);

                    auto range = um.equal_range(10);
                    assert(range.first->first == 10);
                    assert(mystl::distance(range.first, range.second) == 1);
                    std::cout << "Lookup operations PASSED\n";
                }

                // 6. Bucket Interface & Hash Policy
                {
                    std::cout << "\n*** 6. Testing buckets and hash policy ***\n";
                    mystl::unordered_map<int, int> um;
                    um.reserve(100); // Should allocate at least 100 elements worth of buckets
                    assert(um.bucket_count() > 10);

                    for (int i = 0; i < 50; ++i) um.emplace(i, i);

                    assert(um.load_factor() > 0);
                    um.max_load_factor(0.5f);
                    assert(um.max_load_factor() == 0.5f);

                    size_t old_buckets = um.bucket_count();
                    um.rehash(200);
                    assert(um.bucket_count() >= 200);
                    assert(um.bucket_count() > old_buckets);
                    std::cout << "Buckets and hash policy PASSED\n";
                }

                std::cout << "[-------------- End container test : unordered_map -------------]\n";
                std::cout << "[===============================================================]\n\n";
            }

            void unordered_multimap_test() {
                std::cout << "[===============================================================]\n";
                std::cout << "[----------- Run container test : unordered_multimap -----------]\n";
                std::cout << "[--------------------------- API test --------------------------]\n";

                // 1. Constructors and insert
                {
                    std::cout << "\n*** 1. Testing constructors and insert ***\n";
                    mystl::unordered_multimap<int, std::string> umm;
                    umm.insert({ 1, "one" });
                    umm.insert({ 1, "another one" });
                    umm.insert({ 2, "two" });

                    assert(umm.size() == 3);
                    assert(umm.count(1) == 2);
                    assert(umm.count(2) == 1);
                    assert(umm.count(3) == 0);
                    std::cout << "Constructors and insert PASSED\n";
                }

                // 2. Modifiers
                {
                    std::cout << "\n*** 2. Testing modifiers ***\n";
                    mystl::unordered_multimap<int, std::string> umm{
                        {1, "a"}, {2, "b"}, {1, "c"}, {3, "d"}, {1, "e"}
                    };

                    size_t erased_count = umm.erase(1);
                    assert(erased_count == 3);
                    assert(umm.count(1) == 0);
                    assert(umm.size() == 2);

                    auto it = umm.find(2);
                    umm.erase(it);
                    assert(umm.find(2) == umm.end());
                    assert(umm.size() == 1);
                    std::cout << "Modifiers PASSED\n";
                }

                // 3. Lookup Operations
                {
                    std::cout << "\n*** 3. Testing lookup operations ***\n";
                    mystl::unordered_multimap<int, int> umm;
                    umm.insert({ 10, 1 });
                    umm.insert({ 20, 2 });
                    umm.insert({ 10, 3 });

                    assert(umm.count(10) == 2);

                    auto range = umm.equal_range(10);
                    assert(mystl::distance(range.first, range.second) == 2);

                    bool found1 = false, found3 = false;
                    for (auto it = range.first; it != range.second; ++it) {
                        if (it->second == 1) found1 = true;
                        if (it->second == 3) found3 = true;
                    }
                    assert(found1 && found3);
                    std::cout << "Lookup operations PASSED\n";
                }

                std::cout << "[----------- End container test : unordered_multimap -----------]\n";
                std::cout << "[===============================================================]\n\n";
            }

        } // namespace unordered_map_test
    } // namespace test
} // namespace mystl