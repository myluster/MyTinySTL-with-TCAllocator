#include <iostream>   // For std::cout, std::endl, std::cerr
#include <thread>     // For std::thread, std::this_thread
#include <vector>     // For std::vector
#include <string>     // For std::string, std::to_string
#include <chrono>     // For std::chrono::milliseconds
#include <ctime>      // For clock_t and clock() for timing
#include <cstdio>     // For printf
#include <utility>    // For std::forward

#include "../include/HashBucketMemoryPool/MutexMemoryPool.h"

// 使用你的命名空间
using namespace MutexMemoryPool;

// -------------------------------------------------------------------
// 测试用例类：不同大小的对象
// -------------------------------------------------------------------
class P1 {
public:
    int id_; // 4 bytes
    P1(int id = 0) : id_(id) {}
    // ~P1() { std::cout << "P1 dtor " << id_ << std::endl; } // 可以 uncomment 用于调试生命周期
};

class P2 {
public:
    int id_[5]; // 20 bytes (assuming int is 4 bytes)
    P2(int id = 0) { id_[0] = id; }
    // ~P2() { std::cout << "P2 dtor " << id_[0] << std::endl; }
};

class P3 {
public:
    int id_[10]; // 40 bytes
    P3(int id = 0) { id_[0] = id; }
    // ~P3() { std::cout << "P3 dtor " << id_[0] << std::endl; }
};

class P4 {
public:
    int id_[20]; // 80 bytes
    P4(int id = 0) { id_[0] = id; }
    // ~P4() { std::cout << "P4 dtor " << id_[0] << std::endl; }
};

class P_Large { // 模拟大于 MAX_SLOT_SIZE (512字节) 的对象
public:
    int data[200]; // 800 bytes
    P_Large(int id = 0) { data[0] = id; }
    // ~P_Large() { std::cout << "P_Large dtor " << data[0] << std::endl; }
};

// -------------------------------------------------------------------
// 基准测试函数：用于 HashBucketMemoryPool (你的多尺寸内存池)
// -------------------------------------------------------------------
void BenchmarkMutexMemoryPool(size_t ntimes, size_t nworks, size_t rounds)
{

    std::vector<std::thread> vthread(nworks);//线程池
    std::atomic<size_t> total_costtime = 0;

    for (size_t k = 0; k < nworks; ++k) // 创建 nworks 个线程
    {
        vthread[k] = std::thread([&]() { // Lambda 表达式捕获外部变量
            clock_t begin1;
            clock_t end1;
            size_t thread_cost_time = 0; // 每个线程自己的计时器

            for (size_t j = 0; j < rounds; ++j) {
                clock_t begin1 = clock(); // 使用 clock_t, clock()
                for (size_t i = 0; i < ntimes; i++) {
                    // 使用 HashBucketMemoryPool 接口分配和释放不同大小的对象
                    P1* p1 = newElement<P1>(i);
                    deleteElement<P1>(p1);
                    P2* p2 = newElement<P2>(i);
                    deleteElement<P2>(p2);
                    P3* p3 = newElement<P3>(i);
                    deleteElement<P3>(p3);
                    P4* p4 = newElement<P4>(i);
                    deleteElement<P4>(p4);
                    // 测试大于 MAX_SLOT_SIZE 的对象，会回退到 malloc/free
                    P_Large* p_large = newElement<P_Large>(i);
                    deleteElement<P_Large>(p_large);
                }
                end1 = clock();
                thread_cost_time += (end1 - begin1);
            }
            total_costtime += thread_cost_time; // std::atomic 的 += 是线程安全的
            });
    }

    for (auto& t : vthread) {
        t.join(); // 等待所有线程完成
    }

    // 打印结果
    printf("%zu个线程并发执行%zu轮次，每轮次newElement&deleteElement %zu次 (HashBucketMemoryPool)，总计花费：%zu ms\n",
        nworks, rounds, ntimes, total_costtime.load());
}

// -------------------------------------------------------------------
// 基准测试函数：用于标准 new/delete
// -------------------------------------------------------------------
void BenchmarkMutexNewDelete(size_t ntimes, size_t nworks, size_t rounds)
{
    std::vector<std::thread> vthread(nworks); // 线程池
    std::atomic<size_t> total_costtime = 0; 

    for (size_t k = 0; k < nworks; ++k) // 创建 nworks 个线程
    {
        vthread[k] = std::thread([&]() {
            for (size_t j = 0; j < rounds; ++j) {
                clock_t begin1 = clock();
                for (size_t i = 0; i < ntimes; i++) {
                    // 使用标准 new/delete
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

    printf("%lu个线程并发执行%lu轮次，每轮次new/delete %lu次 (Standard)，总计花费：%lu ms\n",
        nworks, rounds, ntimes, total_costtime.load());
}

// -------------------------------------------------------------------
// Main 函数
// -------------------------------------------------------------------
int test_mutex_main()
{
    std::cout << "--- Starting Memory Pool Benchmarks ---" << std::endl;

    BenchmarkMutexNewDelete(1000, 100, 100);

    std::cout << "--- Benchmarks Finished ---" << std::endl;

    // === 关键：初始化 HashBucketMemoryPool 系统 ===
    HashBucket::initMemoryPools();

    // 参数：单轮次申请释放次数, 线程数, 轮次
    BenchmarkMutexMemoryPool(1000, 100, 100);

    std::cout << "===========================================================================" << std::endl;

    return 0;
}