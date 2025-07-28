#include <iostream>   // For std::cout, std::endl
#include <thread>     // For std::thread, std::this_thread
#include <vector>     // For std::vector
#include <string>     // For std::string, std::to_string (used in MyTestObject)
#include <chrono>     // For std::chrono::milliseconds, used for sleep (if needed)
#include <ctime>      // For clock_t and clock() for timing
#include <cstdio>     // For printf
#include <utility>    // For std::forward (used in newElement)
#include <stdexcept>  // For std::runtime_error (for better error handling in newElement)

// 包含你的 SimpleMemoryPool 头文件
#include "../include/HashBucketMemoryPool/SimpleMemoryPool.h"

// -------------------------------------------------------------------
// 测试用例类：不同大小的对象
// -------------------------------------------------------------------
class P1 {
public:
    int id_;
    P1(int id = 0) : id_(id) {}
    // ~P1() { std::cout << "P1 dtor " << id_ << std::endl; } // 可以 uncomment 用于调试生命周期
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
// 内存池辅助函数 (与你的 SimpleMemoryPool::allocator::construct/destroy 类似)
// -------------------------------------------------------------------
namespace SimpleMemoryPool { // 假设 SimpleMemoryPool 也在 SimpleMemoryPool 命名空间内

    // newElement: 从内存池分配内存并在其上构造对象
    template<typename T, typename... Args>
    T* newElement(SimpleMemoryPool& pool, Args&&... args)
    {
        void* p_raw_memory = pool.allocate();
        if (p_raw_memory == nullptr) {
            throw std::runtime_error("Failed to allocate memory from SimpleMemoryPool.");
        }
        // placement new: 在分配的内存上构造对象
        T* p_object = new(p_raw_memory) T(std::forward<Args>(args)...);
        return p_object;
    }

    // deleteElement: 析构对象并将其内存归还给内存池
    template<typename T>
    void deleteElement(SimpleMemoryPool& pool, T* p)
    {
        if (p == nullptr) return;
        p->~T(); // 显式调用对象的析构函数
        pool.deallocate(static_cast<void*>(p));
    }

} // namespace SimpleMemoryPool


// -------------------------------------------------------------------
// 基准测试函数：用于 SimpleMemoryPool
// -------------------------------------------------------------------
void BenchmarkSimpleMemoryPool(size_t ntimes, size_t nworks, size_t rounds)
{
    // SimpleMemoryPool 是单线程的，所以 nworks 必须为 1
    if (nworks > 1) {
        std::cerr << "Warning: SimpleMemoryPool is NOT thread-safe. Running with nworks=1." << std::endl;
        nworks = 1;
    }

    // 创建 SimpleMemoryPool 实例。假设槽大小为最大测试对象 P4 的大小
    // 你也可以根据需要创建多个 SimpleMemoryPool 实例来模拟 HashBucket (但这里是单池)
    SimpleMemoryPool::SimpleMemoryPool pool(4096, sizeof(P4));

    std::vector<std::thread> vthread(nworks); // 线程池 (实际上只有一个线程)
    size_t total_costtime = 0;

    for (size_t k = 0; k < nworks; ++k) // 创建 nworks 个线程
    {
        vthread[k] = std::thread([&]() { // Lambda 表达式捕获 pool 引用
            for (size_t j = 0; j < rounds; ++j) {
                clock_t begin1 = clock(); // 使用 clock_t, clock()
                for (size_t i = 0; i < ntimes; i++) {
                    // 使用 SimpleMemoryPool 接口分配和释放不同大小的对象
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
                total_costtime += (end1 - begin1); // 累加时间
            }
            });
    }

    for (auto& t : vthread) {
        t.join(); // 等待所有线程完成
    }

    // 打印结果
    printf("%lu个线程并发执行%lu轮次，每轮次newElement&deleteElement %lu次 (SimpleMemoryPool)，总计花费：%lu ms\n",
        nworks, rounds, ntimes, total_costtime);
}

// -------------------------------------------------------------------
// 基准测试函数：用于标准 new/delete
// -------------------------------------------------------------------
void BenchmarkNewDelete(size_t ntimes, size_t nworks, size_t rounds)
{
    std::vector<std::thread> vthread(nworks); // 线程池
    size_t total_costtime = 0;

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
        nworks, rounds, ntimes, total_costtime);
}


int test_simple_memory_pool_main()
{
    std::cout << "--- Starting Memory Pool Benchmarks ---" << std::endl;
    std::cout << "Note: SimpleMemoryPool is single-threaded. 'nworks' should be 1 for accurate comparison." << std::endl;

    // 参数：单轮次申请释放次数, 线程数, 轮次
    // 测试 SimpleMemoryPool (nworks 必须为 1)
    BenchmarkSimpleMemoryPool(1000, 1, 10000);

    std::cout << "===========================================================================" << std::endl;

    // 测试标准 new/delete (可以使用多个线程，但这里为了对比保持 nworks=1)
    BenchmarkNewDelete(1000, 1, 10000);

    std::cout << "--- Benchmarks Finished ---" << std::endl;

    return 0;
}