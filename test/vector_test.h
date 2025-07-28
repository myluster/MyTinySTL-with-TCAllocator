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

                // ���Թ��캯��ϵ��
                {
                    std::cout << "*** Testing constructors ***\n";
                    mystl::vector<int> v1;  // Ĭ�Ϲ���
                    mystl::vector<int> v2(10);  // ��乹�죨Ĭ��ֵ��
                    mystl::vector<int> v3(5, 42);  // ��乹��
                    int arr[] = { 1,3,5,7,9 };
                    mystl::vector<int> v4(arr, arr + 5);  // ����������
                    mystl::vector<int> v5(v4);  // ��������
                    mystl::vector<int> v6(std::move(v5));  // �ƶ�����
                    mystl::vector<int> v7{ 2,4,6,8 };  // ��ʼ���б���

                    assert(v1.empty());
                    assert(v2.size() == 10);
                    assert(v3.back() == 42);
                    assert(v4[2] == 5);
                    assert(v5.empty());  // �ƶ���ԭ����ӦΪ��
                    assert(v6.size() == 5);
                    assert(v7.front() == 2);
                }

                // ���Ը�ֵ����ϵ��
                {
                    std::cout << "\n*** Testing assignments ***\n";
                    mystl::vector<int> src{ 1,2,3 };
                    mystl::vector<int> v1;

                    // ���Ƹ�ֵ
                    v1 = src;
                    assert(v1.size() == 3);
                    assert(v1.capacity() >= 3);

                    // �ƶ���ֵ
                    mystl::vector<int> v2;
                    v2 = std::move(src);
                    assert(v2.back() == 3);
                    assert(src.empty());

                    // ��ʼ���б�ֵ
                    v1 = { 4,5,6,7 };
                    assert(v1.size() == 4);
                    assert(v1[3] == 7);
                }

                // ����Ԫ�ط���ϵ��
                {
                    std::cout << "\n*** Testing element access ***\n";
                    mystl::vector<int> v{ 10,20,30,40 };

                    // �߽���
                    try {
                        v.at(5) = 50;  // Ӧ�׳��쳣
                        assert(false);
                    }
                    catch (const std::out_of_range& e) {
                        assert(true);
                    }

                    // ����ָ�����
                    int* p = v.data();
                    assert(*p == 10);
                    p[2] = 33;
                    assert(v[2] == 33);

                    // ���������
                    assert(*v.rbegin() == 40);
                    assert(*(v.rend() - 1) == 10);
                }

                // �����޸���ϵ��
                {
                    std::cout << "\n*** Testing modifiers ***\n";
                    mystl::vector<int> v;

                    // ˳�����
                    v.push_back(1);
                    v.emplace_back(2);
                    v.insert(v.end(), 3);
                    assert(v.back() == 3);

                    // �м����
                    v.insert(v.begin() + 1, 2, 5);  // ��������5
                    assert(v.size() == 5);
                    assert(v[1] == 5 && v[2] == 5);

                    // ɾ������
                    v.erase(v.begin() + 1);
                    assert(v.size() == 4);

                    // ��ղ���
                    v.clear();
                    assert(v.empty());
                    assert(v.capacity() > 0);  // clear���ͷ��ڴ�

                    // �߽����
                    v.reserve(10);
                    v.insert(v.begin(), 5, 42);
                    assert(v.capacity() >= 10);
                }

                // ������������ϵ��
                {
                    std::cout << "\n*** Testing capacity management ***\n";
                    mystl::vector<int> v;

                    // ������չ
                    for (int i = 0; i < 100; ++i) {
                        v.push_back(i);
                    }
                    assert(v.size() == 100);
                    assert(v.capacity() >= 100);

                    // ��������
                    v.shrink_to_fit();
                    assert(v.capacity() == v.size());

                    // ���ô�С
                    v.resize(50);
                    assert(v.size() == 50);
                    assert(v.back() == 49); // ���һ��Ԫ��ӦΪԭ��50��Ԫ��

                    v.resize(60, 42);
                    assert(v[49] == 49);    // ԭԪ�ر��ֲ���
                    assert(v[59] == 42);    // ����Ԫ�س�ʼ����ȷ
                }

                std::cout << "[------------------ End container test : vector -----------------]\n";
                std::cout << "[===============================================================]\n\n";
            }

        } // namespace vector_test
    } // namespace test
} // namespace mystl
