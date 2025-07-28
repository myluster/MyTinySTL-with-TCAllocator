#pragma once
#include <list>
#include <iostream>
#include <cassert>
#include <string>
#include <functional> // for std::greater
#include "../include/list.h" // 假设您的 list.h 在这个路径

namespace mystl {
    namespace test {
        namespace list_test {

            // 辅助函数，用于打印和比较 list 内容
            template <class T>
            void print_list(const mystl::list<T>& l, const std::string& name) {
                std::cout << name << " : [ ";
                for (const auto& item : l) {
                    std::cout << item << " ";
                }
                std::cout << "]\n";
            }

            void list_test() {
                std::cout << "[===============================================================]\n";
                std::cout << "[------------------- Run container test : list -----------------]\n";
                std::cout << "[-------------------------- API test ---------------------------]\n";

                // 1. 构造函数系列
                {
                    std::cout << "\n*** 1. Testing constructors ***\n";
                    int arr[] = { 1,3,5,7,9 };

                    mystl::list<int> l1;
                    assert(l1.empty() && l1.size() == 0);

                    mystl::list<int> l2(5);
                    assert(l2.size() == 5);

                    mystl::list<int> l3(5, 42);
                    assert(l3.size() == 5 && l3.front() == 42 && l3.back() == 42);

                    mystl::list<int> l4(arr, arr + 5);
                    assert(l4.size() == 5 && l4.front() == 1 && l4.back() == 9);

                    mystl::list<int> l5(l4);
                    assert(l5.size() == 5 && l5.front() == 1 && l5.back() == 9);

                    // 假设移动构造函数已修正为将被移动对象置为空状态
                    mystl::list<int> l6(std::move(l5));
                    assert(l6.size() == 5 && l6.front() == 1 && l6.back() == 9);
                    assert(l5.empty() && l5.size() == 0); // 验证被移动对象的状态

                    mystl::list<int> l7{ 2,4,6,8 };
                    assert(l7.size() == 4 && l7.front() == 2 && l7.back() == 8);
                    std::cout << "Constructors PASSED\n";
                }

                // 2. 赋值操作系列
                {
                    std::cout << "\n*** 2. Testing assignments ***\n";
                    mystl::list<int> src{ 1,2,3 };
                    mystl::list<int> l1, l2;

                    l1 = src;
                    assert(l1.size() == 3 && l1.back() == 3);

                    // 移动赋值后，源对象应为空
                    l2 = std::move(src);
                    assert(l2.size() == 3 && l2.back() == 3);
                    assert(src.empty());

                    l1 = { 4,5,6,7 };
                    assert(l1.size() == 4 && l1.back() == 7);

                    // 自我赋值
                    l1 = l1;
                    assert(l1.size() == 4 && l1.back() == 7);
                    std::cout << "Assignments PASSED\n";
                }

                // 3. 迭代器测试
                {
                    std::cout << "\n*** 3. Testing iterators ***\n";
                    mystl::list<int> l{ 1, 2, 3 };
                    const mystl::list<int> cl{ 1, 2, 3 };

                    assert(*l.begin() == 1);
                    assert(*l.cbegin() == 1);
                    assert(*cl.begin() == 1);

                    auto it = l.end();
                    --it;
                    assert(*it == 3);

                    assert(*l.rbegin() == 3);
                    assert(*l.crbegin() == 3);

                    auto rit = l.rend();
                    --rit;
                    assert(*rit == 1);

                    size_t n = 0;
                    for (auto i = l.begin(); i != l.end(); ++i, ++n);
                    assert(n == l.size());

                    std::cout << "Iterators PASSED\n";
                }

                // 4. 容量相关测试
                {
                    std::cout << "\n*** 4. Testing capacity ***\n";
                    mystl::list<int> l;
                    assert(l.empty());
                    assert(l.size() == 0);

                    l.push_back(1);
                    assert(!l.empty());
                    assert(l.size() == 1);

                    std::cout << "Capacity PASSED\n";
                }

                // 5. 修改器系列（重点补充）
                {
                    std::cout << "\n*** 5. Testing modifiers ***\n";

                    // push_front / pop_front (之前缺失的测试)
                    {
                        mystl::list<int> l;
                        l.push_front(10);
                        l.push_front(20);
                        assert(l.size() == 2 && l.front() == 20);
                        l.pop_front();
                        assert(l.size() == 1 && l.front() == 10);
                        l.pop_front();
                        assert(l.empty());
                        std::cout << "push_front / pop_front PASSED\n";
                    }

                    // push_back / pop_back (之前缺失的测试)
                    {
                        mystl::list<int> l;
                        l.push_back(10);
                        l.push_back(20);
                        assert(l.size() == 2 && l.back() == 20);
                        l.pop_back();
                        assert(l.size() == 1 && l.back() == 10);
                        l.pop_back();
                        assert(l.empty());
                        std::cout << "push_back / pop_back PASSED\n";
                    }

                    // insert
                    {
                        mystl::list<int> l{ 1, 5 };
                        auto it = l.begin();
                        ++it; // 指向 5
                        l.insert(it, 3); // 1, 3, 5
                        it = l.begin();
                        assert(*it++ == 1);
                        assert(*it++ == 3);
                        assert(*it == 5);

                        l.insert(l.end(), 2, 8); // 1, 3, 5, 8, 8
                        assert(l.back() == 8 && l.size() == 5);

                        int arr[] = { 6, 7 };
                        l.insert(l.end(), arr, arr + 2); // 1, 3, 5, 8, 8, 6, 7
                        assert(l.back() == 7 && l.size() == 7);
                        std::cout << "insert PASSED\n";
                    }

                    // erase
                    {
                        mystl::list<int> l{ 1,2,3,4,5 };
                        auto it = l.begin();
                        ++it; // 指向 2
                        it = l.erase(it); // 删除 2, it 指向 3
                        assert(*it == 3 && l.size() == 4);

                        auto it2 = l.end();
                        --it2; --it2; // 指向 4
                        it = l.erase(it, it2); // 删除 3, it 指向 4
                        assert(*it == 4 && l.size() == 3);
                        std::cout << "erase PASSED\n";
                    }

                    // resize
                    {
                        mystl::list<int> l{ 1,2,3,4,5 };
                        l.resize(3);
                        assert(l.size() == 3 && l.back() == 3);
                        l.resize(5, 42);
                        assert(l.size() == 5 && l.back() == 42);
                        std::cout << "resize PASSED\n";
                    }

                    // swap
                    {
                        mystl::list<int> l1{ 1, 2 };
                        mystl::list<int> l2{ 3, 4, 5 };
                        l1.swap(l2);
                        assert(l1.size() == 3 && l1.front() == 3);
                        assert(l2.size() == 2 && l2.front() == 1);
                        mystl::swap(l1, l2);
                        assert(l1.size() == 2 && l1.front() == 1);
                        assert(l2.size() == 3 && l2.front() == 3);
                        std::cout << "swap PASSED\n";
                    }
                }

                // 6. List 操作系列
                {
                    std::cout << "\n*** 6. Testing list operations ***\n";

                    // splice
                    {
                        mystl::list<int> l1{ 1, 5 };
                        mystl::list<int> l2{ 2, 3, 4 };
                        auto it = l1.begin();
                        ++it;
                        l1.splice(it, l2); // l1: 1, 2, 3, 4, 5
                        assert(l1.size() == 5 && l2.empty());
                        it = l1.begin();
                        ++it;
                        assert(*it == 2);
                        std::cout << "splice PASSED\n";
                    }

                    // remove / remove_if
                    {
                        mystl::list<int> l{ 1, 2, 3, 2, 4, 2 };
                        l.remove(2);
                        assert(l.size() == 3);
                        assert(l.front() == 1 && l.back() == 4);

                        l.remove_if([](int n) { return n > 2; }); // remove 3, 4
                        assert(l.size() == 1 && l.front() == 1);
                        std::cout << "remove / remove_if PASSED\n";
                    }

                    // unique
                    {
                        mystl::list<int> l{ 1, 1, 2, 3, 3, 3, 2, 2 };
                        l.unique(); // 1, 2, 3, 2
                        assert(l.size() == 4);
                        auto it = l.begin();
                        assert(*it++ == 1);
                        assert(*it++ == 2);
                        assert(*it++ == 3);
                        assert(*it++ == 2);
                        std::cout << "unique PASSED\n";
                    }

                    // merge
                    {
                        mystl::list<int> l1{ 1, 3, 5 };
                        mystl::list<int> l2{ 2, 4 };
                        l1.merge(l2);
                        assert(l1.size() == 5 && l2.empty());
                        auto it = l1.begin();
                        assert(*it++ == 1);
                        assert(*it++ == 2);
                        assert(*it++ == 3);
                        std::cout << "merge PASSED\n";
                    }

                    // sort
                    {
                        mystl::list<int> l{ 5, 2, 4, 1, 3 };
                        l.sort();
                        auto it = l.begin();
                        assert(*it++ == 1);
                        assert(*it++ == 2);
                        l.sort(std::greater<int>());
                        it = l.begin();
                        assert(*it++ == 5);
                        assert(*it++ == 4);
                        std::cout << "sort PASSED\n";
                    }

                    // reverse
                    {
                        mystl::list<int> l{ 1, 2, 3 };
                        l.reverse();
                        auto it = l.begin();
                        assert(*it++ == 3);
                        assert(*it++ == 2);
                        assert(*it++ == 1);
                        std::cout << "reverse PASSED\n";
                    }
                }

                std::cout << "\n[------------------ End container test : list ------------------]\n";
                std::cout << "[===============================================================]\n\n";
            }

        } // namespace list_test
    } // namespace test
} // namespace mystl