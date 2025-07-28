#include <iostream>
#include <thread>
#include <string>
#include <chrono>
#include <cstdio>
#include <atomic>

#include "../include/vector.h"

// 包含您的内存池头文件
#include "../include/TCMalloc/ThreadCache.h"
#include "../include/TCMalloc/CentralCache.h"
#include "../include/TCMalloc/PageCache.h"
#include "../include/TCMalloc/TCMallocutils.h"
#include<vector>


// -------------------------------------------------------------------
// 1. 模仿您的代码，创建模板化的 new/delete 接口
// -------------------------------------------------------------------

// 模板化的 new，用于从您的内存池分配和构造对象
template<typename T, typename... Args>
T* my_new(Args&&... args) {
    // 从 thread_cache 分配内存
    size_t size = sizeof(T);
    void* p = mystl::thread_cache::get_instance().allocate(size).value_or(nullptr);
    if (!p) {
        throw std::bad_alloc();
    }
    // 使用 placement new 在已分配的内存上构造对象
    return new(p) T(std::forward<Args>(args)...);
}

// 模板化的 delete，用于析构和释放对象回您的内存池
template<typename T>
void my_delete(T* p) {
    if (!p) {
        return;
    }
    size_t size = sizeof(T);
    // 显式调用析构函数
    p->~T();
    // 归还内存给 thread_cache
    mystl::thread_cache::get_instance().deallocate(p, size);
}


// -------------------------------------------------------------------
// 2. 测试用例类：不同大小的对象 (与您的示例完全相同)
// -------------------------------------------------------------------
class P1 {
public:
    int id_; // 4 bytes or 8 bytes
    P1(int id = 0) : id_(id) {}
};

class P2 {
public:
    int id_[5]; // 20 bytes
    P2(int id = 0) { id_[0] = id; }
};

class P3 {
public:
    int id_[10]; // 40 bytes
    P3(int id = 0) { id_[0] = id; }
};

class P4 {
public:
    int id_[20]; // 80 bytes
    P4(int id = 0) { id_[0] = id; }
};

// 模拟一个大于 ThreadCache 直接处理上限 (16K) 的大对象
class P_Large {
public:
    char data[mystl::size_utils::MAX_CACHED_UNIT_SIZE + 1]; // 大于16KB
    P_Large(int id = 0) { data[0] = id; }
};

// -------------------------------------------------------------------
// 3. 基准测试函数：用于您的 TCMalloc-like 内存池
// -------------------------------------------------------------------
void BenchmarkTCMalloc(size_t ntimes, size_t nworks, size_t rounds)
{
    mystl::vector<std::thread> vthread(nworks);
    std::atomic<size_t> total_costtime = 0;

    for (size_t k = 0; k < nworks; ++k)
    {
        vthread[k] = std::thread([&]() {
            size_t thread_cost_time = 0;
            mystl::vector<void*> p_vec; // 用于存储分配的指针，以模拟更真实的负载
            p_vec.reserve(ntimes * 4);

            for (size_t j = 0; j < rounds; ++j) {
                clock_t begin1 = clock();
                for (size_t i = 0; i < ntimes; i++) {
                    // 使用 my_new / my_delete 接口
                    my_delete(my_new<P1>(i));
                    my_delete(my_new<P2>(i));
                    my_delete(my_new<P3>(i));
                    my_delete(my_new<P4>(i));
                }
                clock_t end1 = clock();
                thread_cost_time += (end1 - begin1);
            }
            total_costtime += thread_cost_time;
            });
    }

    for (auto& t : vthread) {
        t.join();
    }

    // 打印结果
    printf("%zu个线程并发执行%zu轮次，每轮次my_new/my_delete %zu次 (TCMalloc-like)，总计花费：%zu ms\n",
        nworks, rounds, ntimes, total_costtime.load());
}

// -------------------------------------------------------------------
// 4. 基准测试函数：用于标准 new/delete
// -------------------------------------------------------------------
extern void BenchmarkNewDelete(size_t ntimes, size_t nworks, size_t rounds);

int test_TCMalloc_main()
{
    // 定义测试参数
    const size_t ntimes = 20;  // 每轮操作次数
    const size_t nworks = 1;      // 并发线程数
    const size_t rounds = 10;     // 总轮次

    std::cout << "--- 开始内存池基准测试 ---" << std::endl;
    printf("测试参数: 线程数=%zu, 轮次=%zu, 每轮操作数=%zu\n\n", nworks, rounds, ntimes * 4);

    // 首先运行标准 new/delete 的基准
    BenchmarkNewDelete(ntimes, nworks, rounds);

    // 运行您的 TCMalloc-like 内存池的基准
    // 注意: 您的内存池实现通过 thread_local static 实例自动初始化，无需像示例中那样调用显式的 Init 函数。
    BenchmarkTCMalloc(ntimes, nworks, rounds);

    std::cout << "\n--- 基准测试结束 ---" << std::endl;

    
    mystl::page_cache::get_instance().stop();

    return 0;
}