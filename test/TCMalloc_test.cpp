#include <iostream>
#include <thread>
#include <string>
#include <chrono>
#include <cstdio>
#include <atomic>

#include "../include/vector.h"

// ���������ڴ��ͷ�ļ�
#include "../include/TCMalloc/ThreadCache.h"
#include "../include/TCMalloc/CentralCache.h"
#include "../include/TCMalloc/PageCache.h"
#include "../include/TCMalloc/TCMallocutils.h"
#include<vector>


// -------------------------------------------------------------------
// 1. ģ�����Ĵ��룬����ģ�廯�� new/delete �ӿ�
// -------------------------------------------------------------------

// ģ�廯�� new�����ڴ������ڴ�ط���͹������
template<typename T, typename... Args>
T* my_new(Args&&... args) {
    // �� thread_cache �����ڴ�
    size_t size = sizeof(T);
    void* p = mystl::thread_cache::get_instance().allocate(size).value_or(nullptr);
    if (!p) {
        throw std::bad_alloc();
    }
    // ʹ�� placement new ���ѷ�����ڴ��Ϲ������
    return new(p) T(std::forward<Args>(args)...);
}

// ģ�廯�� delete�������������ͷŶ���������ڴ��
template<typename T>
void my_delete(T* p) {
    if (!p) {
        return;
    }
    size_t size = sizeof(T);
    // ��ʽ������������
    p->~T();
    // �黹�ڴ�� thread_cache
    mystl::thread_cache::get_instance().deallocate(p, size);
}


// -------------------------------------------------------------------
// 2. ���������ࣺ��ͬ��С�Ķ��� (������ʾ����ȫ��ͬ)
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

// ģ��һ������ ThreadCache ֱ�Ӵ������� (16K) �Ĵ����
class P_Large {
public:
    char data[mystl::size_utils::MAX_CACHED_UNIT_SIZE + 1]; // ����16KB
    P_Large(int id = 0) { data[0] = id; }
};

// -------------------------------------------------------------------
// 3. ��׼���Ժ������������� TCMalloc-like �ڴ��
// -------------------------------------------------------------------
void BenchmarkTCMalloc(size_t ntimes, size_t nworks, size_t rounds)
{
    mystl::vector<std::thread> vthread(nworks);
    std::atomic<size_t> total_costtime = 0;

    for (size_t k = 0; k < nworks; ++k)
    {
        vthread[k] = std::thread([&]() {
            size_t thread_cost_time = 0;
            mystl::vector<void*> p_vec; // ���ڴ洢�����ָ�룬��ģ�����ʵ�ĸ���
            p_vec.reserve(ntimes * 4);

            for (size_t j = 0; j < rounds; ++j) {
                clock_t begin1 = clock();
                for (size_t i = 0; i < ntimes; i++) {
                    // ʹ�� my_new / my_delete �ӿ�
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

    // ��ӡ���
    printf("%zu���̲߳���ִ��%zu�ִΣ�ÿ�ִ�my_new/my_delete %zu�� (TCMalloc-like)���ܼƻ��ѣ�%zu ms\n",
        nworks, rounds, ntimes, total_costtime.load());
}

// -------------------------------------------------------------------
// 4. ��׼���Ժ��������ڱ�׼ new/delete
// -------------------------------------------------------------------
extern void BenchmarkNewDelete(size_t ntimes, size_t nworks, size_t rounds);

int test_TCMalloc_main()
{
    // ������Բ���
    const size_t ntimes = 20;  // ÿ�ֲ�������
    const size_t nworks = 1;      // �����߳���
    const size_t rounds = 10;     // ���ִ�

    std::cout << "--- ��ʼ�ڴ�ػ�׼���� ---" << std::endl;
    printf("���Բ���: �߳���=%zu, �ִ�=%zu, ÿ�ֲ�����=%zu\n\n", nworks, rounds, ntimes * 4);

    // �������б�׼ new/delete �Ļ�׼
    BenchmarkNewDelete(ntimes, nworks, rounds);

    // �������� TCMalloc-like �ڴ�صĻ�׼
    // ע��: �����ڴ��ʵ��ͨ�� thread_local static ʵ���Զ���ʼ����������ʾ��������������ʽ�� Init ������
    BenchmarkTCMalloc(ntimes, nworks, rounds);

    std::cout << "\n--- ��׼���Խ��� ---" << std::endl;

    
    mystl::page_cache::get_instance().stop();

    return 0;
}