#pragma once
#include <vector>
#include <iostream>
#include <cassert>
#include "../include/vector.h"

namespace mystl {
    namespace test {
        namespace vector_test {

            void vector_test() {
                std::cout << "[===============================================================]\n";
                std::cout << "[----------------- Run container test : vector -----------------]\n";
                std::cout << "[-------------------------- API test ---------------------------]\n";

                // 测试构造函数系列
                {
                    std::cout << "*** Testing constructors ***\n";
                    mystl::vector<int> v1;  // 默认构造
                    mystl::vector<int> v2(10);  // 填充构造（默认值）
                    mystl::vector<int> v3(5, 42);  // 填充构造
                    int arr[] = { 1,3,5,7,9 };
                    mystl::vector<int> v4(arr, arr + 5);  // 迭代器构造
                    mystl::vector<int> v5(v4);  // 拷贝构造
                    mystl::vector<int> v6(std::move(v5));  // 移动构造
                    mystl::vector<int> v7{ 2,4,6,8 };  // 初始化列表构造

                    assert(v1.empty());
                    assert(v2.size() == 10);
                    assert(v3.back() == 42);
                    assert(v4[2] == 5);
                    assert(v5.empty());  // 移动后原对象应为空
                    assert(v6.size() == 5);
                    assert(v7.front() == 2);
                }

                // 测试赋值操作系列
                {
                    std::cout << "\n*** Testing assignments ***\n";
                    mystl::vector<int> src{ 1,2,3 };
                    mystl::vector<int> v1;

                    // 复制赋值
                    v1 = src;
                    assert(v1.size() == 3);
                    assert(v1.capacity() >= 3);

                    // 移动赋值
                    mystl::vector<int> v2;
                    v2 = std::move(src);
                    assert(v2.back() == 3);
                    assert(src.empty());

                    // 初始化列表赋值
                    v1 = { 4,5,6,7 };
                    assert(v1.size() == 4);
                    assert(v1[3] == 7);
                }

                // 测试元素访问系列
                {
                    std::cout << "\n*** Testing element access ***\n";
                    mystl::vector<int> v{ 10,20,30,40 };

                    // 边界检查
                    try {
                        v.at(5) = 50;  // 应抛出异常
                        assert(false);
                    }
                    catch (const std::out_of_range& e) {
                        assert(true);
                    }

                    // 数据指针操作
                    int* p = v.data();
                    assert(*p == 10);
                    p[2] = 33;
                    assert(v[2] == 33);

                    // 反向迭代器
                    assert(*v.rbegin() == 40);
                    assert(*(v.rend() - 1) == 10);
                }

                // 测试修改器系列
                {
                    std::cout << "\n*** Testing modifiers ***\n";
                    mystl::vector<int> v;

                    // 顺序插入
                    v.push_back(1);
                    v.emplace_back(2);
                    v.insert(v.end(), 3);
                    assert(v.back() == 3);

                    // 中间插入
                    v.insert(v.begin() + 1, 2, 5);  // 插入两个5
                    assert(v.size() == 5);
                    assert(v[1] == 5 && v[2] == 5);

                    // 删除操作
                    v.erase(v.begin() + 1);
                    assert(v.size() == 4);

                    // 清空操作
                    v.clear();
                    assert(v.empty());
                    assert(v.capacity() > 0);  // clear不释放内存

                    // 边界插入
                    v.reserve(10);
                    v.insert(v.begin(), 5, 42);
                    assert(v.capacity() >= 10);
                }

                // 测试容量管理系列
                {
                    std::cout << "\n*** Testing capacity management ***\n";
                    mystl::vector<int> v;

                    // 容量扩展
                    for (int i = 0; i < 100; ++i) {
                        v.push_back(i);
                    }
                    assert(v.size() == 100);
                    assert(v.capacity() >= 100);

                    // 缩减容量
                    v.shrink_to_fit();
                    assert(v.capacity() == v.size());

                    // 重置大小
                    v.resize(50);
                    assert(v.size() == 50);
                    assert(v.back() == 49); // 最后一个元素应为原第50个元素

                    v.resize(60, 42);
                    assert(v[49] == 49);    // 原元素保持不变
                    assert(v[59] == 42);    // 新增元素初始化正确
                }

                std::cout << "[------------------ End container test : vector -----------------]\n";
                std::cout << "[===============================================================]\n\n";
            }

        } // namespace vector_test
    } // namespace test
} // namespace mystl
