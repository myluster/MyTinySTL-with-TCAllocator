#pragma once
#include <atomic>
#include <optional>
//#include "../list.h"
#include "../span.h"
#include "../byte.h"
//#include "../map.h"
#include "TCMallocutils.h"
namespace mystl
{
	//前向声明 list，因为 allocate() 的返回值需要它
	template<typename T> class list;

	//PIMPL实现类的前向声明
	class CentralCacheImpl;

	class central_cache
	{
	public:
		//一次性申请8页的空间
		static constexpr size_t PAGE_SPAN = 8;
		static central_cache& get_instance()
		{
			static central_cache instance;
			return instance;
		}

		//用于分配指向个数的指向大小的空间
		//memory_size:要申请的大小，block_count:申请的个数
		//返回一组相同大小的指定个数的内存块
		std::optional<list<span<byte>>> allocate(size_t memory_size, size_t block_count);

		// 新增的函数：只申请一个内存块，用于打破初始化死锁
		std::optional<span<byte>> allocate_single(size_t memory_size);

		//回收内存块
		//memories:从线程缓存池中回收的内存碎片
		void deallocate(list<span<byte>> memories);

		~central_cache();

	private:
		central_cache();

		//记录分配到TreadCache的内存块
		void record_allocate_memory_span(span<byte> memory);

		std::optional<span<byte>> get_page_from_page_cache(size_t page_allocate_count);

		/*
		list<span<byte>> m_free_array[size_utils::CACHE_LINE_SIZE];
		std::atomic_flag m_status[size_utils::CACHE_LINE_SIZE];

		map<byte*, page_span> m_page_set[size_utils::CACHE_LINE_SIZE];
		*/

		//指针成员
		CentralCacheImpl* pimpl;

	};
}