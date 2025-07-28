#include <iostream>   // For std::cout, std::endl
#include <thread>     // For std::thread, std::this_thread
#include <vector>     // For std::vector
#include <string>     // For std::string, std::to_string (used in MyTestObject)
#include <chrono>     // For std::chrono::milliseconds, used for sleep (if needed)
#include <ctime>      // For clock_t and clock() for timing
#include <cstdio>     // For printf
#include <utility>    // For std::forward (used in newElement)
#include <stdexcept>  // For std::runtime_error (for better error handling in newElement)

// ������� SimpleMemoryPool ͷ�ļ�
#include "../include/HashBucketMemoryPool/SimpleMemoryPool.h"

// -------------------------------------------------------------------
// ���������ࣺ��ͬ��С�Ķ���
// -------------------------------------------------------------------
class P1 {
public:
    int id_;
    P1(int id = 0) : id_(id) {}
    // ~P1() { std::cout << "P1 dtor " << id_ << std::endl; } // ���� uncomment ���ڵ�����������
};

class P2 {
public:
    int id_[5];
    P2(int id = 0) { id_[0] = id; }
    // ~P2() { std::cout << "P2 dtor " << id_[0] << std::endl; }
};

class P3 {
public:
    int id_[10];
    P3(int id = 0) { id_[0] = id; }
    // ~P3() { std::cout << "P3 dtor " << id_[0] << std::endl; }
};

class P4 {
public:
    int id_[20];
    P4(int id = 0) { id_[0] = id; }
    // ~P4() { std::cout << "P4 dtor " << id_[0] << std::endl; }
};

// -------------------------------------------------------------------
// �ڴ�ظ������� (����� SimpleMemoryPool::allocator::construct/destroy ����)
// -------------------------------------------------------------------
namespace SimpleMemoryPool { // ���� SimpleMemoryPool Ҳ�� SimpleMemoryPool �����ռ���

    // newElement: ���ڴ�ط����ڴ沢�����Ϲ������
    template<typename T, typename... Args>
    T* newElement(SimpleMemoryPool& pool, Args&&... args)
    {
        void* p_raw_memory = pool.allocate();
        if (p_raw_memory == nullptr) {
            throw std::runtime_error("Failed to allocate memory from SimpleMemoryPool.");
        }
        // placement new: �ڷ�����ڴ��Ϲ������
        T* p_object = new(p_raw_memory) T(std::forward<Args>(args)...);
        return p_object;
    }

    // deleteElement: �������󲢽����ڴ�黹���ڴ��
    template<typename T>
    void deleteElement(SimpleMemoryPool& pool, T* p)
    {
        if (p == nullptr) return;
        p->~T(); // ��ʽ���ö������������
        pool.deallocate(static_cast<void*>(p));
    }

} // namespace SimpleMemoryPool


// -------------------------------------------------------------------
// ��׼���Ժ��������� SimpleMemoryPool
// -------------------------------------------------------------------
void BenchmarkSimpleMemoryPool(size_t ntimes, size_t nworks, size_t rounds)
{
    // SimpleMemoryPool �ǵ��̵߳ģ����� nworks ����Ϊ 1
    if (nworks > 1) {
        std::cerr << "Warning: SimpleMemoryPool is NOT thread-safe. Running with nworks=1." << std::endl;
        nworks = 1;
    }

    // ���� SimpleMemoryPool ʵ��������۴�СΪ�����Զ��� P4 �Ĵ�С
    // ��Ҳ���Ը�����Ҫ������� SimpleMemoryPool ʵ����ģ�� HashBucket (�������ǵ���)
    SimpleMemoryPool::SimpleMemoryPool pool(4096, sizeof(P4));

    std::vector<std::thread> vthread(nworks); // �̳߳� (ʵ����ֻ��һ���߳�)
    size_t total_costtime = 0;

    for (size_t k = 0; k < nworks; ++k) // ���� nworks ���߳�
    {
        vthread[k] = std::thread([&]() { // Lambda ���ʽ���� pool ����
            for (size_t j = 0; j < rounds; ++j) {
                clock_t begin1 = clock(); // ʹ�� clock_t, clock()
                for (size_t i = 0; i < ntimes; i++) {
                    // ʹ�� SimpleMemoryPool �ӿڷ�����ͷŲ�ͬ��С�Ķ���
                    P1* p1 = SimpleMemoryPool::newElement<P1>(pool, i);
                    SimpleMemoryPool::deleteElement<P1>(pool, p1);
                    P2* p2 = SimpleMemoryPool::newElement<P2>(pool, i);
                    SimpleMemoryPool::deleteElement<P2>(pool, p2);
                    P3* p3 = SimpleMemoryPool::newElement<P3>(pool, i);
                    SimpleMemoryPool::deleteElement<P3>(pool, p3);
                    P4* p4 = SimpleMemoryPool::newElement<P4>(pool, i);
                    SimpleMemoryPool::deleteElement<P4>(pool, p4);
                }
                clock_t end1 = clock();
                total_costtime += (end1 - begin1); // �ۼ�ʱ��
            }
            });
    }

    for (auto& t : vthread) {
        t.join(); // �ȴ������߳����
    }

    // ��ӡ���
    printf("%lu���̲߳���ִ��%lu�ִΣ�ÿ�ִ�newElement&deleteElement %lu�� (SimpleMemoryPool)���ܼƻ��ѣ�%lu ms\n",
        nworks, rounds, ntimes, total_costtime);
}

// -------------------------------------------------------------------
// ��׼���Ժ��������ڱ�׼ new/delete
// -------------------------------------------------------------------
void BenchmarkNewDelete(size_t ntimes, size_t nworks, size_t rounds)
{
    std::vector<std::thread> vthread(nworks); // �̳߳�
    size_t total_costtime = 0;

    for (size_t k = 0; k < nworks; ++k) // ���� nworks ���߳�
    {
        vthread[k] = std::thread([&]() {
            for (size_t j = 0; j < rounds; ++j) {
                clock_t begin1 = clock();
                for (size_t i = 0; i < ntimes; i++) {
                    // ʹ�ñ�׼ new/delete
                    P1* p1 = new P1;
                    delete p1;
                    P2* p2 = new P2;
                    delete p2;
                    P3* p3 = new P3;
                    delete p3;
                    P4* p4 = new P4;
                    delete p4;
                }
                clock_t end1 = clock();
                total_costtime += (end1 - begin1);
            }
            });
    }

    for (auto& t : vthread) {
        t.join();
    }

    printf("%lu���̲߳���ִ��%lu�ִΣ�ÿ�ִ�new/delete %lu�� (Standard)���ܼƻ��ѣ�%lu ms\n",
        nworks, rounds, ntimes, total_costtime);
}


int test_simple_memory_pool_main()
{
    std::cout << "--- Starting Memory Pool Benchmarks ---" << std::endl;
    std::cout << "Note: SimpleMemoryPool is single-threaded. 'nworks' should be 1 for accurate comparison." << std::endl;

    // ���������ִ������ͷŴ���, �߳���, �ִ�
    // ���� SimpleMemoryPool (nworks ����Ϊ 1)
    BenchmarkSimpleMemoryPool(1000, 1, 10000);

    std::cout << "===========================================================================" << std::endl;

    // ���Ա�׼ new/delete (����ʹ�ö���̣߳�������Ϊ�˶Աȱ��� nworks=1)
    BenchmarkNewDelete(1000, 1, 10000);

    std::cout << "--- Benchmarks Finished ---" << std::endl;

    return 0;
}