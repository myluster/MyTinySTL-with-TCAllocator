#undef value
#include <iostream>
#include "test/vector_test.h"
#include "test/map_test.h"
#include "test/set_test.h"
#include "test/list_test.h"
#include "test/unordered_map_test.h"
// 声明其他测试文件中的函数
int test_hash_bucket_main();
int test_simple_memory_pool_main();
int test_mutex_main();
int test_LockFree_main();
int test_TCMalloc_main();

int main() {
    //std::cout << "Running Combined Memory Pool Tests..." << std::endl;

    //std::cout << "\n--- Running HashBucketMemoryPool Test ---" << std::endl;
    //test_hash_bucket_main(); // 调用 HashBucket 的测试

    //std::cout << "\n--- Running SimpleMemoryPool Test ---" << std::endl;
    //test_simple_memory_pool_main(); // 调用 SimpleMemoryPool 的测试

    //std::cout << "\n--- Running MutexMemoryPool Test ---" << std::endl;
    //test_mutex_main();

    //std::cout << "\n--- Running LockFreeMemoryPool Test ---" << std::endl;
    //test_LockFree_main();

   mystl::test::vector_test::vector_test();

    //mystl::test::map_test::map_test();
    //mystl::test::map_test::multimap_test();

    //mystl::test::set_test::set_test();
    //mystl::test::set_test::multiset_test();

    //mystl::test::list_test::list_test();

    //mystl::test::unordered_map_test::unordered_map_test();

    //std::cout << "\n--- Running TCMalloc Test ---" << std::endl;
    //test_TCMalloc_main();
    std::cout << "\nAll Tests Finished." << std::endl;
    return 0;
}