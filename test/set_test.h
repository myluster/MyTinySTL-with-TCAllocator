#pragma once
#pragma once
#include <iostream>
#include <cassert>
#include <string>
#include <set>
#include <algorithm>
#include <initializer_list>

#include "../include/set.h"
#include "../include/vector.h" 

namespace mystl {
    namespace test {
        namespace set_test {

            void print_set(const mystl::set<int>& s, const std::string& name) {
                std::cout << name << ": { ";
                for (int val : s) {
                    std::cout << val << " ";
                }
                std::cout << "}\n";
            }

            void print_multiset(const mystl::multiset<int>& ms, const std::string& name) {
                std::cout << name << ": { ";
                for (int val : ms) {
                    std::cout << val << " ";
                }
                std::cout << "}\n";
            }

            void set_test() {
                std::cout << "[===============================================================]\n";
                std::cout << "[----------------- Run container test : set --------------------]\n";
                std::cout << "[-------------------------- API test ---------------------------]\n";

                // Test constructors
                {
                    std::cout << "\n*** Testing set constructors ***\n";
                    mystl::set<int> s1; // Default constructor
                    assert(s1.empty());
                    assert(s1.size() == 0);

                    int arr[] = { 5, 1, 4, 1, 9, 8 };
                    mystl::set<int> s2(std::begin(arr), std::end(arr)); // Iterator constructor
                    print_set(s2, "s2");
                    assert(s2.size() == 5); // Duplicates removed: 1, 4, 5, 8, 9
                    assert(*s2.begin() == 1);
                    assert(*s2.rbegin() == 9);

                    mystl::set<int> s3{ 3, 1, 4, 1, 5, 9, 2, 6 }; // Initializer list constructor
                    print_set(s3, "s3");
                    assert(s3.size() == 7); // Duplicates removed: 1, 2, 3, 4, 5, 6, 9
                    assert(*s3.begin() == 1);
                    assert(*s3.rbegin() == 9);

                    mystl::set<int> s4(s2); // Copy constructor
                    print_set(s4, "s4");
                    assert(s4.size() == s2.size());
                    assert(mystl::equal(s4.begin(), s4.end(), s2.begin()));

                    mystl::set<int> s5(std::move(s3)); // Move constructor
                    print_set(s5, "s5");
                    assert(s5.size() == 7);
                    assert(s3.empty()); // s3 should be empty after move
                }

                // Test assignment operators
                {
                    std::cout << "\n*** Testing set assignment operators ***\n";
                    mystl::set<int> s1;
                    mystl::set<int> s2{ 10, 20, 30 };

                    s1 = s2; // Copy assignment
                    print_set(s1, "s1 (copy assigned)");
                    assert(s1.size() == 3);
                    assert(mystl::equal(s1.begin(), s1.end(), s2.begin()));

                    mystl::set<int> s3{ 40, 50, 60 };
                    s1 = std::move(s3); // Move assignment
                    print_set(s1, "s1 (move assigned)");
                    assert(s1.size() == 3);
                    assert(s3.empty());

                    s1 = { 1, 2, 3, 4 }; // Initializer list assignment
                    print_set(s1, "s1 (initializer list assigned)");
                    assert(s1.size() == 4);
                    assert(*s1.begin() == 1);
                    assert(*s1.rbegin() == 4);
                }

                // Test iterators
                {
                    std::cout << "\n*** Testing set iterators ***\n";
                    mystl::set<int> s{ 10, 20, 30, 40, 50 };

                    std::cout << "Forward iteration: ";
                    for (auto it = s.begin(); it != s.end(); ++it) {
                        std::cout << *it << " ";
                    }
                    std::cout << "\n";
                    assert(*s.begin() == 10);
                    assert(*(--s.end()) == 50);

                    std::cout << "Reverse iteration: ";
                    for (auto it = s.rbegin(); it != s.rend(); ++it) {
                        std::cout << *it << " ";
                    }
                    std::cout << "\n";
                    assert(*s.rbegin() == 50);
                    assert(*(--s.rend()) == 10);

                    const mystl::set<int>& cs = s;
                    assert(*cs.cbegin() == 10);
                    assert(*cs.crbegin() == 50);
                }

                // Test capacity
                {
                    std::cout << "\n*** Testing set capacity ***\n";
                    mystl::set<int> s;
                    assert(s.empty());
                    assert(s.size() == 0);
                    s.insert(1);
                    assert(!s.empty());
                    assert(s.size() == 1);
                    // max_size is usually very large, so just check it's non-zero
                    assert(s.max_size() > 0);
                }

                // Test modifiers (insert, emplace, erase, clear, swap)
                {
                    std::cout << "\n*** Testing set modifiers ***\n";
                    mystl::set<int> s;

                    // emplace
                    auto p1 = s.emplace(5);
                    assert(p1.second == true && *p1.first == 5);
                    print_set(s, "s after emplace(5)");
                    auto p2 = s.emplace(5); // Duplicate
                    assert(p2.second == false && *p2.first == 5);
                    assert(s.size() == 1);

                    // insert(value_type)
                    auto p3 = s.insert(10);
                    assert(p3.second == true && *p3.first == 10);
                    print_set(s, "s after insert(10)");
                    s.insert(2);
                    s.insert(8);
                    s.insert(6);
                    print_set(s, "s after multiple inserts");
                    assert(s.size() == 5); // 2, 5, 6, 8, 10

                    // insert(hint, value_type)
                    s.insert(s.find(5), 4); // Insert near 5
                    print_set(s, "s after insert(hint, 4)");
                    assert(s.count(4) == 1);

                    s.insert(s.end(), 12); // Insert at end
                    print_set(s, "s after insert(end, 12)");
                    assert(s.count(12) == 1);

                    s.insert(s.begin(), 0); // Insert at beginning
                    print_set(s, "s after insert(begin, 0)");
                    assert(s.count(0) == 1);

                    // insert(InputIterator, InputIterator)
                    mystl::vector<int> v = { 100, 200, 10, 300 };
                    s.insert(v.begin(), v.end());
                    print_set(s, "s after insert(vector range)");
                    assert(s.count(100) == 1);
                    assert(s.count(200) == 1);
                    assert(s.count(300) == 1);
                    assert(s.size() == 11); // 0, 2, 4, 5, 6, 8, 10, 12, 100, 200, 300 - 10 is duplicate

                    // erase(iterator)
                    s.erase(s.find(5));
                    print_set(s, "s after erase(5)");
                    assert(s.count(5) == 0);
                    assert(s.size() == 10);

                    // erase(key_type)
                    s.erase(8);
                    print_set(s, "s after erase(8)");
                    assert(s.count(8) == 0);
                    assert(s.size() == 9);

                    assert(s.erase(999) == 0); // Erase non-existent key

                    // erase(first, last)
                    auto it_begin_erase = s.find(4);
                    auto it_end_erase = s.find(100);
                    s.erase(it_begin_erase, it_end_erase);
                    print_set(s, "s after erase(range 4 to 100)");
                    assert(s.count(4) == 0);
                    assert(s.count(6) == 0);
                    assert(s.count(10) == 0);
                    assert(s.count(12) == 0);
                    assert(s.size() == 5); // 0, 200, 300

                    // clear
                    s.clear();
                    print_set(s, "s after clear");
                    assert(s.empty());
                    assert(s.size() == 0);

                    // swap
                    mystl::set<int> s_swap1{ 1, 2, 3 };
                    mystl::set<int> s_swap2{ 4, 5, 6, 7 };
                    s_swap1.swap(s_swap2);
                    print_set(s_swap1, "s_swap1 after swap");
                    print_set(s_swap2, "s_swap2 after swap");
                    assert(s_swap1.size() == 4 && s_swap1.count(4) == 1);
                    assert(s_swap2.size() == 3 && s_swap2.count(1) == 1);
                }

                // Test set operations
                {
                    std::cout << "\n*** Testing set operations ***\n";
                    mystl::set<int> s{ 10, 20, 30, 40, 50 };

                    // find
                    assert(*s.find(30) == 30);
                    assert(s.find(99) == s.end());

                    // count
                    assert(s.count(30) == 1);
                    assert(s.count(99) == 0);

                    // lower_bound, upper_bound, equal_range
                    mystl::set<int> s_bounds{ 10, 20, 30, 30, 40, 50 }; // Note: set removes duplicates
                    // So for mystl::set it will be {10, 20, 30, 40, 50}
                    // The underlying rb_tree_unique ensures this.
                    // Let's create a scenario where lower_bound/upper_bound are clearly tested
                    mystl::set<int> s_lb_ub{ 10, 20, 35, 40, 50 };

                    auto lb_it = s_lb_ub.lower_bound(30);
                    assert(*lb_it == 35);
                    lb_it = s_lb_ub.lower_bound(35);
                    assert(*lb_it == 35);

                    auto ub_it = s_lb_ub.upper_bound(30);
                    assert(*ub_it == 35);
                    ub_it = s_lb_ub.upper_bound(35);
                    assert(*ub_it == 40);

                    auto eq_range = s_lb_ub.equal_range(35);
                    assert(*eq_range.first == 35);
                    assert(*eq_range.second == 40);

                    auto eq_range_non_existent = s_lb_ub.equal_range(33);
                    assert(*eq_range_non_existent.first == 35);
                    assert(*eq_range_non_existent.second == 35);
                }

                // Test relational operators
                {
                    std::cout << "\n*** Testing set relational operators ***\n";
                    mystl::set<int> s1{ 1, 2, 3 };
                    mystl::set<int> s2{ 1, 2, 3 };
                    mystl::set<int> s3{ 1, 2, 4 };
                    mystl::set<int> s4{ 1, 2 };

                    assert(s1 == s2);
                    assert(s1 != s3);
                    assert(s1 < s3);
                    assert(s3 > s1);
                    assert(s1 <= s2);
                    assert(s1 <= s3);
                    assert(s2 >= s1);
                    assert(s3 >= s1);
                    assert(s1 > s4);
                    assert(s4 < s1);
                }

                std::cout << "[------------------ End container test : set -----------------]\n";
                std::cout << "[===============================================================]\n\n";
            }

            void multiset_test() {
                std::cout << "[===============================================================]\n";
                std::cout << "[----------------- Run container test : multiset ---------------]\n";
                std::cout << "[-------------------------- API test ---------------------------]\n";

                // Test constructors
                {
                    std::cout << "\n*** Testing multiset constructors ***\n";
                    mystl::multiset<int> ms1; // Default constructor
                    assert(ms1.empty());
                    assert(ms1.size() == 0);

                    int arr[] = { 5, 1, 4, 1, 9, 8 };
                    mystl::multiset<int> ms2(std::begin(arr), std::end(arr)); // Iterator constructor
                    print_multiset(ms2, "ms2");
                    assert(ms2.size() == 6); // Duplicates allowed: 1, 1, 4, 5, 8, 9
                    assert(*ms2.begin() == 1);
                    assert(*ms2.rbegin() == 9);

                    mystl::multiset<int> ms3{ 3, 1, 4, 1, 5, 9, 2, 6 }; // Initializer list constructor
                    print_multiset(ms3, "ms3");
                    assert(ms3.size() == 8); // Duplicates allowed: 1, 1, 2, 3, 4, 5, 6, 9
                    assert(*ms3.begin() == 1);
                    assert(*ms3.rbegin() == 9);

                    mystl::multiset<int> ms4(ms2); // Copy constructor
                    print_multiset(ms4, "ms4");
                    assert(ms4.size() == ms2.size());
                    assert(mystl::equal(ms4.begin(), ms4.end(), ms2.begin()));

                    mystl::multiset<int> ms5(std::move(ms3)); // Move constructor
                    print_multiset(ms5, "ms5");
                    assert(ms5.size() == 8);
                    assert(ms3.empty()); // ms3 should be empty after move
                }

                // Test assignment operators
                {
                    std::cout << "\n*** Testing multiset assignment operators ***\n";
                    mystl::multiset<int> ms1;
                    mystl::multiset<int> ms2{ 10, 20, 30 };

                    ms1 = ms2; // Copy assignment
                    print_multiset(ms1, "ms1 (copy assigned)");
                    assert(ms1.size() == 3);
                    assert(mystl::equal(ms1.begin(), ms1.end(), ms2.begin()));

                    mystl::multiset<int> ms3{ 40, 50, 60 };
                    ms1 = std::move(ms3); // Move assignment
                    print_multiset(ms1, "ms1 (move assigned)");
                    assert(ms1.size() == 3);
                    assert(ms3.empty());

                    ms1 = { 1, 2, 3, 4 }; // Initializer list assignment
                    print_multiset(ms1, "ms1 (initializer list assigned)");
                    assert(ms1.size() == 4);
                    assert(*ms1.begin() == 1);
                    assert(*ms1.rbegin() == 4);
                }

                // Test iterators
                {
                    std::cout << "\n*** Testing multiset iterators ***\n";
                    mystl::multiset<int> ms{ 10, 20, 20, 30, 40, 50 };

                    std::cout << "Forward iteration: ";
                    for (auto it = ms.begin(); it != ms.end(); ++it) {
                        std::cout << *it << " ";
                    }
                    std::cout << "\n";
                    assert(*ms.begin() == 10);
                    assert(*(--ms.end()) == 50);

                    std::cout << "Reverse iteration: ";
                    for (auto it = ms.rbegin(); it != ms.rend(); ++it) {
                        std::cout << *it << " ";
                    }
                    std::cout << "\n";
                    assert(*ms.rbegin() == 50);
                    assert(*(++ms.rbegin()) == 40); // Second element in reverse is 40
                    assert(*(--ms.rend()) == 10);

                    const mystl::multiset<int>& cms = ms;
                    assert(*cms.cbegin() == 10);
                    assert(*cms.crbegin() == 50);
                }

                // Test capacity
                {
                    std::cout << "\n*** Testing multiset capacity ***\n";
                    mystl::multiset<int> ms;
                    assert(ms.empty());
                    assert(ms.size() == 0);
                    ms.insert(1);
                    assert(!ms.empty());
                    assert(ms.size() == 1);
                    assert(ms.max_size() > 0);
                }

                // Test modifiers (insert, emplace, erase, clear, swap)
                {
                    std::cout << "\n*** Testing multiset modifiers ***\n";
                    mystl::multiset<int> ms;

                    // emplace
                    auto it_emplace = ms.emplace(5);
                    assert(*it_emplace == 5);
                    print_multiset(ms, "ms after emplace(5)");
                    it_emplace = ms.emplace(5); // Duplicate allowed
                    assert(*it_emplace == 5);
                    assert(ms.size() == 2);

                    // insert(value_type)
                    ms.insert(10);
                    ms.insert(2);
                    ms.insert(8);
                    ms.insert(6);
                    ms.insert(6); // Duplicate
                    print_multiset(ms, "ms after multiple inserts");
                    assert(ms.size() == 7); // 2, 5, 5, 6, 6, 8, 10

                    // insert(hint, value_type)
                    ms.insert(ms.find(5), 4); // Insert near 5
                    print_multiset(ms, "ms after insert(hint, 4)");
                    assert(ms.count(4) == 1);

                    ms.insert(ms.end(), 12); // Insert at end
                    print_multiset(ms, "ms after insert(end, 12)");
                    assert(ms.count(12) == 1);

                    ms.insert(ms.begin(), 0); // Insert at beginning
                    print_multiset(ms, "ms after insert(begin, 0)");
                    assert(ms.count(0) == 1);

                    // insert(InputIterator, InputIterator)
                    mystl::vector<int> v = { 100, 200, 10, 300, 10 }; // Two 10s
                    ms.insert(v.begin(), v.end());
                    print_multiset(ms, "ms after insert(vector range)");
                    assert(ms.count(100) == 1);
                    assert(ms.count(200) == 1);
                    assert(ms.count(300) == 1);
                    assert(ms.count(10) == 3); // Original 10 + two from vector
                    assert(ms.size() == 15);

                    // erase(iterator)
                    ms.erase(ms.find(5)); // Erase one instance of 5
                    print_multiset(ms, "ms after erase(one 5)");
                    assert(ms.count(5) == 1);
                    assert(ms.size() == 14);

                    // erase(key_type)
                    ms.erase(6); // Erase all instances of 6
                    print_multiset(ms, "ms after erase(all 6s)");
                    assert(ms.count(6) == 0);
                    assert(ms.size() == 12);

                    assert(ms.erase(999) == 0); // Erase non-existent key

                    // erase(first, last)
                    auto it_begin_erase = ms.find(4);
                    auto it_end_erase = ms.find(100);
                    ms.erase(it_begin_erase, it_end_erase);
                    print_multiset(ms, "ms after erase(range 4 to 100)");
                    assert(ms.count(4) == 0);
                    assert(ms.count(8) == 0);
                    assert(ms.count(10) == 0); // All 10s erased
                    assert(ms.count(12) == 0);
                    assert(ms.size() == 5); // 0, 5, 200, 300

                    // clear
                    ms.clear();
                    print_multiset(ms, "ms after clear");
                    assert(ms.empty());
                    assert(ms.size() == 0);

                    // swap
                    mystl::multiset<int> ms_swap1{ 1, 2, 3 };
                    mystl::multiset<int> ms_swap2{ 4, 5, 6, 7 };
                    ms_swap1.swap(ms_swap2);
                    print_multiset(ms_swap1, "ms_swap1 after swap");
                    print_multiset(ms_swap2, "ms_swap2 after swap");
                    assert(ms_swap1.size() == 4 && ms_swap1.count(4) == 1);
                    assert(ms_swap2.size() == 3 && ms_swap2.count(1) == 1);
                }

                // Test multiset operations
                {
                    std::cout << "\n*** Testing multiset operations ***\n";
                    mystl::multiset<int> ms{ 10, 20, 30, 20, 40, 50, 30 }; // 10, 20, 20, 30, 30, 40, 50

                    // find
                    assert(*ms.find(20) == 20); // Returns iterator to first 20
                    assert(ms.find(99) == ms.end());

                    // count
                    assert(ms.count(20) == 2);
                    assert(ms.count(30) == 2);
                    assert(ms.count(99) == 0);

                    // lower_bound, upper_bound, equal_range
                    auto lb_it = ms.lower_bound(20);
                    assert(*lb_it == 20); // First 20

                    auto ub_it = ms.upper_bound(20);
                    assert(*ub_it == 30); // First element greater than 20

                    auto eq_range = ms.equal_range(20);
                    assert(*eq_range.first == 20); // Iterator to first 20
                    assert(*eq_range.second == 30); // Iterator to first 30

                    assert(mystl::distance(eq_range.first, eq_range.second) == 2);

                    auto eq_range_non_existent = ms.equal_range(25);
                    assert(*eq_range_non_existent.first == 30);
                    assert(*eq_range_non_existent.second == 30);
                }

                // Test relational operators (same as set, relies on underlying rb_tree)
                {
                    std::cout << "\n*** Testing multiset relational operators ***\n";
                    mystl::multiset<int> ms1{ 1, 2, 3 };
                    mystl::multiset<int> ms2{ 1, 2, 3 };
                    mystl::multiset<int> ms3{ 1, 2, 3, 4 };
                    mystl::multiset<int> ms4{ 1, 2, 2 };

                    assert(ms1 == ms2);
                    assert(ms1 != ms3);
                    assert(ms1 < ms3);
                    assert(ms3 > ms1);
                    assert(ms1 <= ms2);
                    assert(ms1 <= ms3);
                    assert(ms2 >= ms1);
                    assert(ms3 >= ms1);
                    assert(ms4 < ms1); // {1,2,2} < {1,2,3}
                    assert(ms1 > ms4);
                }

                std::cout << "[------------------ End container test : multiset --------------]\n";
                std::cout << "[===============================================================]\n\n";
            }

        } // namespace set_test
    } // namespace test
} // namespace mystl