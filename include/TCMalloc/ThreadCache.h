#pragma once
//#include "../list.h"
//#include "../set.h"
//#include "../unordered_map.h"
#include <optional>
#include "TCMallocutils.h"
#include "../span.h"

namespace mystl
{
	//PIMPL实现类的前向声明
	class ThreadCacheImpl;

	class thread_cache
	{
	public:
		static thread_cache& get_instance()
		{
			static thread_local thread_cache instance;
			return instance;
		}

		//向内存池申请一块内存
		std::optional<void*> allocate(size_t memory_size);

		//向内存池归还一片空间
		void deallocate(void* start_p, size_t memory_size);

		~thread_cache();

	private:
		thread_cache();

		//向CentralCache层申请一块空间
		std::optional<span<byte>> allocate_from_central_cache(size_t memory_size);
		
		//动态分配内存
		size_t compute_allocate_count(size_t memory_size);

		/*
		//空闲链表
		list<span<byte>> m_free_cache[size_utils::CACHE_LINE_SIZE];

		//用于表示下一次再申请指定大小的内存时会申请几个内存
		size_t m_next_allocate_count[size_utils::CACHE_LINE_SIZE];
		*/

		//指针成员
		ThreadCacheImpl* pimpl;

		// 设置每个列表缓存的上限为256KB（对于16KB的对象即为缓存 256KB / 16KB = 16个）
		// 这个阈值的设置需要分析，如果常用的分配的量比较少
		// 比如只申请几个固定大小的空间，则这个值可以设置的大一些
		// 而申请的内存空间的大小很复杂，则需要设置的小一些，不然可能会让单个线程的空间占用过多
		static constexpr size_t MAX_FREE_BYTES_PER_LISTS = 256 * 1024;

	};
}