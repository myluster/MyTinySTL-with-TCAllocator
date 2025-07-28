#include <iostream>   // For std::cout, std::endl, std::cerr
#include <thread>     // For std::thread, std::this_thread
#include <vector>     // For std::vector
#include <string>     // For std::string, std::to_string
#include <chrono>     // For std::chrono::milliseconds
#include <ctime>      // For clock_t and clock() for timing
#include <cstdio>     // For printf
#include <utility>    // For std::forward

// ������Ĺ�ϣͰ�ڴ��ͷ�ļ�
#include "../include/HashBucketMemoryPool/HashBucketMemoryPool.h"

// ʹ����������ռ�
using namespace HashBucketMemoryPool;

// -------------------------------------------------------------------
// ���������ࣺ��ͬ��С�Ķ���
// -------------------------------------------------------------------
class P1 {
public:
    int id_; // 4 bytes
    P1(int id = 0) : id_(id) {}
    // ~P1() { std::cout << "P1 dtor " << id_ << std::endl; } // ���� uncomment ���ڵ�����������
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

class P_Large { // ģ����� MAX_SLOT_SIZE (512�ֽ�) �Ķ���
public:
    int data[200]; // 800 bytes
    P_Large(int id = 0) { data[0] = id; }
    // ~P_Large() { std::cout << "P_Large dtor " << data[0] << std::endl; }
};

// -------------------------------------------------------------------
// ��׼���Ժ��������� HashBucketMemoryPool (��Ķ�ߴ��ڴ��)
// -------------------------------------------------------------------
void BenchmarkHashBucketMemoryPool(size_t ntimes, size_t nworks, size_t rounds)
{
    // HashBucketMemoryPool �ڵ�ǰʵ�����ǵ��̰߳�ȫ�� (��Ϊ SimpleMemoryPool �ǵ��̵߳�)��
    // nworks ����Ϊ 1�����򽫵������ݾ�����
    if (nworks > 1) {
        std::cerr << "Warning: HashBucketMemoryPool is NOT fully thread-safe in this version. Running with nworks=1." << std::endl;
        nworks = 1;
    }

    std::vector<std::thread> vthread(nworks); // �̳߳� (ʵ����ֻ��һ���߳�)
    size_t total_costtime = 0;

    for (size_t k = 0; k < nworks; ++k) // ���� nworks ���߳�
    {
        vthread[k] = std::thread([&]() { // Lambda ���ʽ�����ⲿ����
            for (size_t j = 0; j < rounds; ++j) {
                clock_t begin1 = clock(); // ʹ�� clock_t, clock()
                for (size_t i = 0; i < ntimes; i++) {
                    // ʹ�� HashBucketMemoryPool �ӿڷ�����ͷŲ�ͬ��С�Ķ���
                    P1* p1 = newElement<P1>(i);
                    deleteElement<P1>(p1);
                    P2* p2 = newElement<P2>(i);
                    deleteElement<P2>(p2);
                    P3* p3 = newElement<P3>(i);
                    deleteElement<P3>(p3);
                    P4* p4 = newElement<P4>(i);
                    deleteElement<P4>(p4);
                    // ���Դ��� MAX_SLOT_SIZE �Ķ��󣬻���˵� malloc/free
                    P_Large* p_large = newElement<P_Large>(i); 
                    deleteElement<P_Large>(p_large);
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
    printf("%zu���̲߳���ִ��%zu�ִΣ�ÿ�ִ�newElement&deleteElement %zu�� (HashBucketMemoryPool)���ܼƻ��ѣ�%zu ms\n", 
           nworks, rounds, ntimes, total_costtime);
}

// -------------------------------------------------------------------
// ��׼���Ժ��������ڱ�׼ new/delete
// -------------------------------------------------------------------
extern void BenchmarkNewDelete(size_t ntimes, size_t nworks, size_t rounds);


// -------------------------------------------------------------------
// Main ����
// -------------------------------------------------------------------
int test_hash_bucket_main()
{
    std::cout << "--- Starting Memory Pool Benchmarks ---" << std::endl;
    std::cout << "Note: HashBucketMemoryPool is NOT thread-safe in this version. 'nworks' should be 1 for accurate comparison." << std::endl;

    // === �ؼ�����ʼ�� HashBucketMemoryPool ϵͳ ===
    HashBucket::initMemoryPools(); 

    // ���������ִ������ͷŴ���, �߳���, �ִ�
    // ���� HashBucketMemoryPool (nworks ����Ϊ 1)
    BenchmarkHashBucketMemoryPool(1000, 1, 10000);

    std::cout << "===========================================================================" << std::endl;
    
    // ���Ա�׼ new/delete (����ʹ�ö���̣߳�������Ϊ�˶Աȱ��� nworks=1)
    BenchmarkNewDelete(1000, 1, 10000);

    std::cout << "--- Benchmarks Finished ---" << std::endl;
    
    return 0;
}