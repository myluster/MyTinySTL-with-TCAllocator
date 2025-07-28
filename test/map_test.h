#pragma once
#pragma once
#include <iostream>
#include <cassert>
#include <map> // For comparison with std::map
#include <string>
#include "../include/vector.h" 
#include "../include/map.h" // Assuming your map.h is in ../include/
#include "../include/utils.h"
namespace mystl {
    namespace test {
        namespace map_test {

            void map_test() {
                std::cout << "[===============================================================]\n";
                std::cout << "[------------------ Run container test : map -------------------]\n";
                std::cout << "[--------------------------- API test --------------------------]\n";

                // Test constructors
                {
                    std::cout << "*** Testing constructors ***\n";
                    mystl::map<int, std::string> m1; // Default constructor
                    assert(m1.empty());
                    assert(m1.size() == 0);

                    // Range constructor
                    mystl::vector<mystl::pair<const int, std::string>> v_pairs = {
                        mystl::make_pair(1, "one"),
                        mystl::make_pair(3, "three"),
                        mystl::make_pair(2, "two")
                    };
                    mystl::map<int, std::string> m2(v_pairs.begin(), v_pairs.end());
                    assert(m2.size() == 3);
                    assert(m2.begin()->first == 1);
                    assert(m2.rbegin()->first == 3);

                    // Initializer list constructor
                    mystl::map<int, std::string> m3{ {4, "four"}, {6, "six"}, {5, "five"} };
                    assert(m3.size() == 3);
                    assert(m3.begin()->first == 4);
                    assert(m3.rbegin()->first == 6);

                    // Copy constructor
                    mystl::map<int, std::string> m4(m3);
                    assert(m4.size() == 3);
                    assert(m4.begin()->first == 4);
                    assert(m4.rbegin()->first == 6);

                    // Move constructor
                    mystl::map<int, std::string> m5(std::move(m4));
                    assert(m5.size() == 3);
                    assert(m4.empty()); // Original object should be empty after move
                }

                // Test assignment operators
                {
                    std::cout << "\n*** Testing assignments ***\n";
                    mystl::map<int, std::string> src{ {1, "a"}, {2, "b"} };
                    mystl::map<int, std::string> m1;

                    // Copy assignment
                    m1 = src;
                    assert(m1.size() == 2);
                    assert(m1[1] == "a");

                    // Move assignment
                    mystl::map<int, std::string> m2;
                    m2 = std::move(src);
                    assert(m2.size() == 2);
                    assert(m2[2] == "b");
                    assert(src.empty()); // Original object should be empty

                    // Initializer list assignment
                    m1 = { {10, "ten"}, {20, "twenty"} };
                    assert(m1.size() == 2);
                    assert(m1[10] == "ten");
                }

                // Test element access
                {
                    std::cout << "\n*** Testing element access ***\n";
                    mystl::map<int, std::string> m{ {1, "apple"}, {2, "banana"} };

                    // operator[] for existing key
                    assert(m[1] == "apple");
                    m[1] = "apricot";
                    assert(m[1] == "apricot");

                    // operator[] for new key (inserts default constructed value)
                    assert(m[3] == ""); // Default constructed string is empty
                    assert(m.size() == 3);
                    m[3] = "cherry";
                    assert(m[3] == "cherry");

                    // at() for existing key
                    assert(m.at(2) == "banana");
                    m.at(2) = "blueberry";
                    assert(m.at(2) == "blueberry");

                    // at() for non-existing key (should throw out_of_range)
                    try {
                        m.at(99) = "grape";
                        assert(false); // Should not reach here
                    }
                    catch (const std::out_of_range& e) {
                        assert(true);
                    }
                }

                // Test iterators
                {
                    std::cout << "\n*** Testing iterators ***\n";
                    mystl::map<int, int> m{ {1, 10}, {3, 30}, {2, 20} };

                    auto it = m.begin();
                    assert(it->first == 1);
                    assert((++it)->first == 2);
                    assert((++it)->first == 3);
                    assert(++it == m.end());

                    auto rit = m.rbegin();
                    assert(rit->first == 3);
                    assert((++rit)->first == 2);
                    assert((++rit)->first == 1);
                    assert(++rit == m.rend());
                }

                // Test capacity
                {
                    std::cout << "\n*** Testing capacity ***\n";
                    mystl::map<int, int> m;
                    assert(m.empty());
                    assert(m.size() == 0);

                    m.insert({ 1, 1 });
                    assert(!m.empty());
                    assert(m.size() == 1);

                    for (int i = 2; i <= 10; ++i) {
                        m.insert({ i, i });
                    }
                    assert(m.size() == 10);
                }

                // Test modifiers
                {
                    std::cout << "\n*** Testing modifiers ***\n";
                    mystl::map<int, std::string> m;

                    // emplace
                    auto p1 = m.emplace(2, "two");
                    assert(p1.second == true);
                    assert(p1.first->first == 2);
                    assert(m.size() == 1);

                    // emplace existing key (should not insert)
                    auto p2 = m.emplace(2, "another two");
                    assert(p2.second == false);
                    assert(p2.first->first == 2);
                    assert(p2.first->second == "two"); // Original value should remain
                    assert(m.size() == 1);

                    // emplace_hint
                    auto it_hint = m.begin();
                    auto it1 = m.emplace_hint(it_hint, 1, "one");
                    assert(it1->first == 1);
                    assert(m.size() == 2);

                    auto it2 = m.emplace_hint(m.end(), 3, "three");
                    assert(it2->first == 3);
                    assert(m.size() == 3);

                    // insert(const value_type&)
                    auto p3 = m.insert(mystl::make_pair(4, "four"));
                    assert(p3.second == true);
                    assert(p3.first->first == 4);
                    assert(m.size() == 4);

                    // insert(value_type&&)
                    mystl::pair<const int, std::string> mv_pair(5, "five");
                    auto p4 = m.insert(std::move(mv_pair));
                    assert(p4.second == true);
                    assert(p4.first->first == 5);
                    assert(m.size() == 5);

                    // insert(hint, const value_type&)
                    auto it3 = m.insert(m.find(3), mystl::make_pair(0, "zero"));
                    assert(it3->first == 0);
                    assert(m.size() == 6);

                    // insert(first, last)
                    mystl::vector<mystl::pair<const int, std::string>> new_pairs = {
                        mystl::make_pair(6, "six"),
                        mystl::make_pair(7, "seven")
                    };
                    m.insert(new_pairs.begin(), new_pairs.end());
                    assert(m.size() == 8);
                    assert(m[6] == "six");

                    // erase(iterator)
                    auto it_erase = m.find(1);
                    m.erase(it_erase);
                    assert(m.size() == 7);
                    assert(m.find(1) == m.end());

                    // erase(key_type)
                    size_t erased_count = m.erase(2);
                    assert(erased_count == 1);
                    assert(m.size() == 6);
                    assert(m.find(2) == m.end());

                    // erase(first, last)
                    auto first_to_erase = m.find(3);
                    auto last_to_erase = m.find(6);
                    m.erase(first_to_erase, last_to_erase); // Erase 3, 4, 5
                    assert(m.size() == 3);
                    assert(m.find(3) == m.end());
                    assert(m.find(4) == m.end());
                    assert(m.find(5) == m.end());
                    assert(m.find(0) != m.end()); // 0 should still be there
                    assert(m.find(6) != m.end()); // 6 should still be there (last_to_erase is exclusive)

                    // clear
                    m.clear();
                    assert(m.empty());
                    assert(m.size() == 0);
                }

                // Test map operations
                {
                    std::cout << "\n*** Testing map operations ***\n";
                    mystl::map<int, int> m{ {10,1}, {20,2}, {30,3}, {40,4}, {50,5} };

                    // find
                    auto it_found = m.find(30);
                    assert(it_found != m.end());
                    assert(it_found->second == 3);
                    assert(m.find(99) == m.end()); // Not found

                    // count
                    assert(m.count(20) == 1);
                    assert(m.count(99) == 0);

                    // lower_bound
                    auto lb1 = m.lower_bound(25); // Should point to 30
                    assert(lb1->first == 30);
                    auto lb2 = m.lower_bound(10); // Should point to 10
                    assert(lb2->first == 10);
                    auto lb3 = m.lower_bound(60); // Should point to end()
                    assert(lb3 == m.end());

                    // upper_bound
                    auto ub1 = m.upper_bound(25); // Should point to 30
                    assert(ub1->first == 30);
                    auto ub2 = m.upper_bound(30); // Should point to 40
                    assert(ub2->first == 40);
                    auto ub3 = m.upper_bound(60); // Should point to end()
                    assert(ub3 == m.end());

                    // equal_range
                    auto er1 = m.equal_range(30);
                    assert(er1.first->first == 30);
                    assert(er1.second->first == 40); // For unique keys, second is next

                    auto er2 = m.equal_range(99);
                    assert(er2.first == m.end());
                    assert(er2.second == m.end());

                    // swap
                    mystl::map<int, int> m_other{ {100,10}, {200,20} };
                    m.swap(m_other);
                    assert(m.size() == 2);
                    assert(m[100] == 10);
                    assert(m_other.size() == 5);
                    assert(m_other[10] == 1);
                }

                // Test relational operators
                {
                    std::cout << "\n*** Testing relational operators ***\n";
                    mystl::map<int, int> m1{ {1,1}, {2,2} };
                    mystl::map<int, int> m2{ {1,1}, {2,2} };
                    mystl::map<int, int> m3{ {1,1}, {2,3} };
                    mystl::map<int, int> m4{ {1,1}, {2,2}, {3,3} };

                    assert(m1 == m2);
                    assert(m1 != m3);
                    assert(m1 < m4);
                    assert(m4 > m1);
                    assert(m1 <= m2);
                    assert(m4 >= m1);
                }

                std::cout << "[------------------- End container test : map ------------------]\n";
                std::cout << "[===============================================================]\n\n";
            }

            void multimap_test() {
                std::cout << "[===============================================================]\n";
                std::cout << "[----------------- Run container test : multimap ---------------]\n";
                std::cout << "[--------------------------- API test --------------------------]\n";

                // Test constructors
                {
                    std::cout << "*** Testing constructors ***\n";
                    mystl::multimap<int, std::string> mm1; // Default constructor
                    assert(mm1.empty());
                    assert(mm1.size() == 0);

                    // Range constructor
                    mystl::vector<mystl::pair<const int, std::string>> v_pairs = {
                        mystl::make_pair(1, "one"),
                        mystl::make_pair(3, "three"),
                        mystl::make_pair(2, "two"),
                        mystl::make_pair(1, "another one") // Duplicate key
                    };
                    mystl::multimap<int, std::string> mm2(v_pairs.begin(), v_pairs.end());
                    assert(mm2.size() == 4);
                    assert(mm2.begin()->first == 1);
                    assert(mm2.rbegin()->first == 3);
                    assert(mm2.count(1) == 2);

                    // Initializer list constructor
                    mystl::multimap<int, std::string> mm3{ {4, "four"}, {6, "six"}, {5, "five"}, {4, "another four"} };
                    assert(mm3.size() == 4);
                    assert(mm3.begin()->first == 4);
                    assert(mm3.rbegin()->first == 6);
                    assert(mm3.count(4) == 2);

                    // Copy constructor
                    mystl::multimap<int, std::string> mm4(mm3);
                    assert(mm4.size() == 4);
                    assert(mm4.count(4) == 2);

                    // Move constructor
                    mystl::multimap<int, std::string> mm5(std::move(mm4));
                    assert(mm5.size() == 4);
                    assert(mm4.empty()); // Original object should be empty after move
                }

                // Test assignment operators
                {
                    std::cout << "\n*** Testing assignments ***\n";
                    mystl::multimap<int, std::string> src{ {1, "a"}, {2, "b"}, {1, "c"} };
                    mystl::multimap<int, std::string> mm1;

                    // Copy assignment
                    mm1 = src;
                    assert(mm1.size() == 3);
                    assert(mm1.count(1) == 2);

                    // Move assignment
                    mystl::multimap<int, std::string> mm2;
                    mm2 = std::move(src);
                    assert(mm2.size() == 3);
                    assert(mm2.count(1) == 2);
                    assert(src.empty()); // Original object should be empty

                    // Initializer list assignment
                    mm1 = { {10, "ten"}, {20, "twenty"}, {10, "another ten"} };
                    assert(mm1.size() == 3);
                    assert(mm1.count(10) == 2);
                }

                // Test iterators
                {
                    std::cout << "\n*** Testing iterators ***\n";
                    mystl::multimap<int, int> mm{ {1, 10}, {3, 30}, {2, 20}, {1, 15} }; // Keys 1, 1, 2, 3

                    auto it = mm.begin();
                    assert(it->first == 1); // First '1'
                    assert((++it)->first == 1); // Second '1'
                    assert((++it)->first == 2);
                    assert((++it)->first == 3);
                    assert(++it == mm.end());

                    auto rit = mm.rbegin();
                    assert(rit->first == 3);
                    assert((++rit)->first == 2);
                    assert((++rit)->first == 1); // Second '1' from end (value 15)
                    assert((++rit)->first == 1); // First '1' from end (value 10)
                    assert(++rit == mm.rend());
                }

                // Test capacity
                {
                    std::cout << "\n*** Testing capacity ***\n";
                    mystl::multimap<int, int> mm;
                    assert(mm.empty());
                    assert(mm.size() == 0);

                    mm.insert({ 1, 1 });
                    assert(!mm.empty());
                    assert(mm.size() == 1);

                    for (int i = 2; i <= 10; ++i) {
                        mm.insert({ i, i });
                        mm.insert({ i, i + 100 }); // Duplicate key
                    }
                    assert(mm.size() == 1 + 9 * 2); // 1 + 18 = 19
                    assert(mm.count(2) == 2);
                }

                // Test modifiers
                {
                    std::cout << "\n*** Testing modifiers ***\n";
                    mystl::multimap<int, std::string> mm;

                    // emplace
                    auto it1 = mm.emplace(2, "two");
                    assert(it1->first == 2);
                    assert(mm.size() == 1);

                    // emplace duplicate key
                    auto it2 = mm.emplace(2, "another two");
                    assert(it2->first == 2);
                    assert(it2->second == "another two"); // Should insert
                    assert(mm.size() == 2);
                    assert(mm.count(2) == 2);

                    // emplace_hint
                    auto it_hint = mm.begin();
                    auto it3 = mm.emplace_hint(it_hint, 1, "one");
                    assert(it3->first == 1);
                    assert(mm.size() == 3);

                    auto it4 = mm.emplace_hint(mm.end(), 3, "three");
                    assert(it4->first == 3);
                    assert(mm.size() == 4);

                    // insert(const value_type&)
                    auto it5 = mm.insert(mystl::make_pair(4, "four"));
                    assert(it5->first == 4);
                    assert(mm.size() == 5);

                    // insert(value_type&&)
                    mystl::pair<const int, std::string> mv_pair(5, "five");
                    auto it6 = mm.insert(std::move(mv_pair));
                    assert(it6->first == 5);
                    assert(mm.size() == 6);

                    // insert(hint, const value_type&)
                    auto it7 = mm.insert(mm.find(3), mystl::make_pair(0, "zero"));
                    assert(it7->first == 0);
                    assert(mm.size() == 7);

                    // insert(first, last)
                    mystl::vector<mystl::pair<const int, std::string>> new_pairs = {
                        mystl::make_pair(6, "six"),
                        mystl::make_pair(7, "seven"),
                        mystl::make_pair(6, "another six")
                    };
                    mm.insert(new_pairs.begin(), new_pairs.end());
                    assert(mm.size() == 10);
                    assert(mm.count(6) == 2);

                    // erase(iterator)
                    auto it_erase = mm.find(1);
                    mm.erase(it_erase);
                    assert(mm.size() == 9);
                    assert(mm.find(1) == mm.end()); // Only one '1' was present, so now none. If multiple, one is removed.

                    // erase(key_type) - removes all with that key
                    size_t erased_count = mm.erase(2);
                    assert(erased_count == 2); // Two '2's were present
                    assert(mm.size() == 7);
                    assert(mm.find(2) == mm.end());

                    // erase(first, last)
                    auto first_to_erase = mm.find(3);
                    auto last_to_erase = mm.find(6);
                    mm.erase(first_to_erase, last_to_erase); // Erase 3, 4, 5
                    assert(mm.size() == 4);
                    assert(mm.find(3) == mm.end());
                    assert(mm.find(4) == mm.end());
                    assert(mm.find(5) == mm.end());
                    assert(mm.find(0) != mm.end());
                    assert(mm.count(6) == 2); // 6s should still be there

                    // clear
                    mm.clear();
                    assert(mm.empty());
                    assert(mm.size() == 0);
                }

                // Test multimap operations
                {
                    std::cout << "\n*** Testing multimap operations ***\n";
                    mystl::multimap<int, int> mm{ {10,1}, {20,2}, {30,3}, {20,22}, {40,4}, {50,5} };

                    // find
                    auto it_found = mm.find(20);
                    assert(it_found != mm.end());
                    assert(it_found->first == 20); // Could be either 2 or 22

                    // count
                    assert(mm.count(20) == 2);
                    assert(mm.count(99) == 0);

                    // lower_bound
                    auto lb1 = mm.lower_bound(20); // Should point to the first '20'
                    assert(lb1->first == 20);
                    auto lb2 = mm.lower_bound(25); // Should point to 30
                    assert(lb2->first == 30);

                    // upper_bound
                    auto ub1 = mm.upper_bound(20); // Should point to 30 (after all '20's)
                    assert(ub1->first == 30);
                    auto ub2 = mm.upper_bound(25); // Should point to 30
                    assert(ub2->first == 30);

                    // equal_range
                    auto er1 = mm.equal_range(20);
                    assert(er1.first->first == 20);
                    assert(er1.second->first == 30);
                    assert(mystl::distance(er1.first, er1.second) == 2); // Two elements with key 20

                    auto er2 = mm.equal_range(99);
                    assert(er2.first == mm.end());
                    assert(er2.second == mm.end());

                    // swap
                    mystl::multimap<int, int> mm_other{ {100,10}, {200,20}, {100,101} };
                    mm.swap(mm_other);
                    assert(mm.size() == 3);
                    assert(mm.count(100) == 2);
                    assert(mm_other.size() == 6);
                    assert(mm_other.count(20) == 2);
                }

                // Test relational operators (same as map for basic functionality)
                {
                    std::cout << "\n*** Testing relational operators (multimap) ***\n";
                    mystl::multimap<int, int> mm1{ {1,1}, {2,2} };
                    mystl::multimap<int, int> mm2{ {1,1}, {2,2} };
                    mystl::multimap<int, int> mm3{ {1,1}, {2,3} };
                    mystl::multimap<int, int> mm4{ {1,1}, {2,2}, {3,3} };
                    mystl::multimap<int, int> mm5{ {1,1}, {1,1}, {2,2} };


                    assert(mm1 == mm2);
                    assert(mm1 != mm3);
                    assert(mm1 < mm4);
                    assert(mm4 > mm1);
                    assert(mm1 <= mm2);
                    assert(mm4 >= mm1);
                    assert(mm5 < mm1);
                }

                std::cout << "[----------------- End container test : multimap ---------------]\n";
                std::cout << "[===============================================================]\n\n";
            }

        } // namespace map_test
    } // namespace test
} // namespace mystl